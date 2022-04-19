#ifndef PALETTIZER_HPP
#define PALETTIZER_HPP

#include <pse/color/pse_color.hpp>

template <typename _Scalar,
          Color::Space _space          = Color::LAB,
          Color::UnscaledSpace _uspace = Color::Lab_128>
class Palettizer{
public:


    using Scalar                                 = _Scalar;
    static constexpr Color::Space space          = _space;
    static constexpr Color::UnscaledSpace uspace = _uspace;
    using ColorT                                 = Color::ColorBase<Scalar, space>;

    //! this structure handles the data contained in one bin
    struct BinData{
        BinData(): _nativeCol(0,0,0), _binCount(0), _volatileWeight(0){}
        BinData(const ColorT &c, int count = 0):
            _nativeCol(c), _binCount(count), _volatileWeight(0){ }

        BinData(const typename ColorT::CVector &c,
                int count = 0):
            _binCount(count), _volatileWeight(0)
        { _nativeCol.setNative(c); }

        ColorT _nativeCol; //! sum of color collected in the bin
        size_t _binCount; //! number of colors colected in the bin
        Scalar _volatileWeight; //! the weight of the bin
    };

    using BinContainer                    = std::vector<BinData>;

    inline
    Palettizer(size_t k, Scalar kmeansThreshold = Scalar(0.1))
        :_meanCount(k), _kmeansThreshold(kmeansThreshold), _nIter(0){
        clear();
    }
    inline
    ~Palettizer(){}

    //! \warning Copy only parameters
    inline explicit
    Palettizer(const Palettizer& other)
        : _meanCount(other._meanCount),
          _kmeansThreshold(other._kmeansThreshold){
        clear();
    }


    //! \warning Copy only parameters
    inline
    void
    operator=(const Palettizer& other){
        _meanCount = other._meanCount;
        _kmeansThreshold = other._kmeansThreshold;
        clear();
    }

    inline void clear(){
        _bins.clear();
        _bins.resize(_binSizePerDim*_binSizePerDim*_binSizePerDim);
        _means.clear();
        _nIter = 0;
    }

    template <Color::Space inSpace>
    inline
    void addColor(const Color::ColorBase<Scalar, inSpace> &c){
        typename Color::ColorBase<Scalar, inSpace>::CVector v = c.getRGB();
        size_t binIndex = access(scalarToIndex(v[0]), scalarToIndex(v[1]), scalarToIndex(v[2]));
        _bins[binIndex]._nativeCol.template setFrom<space>(_bins[binIndex]._nativeCol.template getAs<space>() + c.template getAs<space>());
        _bins[binIndex]._binCount++;
    }

    void solve(){
        // compute weights
        computeWeights();
        // init means
        initMeans();
        // iterate
        Scalar maxMeanMove;
        _nIter = 0;

        do{
            maxMeanMove = iterate();
            _nIter++;
        } while (maxMeanMove > _kmeansThreshold );
    }

    //! last  color for p is black, always
    template<Color::Space outspace>
    void getPalette(std::vector<Color::ColorBase<Scalar, outspace>> &p) const {
        p.clear();
        p.reserve(_means.size());
        for(auto c : _means){
            p.push_back(c._nativeCol);// the conversion is done implictely
        }
    }


    void setPalette(const std::vector<
                    std::pair<typename Color::ColorBase<Scalar, space>::CVector,
                    int> > &p) {
//        std::cout << "Old means" << std::endl;
//        for (auto c : _means)
//            std::cout << c._nativeCol.template getNative().transpose()
//                      << std::endl;
        _means.clear();
        _means.reserve(p.size());
        for(auto c : p)
            _means.push_back(BinData(c.first, c.second));

//        std::cout << "New means" << std::endl;
//        for (auto c : _means)
//            std::cout << c._nativeCol.template getNative().transpose()
//                      << std::endl;
    }

    inline const BinContainer& getMeans() const { return _means; }
    inline const BinContainer& getBins()  const { return _bins; }
    inline int getNbIter() const { return _nIter; }

    inline void setKMeansThreshold( Scalar kmeansThreshold)
    { _kmeansThreshold = kmeansThreshold; }

    inline Scalar kmeansThreshold() const { return _kmeansThreshold; }

protected:
    inline size_t access(int i, int j, int k) const {
        return i*_binSizePerDim * _binSizePerDim  + j *_binSizePerDim  + k;
    }
    inline size_t scalarToIndex(Scalar v) const { assert(v>=Scalar(0)); return std::min(int(v*_binSizePerDim), int(_binSizePerDim-1)); }

    //! initialize weight to binCount, and compute the mean of color for each bin, by dividing by the number
    inline void computeWeights(){
        for(auto & b : _bins){
            b._volatileWeight = Scalar(b._binCount);
            if(b._binCount>0)
                b._nativeCol.template setFrom<space>(b._nativeCol.template getAs<space>()/b._volatileWeight);
        }
    }

    inline Scalar iterate(){
        BinContainer newMeans;
        newMeans.resize(_means.size());
        for(auto &b : _bins){
            // assign sample to means

            //skip unsignificant bin
            if(b._binCount == 0) continue;

            //! \todo : add weight
            auto cmp = [&] (const BinData &m, const BinData &n){
                return (m._nativeCol.getUnscaledAs(uspace) - b._nativeCol.getUnscaledAs(uspace)).norm()<(n._nativeCol.getUnscaledAs(uspace)-b._nativeCol.getUnscaledAs(uspace)).norm();
                //return (m._nativeCol.getUnscaledAs(uspace) - b._nativeCol.getUnscaledAs(uspace)).norm()*Scalar(1)/Scalar(m._binCount)<(n._nativeCol.getUnscaledAs(uspace)-b._nativeCol.getUnscaledAs(uspace)).norm()*Scalar(1)/Scalar(n._binCount);
            };

            auto nearestMean = std::min_element(_means.begin(), _means.end(), cmp);
            auto meanIndex = std::distance(_means.begin(), nearestMean);
            newMeans[meanIndex]._nativeCol.template setFrom<space>(newMeans[meanIndex]._nativeCol.template getAs<space>() + b._nativeCol.template getAs<space>());
            newMeans[meanIndex]._binCount ++;
        }
        // size()-1 since black is fixed at the end
        Scalar localMaxMeanMove = 0;
        for(size_t i=0; i<_means.size()-1; i++){
            // update means
            ColorT nw;
            nw.template setFrom<space>(newMeans[i]._nativeCol.template getAs<space>() / Scalar(newMeans[i]._binCount));
            Scalar meanMove = (_means[i]._nativeCol.template getAs<space>() - nw.template getAs<space>()).norm();
            _means[i]._nativeCol.template setFrom<space>( nw.template getAs<space>() );
            if(localMaxMeanMove<meanMove) localMaxMeanMove = meanMove;
        }
        return localMaxMeanMove;
    }

    inline void initMeans(){
        _means.clear();
        _means.reserve(_meanCount);
        // we are searching for _meanCount - 1 colors, plus black
        for(size_t i=0; i<_meanCount-1; i++){
            auto maxBin = max_element(_bins.begin(), _bins.end(), [](const BinData&a, const BinData &b){ return a._volatileWeight < b._volatileWeight; } );
            _means.push_back(*maxBin);

            auto unscaledMaxBinCol = maxBin->_nativeCol.getUnscaledAs(uspace);

            // update volatile weight, even for current max
            for(auto &b : _bins){
                Scalar dij2 = (unscaledMaxBinCol - b._nativeCol.getUnscaledAs(uspace)).squaredNorm();
                Scalar d = -dij2/_rho2;
                b._volatileWeight *= Scalar(1) - std::exp( d );
            }
        }

        // lock black at the end
        BinData bd = BinData(ColorT::Black());
        _means.push_back(bd);
    }

    size_t _meanCount;
    const size_t _binSizePerDim = 16;
    const Scalar _rho2 = 80*80;

    Scalar _kmeansThreshold;
    BinContainer _bins;
    BinContainer _means;
    int _nIter;
};

#endif // PALETTIZER_HPP
