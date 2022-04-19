#ifndef PALETTIZEDRASTER_HPP
#define PALETTIZEDRASTER_HPP

#include <pse/color/pse_color.hpp>
#include <pse/color/pse_color_palettizer.hpp>

/*!
 * \brief Raster image represented as a color displacement map
 */
template <typename _Scalar, Color::Space _space>
class PalettizedRaster{
public:
    static constexpr Color::Space space = _space;
    using Scalar       = _Scalar;
    using ColorT       = Color::ColorBase<Scalar, space>;
    using WColorVec    = std::pair<typename ColorT::CVector, int>;

    struct Pixel{
        using Idx = int;

        //! \brief Id the reference color in the palette
        int colorId;

        //! \brief Offset vector defined relatively to the reference color
        ColorT offset;

        inline Pixel(): colorId(0), offset(ColorT::Black()){} //Black=Zero
    };
    using PixContainer = std::vector<Pixel>;

public:
    inline PalettizedRaster() :_width(0), _height(0) {}

    inline PalettizedRaster(int width, int height)
        :_width(width), _height(height) { _data.resize(width*height); }

    //! \brief Create a palettized image from an input image
    template <typename inContainer, typename PalettizerT>
    inline PalettizedRaster(int width, int height,
                            const inContainer& inputIm,
                            PalettizerT& palettizer)
    { process(width, height, inputIm, palettizer); }

    inline const PixContainer& data() const { return _data; }
    inline int width() const { return _width; }
    inline int height() const { return _height; }

    inline const Pixel& pixel(int x, int y) const { return _data.at(y*_width + x); }
    inline       Pixel& pixel(int x, int y)       { return _data.at(y*_width + x); }

    /*!
     * \brief Use a Palettizer to find the references colors and fill the
     * internal PixContainer
     */
    template <typename InContainer, typename PalettizerT>
    inline void process (int width, int height,
                         const InContainer& inputIm,
                         PalettizerT& palettizer );

    /*!
     * \brief Reconstruct an image from the decomposition and a arbitrary palette
     * by re-applying the color offset to the new palette
     */
    template<Color::Space inSpace, typename OutContainer>
    inline void reconstructFromPalette(
            const std::vector<Color::ColorBase<Scalar, inSpace>>& palette,
            OutContainer& output) const;

    /*!
     * \brief Copy the segmentation of the current raster to another image and
     * recompute the associated palette
     * \param source Input raster, from which will be copied the spatial segmentation
     * \param targetIm Target image, from which we compute the new palette using the source segmentation
     * \param out Output raster, combining the source segmentation and the targetIm colors
     */
    template <typename InContainer>
    static std::vector<WColorVec>
    copySegmentation(const PalettizedRaster<Scalar, space>& segSource,
                            const InContainer& targetIm,
                            PalettizedRaster<Scalar, space>& out);


    /*!
     * \brief Recompute the displacement vectors using a palette for a given
     *        image and using the raster segmentation
     */
    template <Color::Space inSpace, typename InContainer>
    inline void
    recomputeDisplacement( const std::vector<Color::ColorBase<Scalar, inSpace>>& palette,
                           const InContainer& targetIm);

private:
    PixContainer _data;
    int _width, _height;
};

template <typename Scalar, Color::Space space>
template <typename InContainer, typename PalettizerT>
void
PalettizedRaster<Scalar, space>::process (int width, int height,
        const InContainer& inputIm,
        PalettizerT &palettizer){
    _data.resize(width*height);
    _width  = width;
    _height = height;
    palettizer.clear();

    // compute palette
    for( auto color : inputIm ){
        palettizer.addColor(color);
    }
    palettizer.solve();

    const typename PalettizerT::BinContainer& means = palettizer.getMeans();
    constexpr Color::UnscaledSpace uspace = PalettizerT::uspace;

//#define RECOMPUTE_PALETTE
#ifdef RECOMPUTE_PALETTE
    int paletteSize = palettizer.getMeans().size();

    // Color vector + weight
    std::vector<WColorVec> palette;//
    palette.resize(paletteSize);
#endif

    // compute the reference color + displacement vector of each pixel
    // let's openmp collapse the loops for us
#pragma omp parallel for
        for (int idx = 0; idx < width*height; ++idx){

            const typename InContainer::value_type& inColor = inputIm.at(idx);
            typename InContainer::value_type::CVector
                    unscaledInColor = inColor.getUnscaledAs(uspace);

            Pixel& offsetPixel = _data[idx];

            // lambda used to get the closest mean
            auto cmp = [&unscaledInColor,&uspace] (
                    const typename PalettizerT::BinData &m,
                    const typename PalettizerT::BinData &n){
                return (m._nativeCol.getUnscaledAs(uspace) -
                        unscaledInColor).norm()
                        <(n._nativeCol.getUnscaledAs(uspace)-
                          unscaledInColor).norm();
            };


            // compute closest mean
            auto nearestMean    = std::min_element(means.cbegin(), means.cend(), cmp);
#ifdef RECOMPUTE_PALETTE
            int colorId         =
#endif
                    offsetPixel.colorId = std::distance(means.cbegin(), nearestMean);
#ifdef RECOMPUTE_PALETTE
            WColorVec& col      = palette.at(colorId);
            col.first.array()  += inputIm.at(idx).template getAs<space>().array();
            ++ col.second;
#else
            offsetPixel.offset = inputIm.at(idx) - nearestMean->_nativeCol;
#endif
        }

#ifdef RECOMPUTE_PALETTE
        for( WColorVec& col : palette ) col.first /= Scalar(col.second);

        // now that the pixels are clustered we recompute the means to center it
        // inside the cluster and get better displacement values
//#pragma omp for
        for (int idx = 0; idx < width*height; ++idx){
            Pixel& offsetPixel     = _data[idx];
            WColorVec& col         = palette.at(offsetPixel.colorId);
            offsetPixel.offset.template setFrom<space>(
                        inputIm.at(idx).template getAs<space>().array()
                        - col.first.array());

        }

    palettizer.setPalette(palette);

#endif

}


/*!
 * \brief Reconstruct an image from the decomposition and a arbitrary palette
 * by re-applying the color offset to the new palette
 */
template <typename Scalar, Color::Space space>
template<Color::Space inSpace, typename OutContainer>
void
PalettizedRaster<Scalar, space>::reconstructFromPalette(
        const std::vector<Color::ColorBase<Scalar, inSpace>>& palette,
        OutContainer& output) const{
    output.resize(_width*_height);

    // convert to QImage
    // let's openmp collapse the loops for us
#pragma omp parallel for collapse(2)
    for (int y = 0; y < _height; ++y){
        for (int x = 0; x < _width; ++x){
            int idx = y*_width + x;
            const Pixel& pixel = _data[idx];
            output[idx] = palette[pixel.colorId] + pixel.offset;
        }
    }
}


template <typename Scalar, Color::Space space>
template <Color::Space inSpace, typename InContainer>
void
PalettizedRaster<Scalar, space>::recomputeDisplacement(
        const std::vector<Color::ColorBase<Scalar, inSpace>>& palette,
        const InContainer& image){

    // compute the Pixels displacement
#pragma omp parallel for
    for(unsigned int i = 0; i< _data.size(); ++i){
        Pixel &pixel = _data[i];
        pixel.offset = image.at(i) - palette[pixel.colorId];
    }
}


template <typename Scalar, Color::Space space>
template <typename InContainer>
std::vector<typename PalettizedRaster<Scalar, space>::WColorVec>
PalettizedRaster<Scalar, space>::copySegmentation(
        const PalettizedRaster<Scalar, space>& segmentationSource,
        const InContainer& image,
        PalettizedRaster<Scalar, space>& out){
    using T = PalettizedRaster<Scalar, space>;

    // init source memory
    out = T(segmentationSource._width, segmentationSource._height);


    const int length = out._data.size();

    // retrieve input palette size
    int paletteSize =
            std::max_element(segmentationSource._data.begin(), segmentationSource._data.end(),
                             [](const Pixel&__lhs, const Pixel&__rhs){
        return __lhs.colorId < __rhs.colorId;
    })->colorId + 1 ;

    // Color vector + weight
    std::vector<ColorT> palette;
    std::vector<int> weights;
    palette.resize(paletteSize, ColorT::Zero());
    weights.resize(paletteSize, 0);

    // compute Palette values
    for(int i = 0; i< length; ++i){
        int colorId          =  out._data[i].colorId = segmentationSource._data[i].colorId;
        palette.at(colorId) += image.at(i);
        ++ weights[colorId];
    }

    for(int i = 0; i < paletteSize; ++i)
        palette[i] /= Scalar(weights[i]);

    out.recomputeDisplacement(palette, image);


    // generate output, looks stupid to wrap it up here, but it's mainly for
    // backward compatibility issues
    std::vector<typename PalettizedRaster<Scalar, space>::WColorVec> wpal;
    wpal.reserve(paletteSize);
    for(int i = 0; i < paletteSize; ++i)
        wpal.push_back( std::make_pair(palette[i].getNative(), weights[i]) );

    return wpal;
}

#endif // PALETTIZEDRASTER_HPP
