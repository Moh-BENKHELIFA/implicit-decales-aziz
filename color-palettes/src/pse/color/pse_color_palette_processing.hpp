#ifndef PALETTEPROCESSING_HPP
#define PALETTEPROCESSING_HPP

#include <pse/color/pse_color.hpp>

namespace PaletteProcessing {

namespace Sorting {

/*!
 * Sorting functors, compatible with std::sort API
 */
template <int dim, Color::Space space>
struct perDimFunctor {

    template <typename ColT1, typename ColT2>
    static inline bool cmp(const ColT1& c1, const ColT1& c2) {
        using namespace Color;

        constexpr Color::Space space1 = ColT1::space;
        constexpr Color::Space space2 = ColT2::space;

        // check input are actual colors
        static_assert(std::is_base_of<ColorBase<typename ColT1::Scalar, space1>,
                      ColT1>::value,
                      "Inputs must inherit Color::ColorBase" );
        static_assert(std::is_base_of<ColorBase<typename ColT2::Scalar, space2>,
                      ColT2>::value,
                      "Inputs must inherit Color::ColorBase" );


        return c1.template getAs<space>()(dim) < c2.template getAs<space>()(dim);
    }

    template <typename ColT1, typename ColT2>
    inline bool operator()(const ColT1& c1, const ColT1& c2) {
        return cmp<ColT1, ColT2>(c1, c2);
    }

};

using perHue        =  perDimFunctor<0, Color::HSV>;
using perSaturation =  perDimFunctor<1, Color::HSV>;
using perValue      =  perDimFunctor<2, Color::HSV>;
using perLuminance  =  perDimFunctor<0, Color::LAB>;
using perLabA       =  perDimFunctor<1, Color::LAB>;
using perLabB       =  perDimFunctor<2, Color::LAB>;

} // namespace Sorting


/*!
 * Groups the colors that are similar
 *
 * Let the colors in the same group if their attribute is below the input threshold
 * Only pairwise relations are considered
 *
 * \return the number of generated group
 */
template <int dim, Color::Space space, typename S, typename Palette, typename Ids>
static inline int groupSimilar(S threshold,
                                const Palette& palette,
                                Ids &ids){
    using IdsT        = typename Ids::value_type;
    using InputColorT = typename Palette::value_type;

    int nColors = palette.size();

    // deref stores the ids of each color within the palette
    ids.resize(nColors);
    std::vector<int>deref (nColors);
    std::iota(deref.begin(), deref.end(), IdsT(0));

    // now sorting the deref ids according to the colors order
    std::sort(deref.begin(), deref.end(), [&palette](const int& i0, const int& i1){
        using ColorSort = Sorting::perDimFunctor<dim, space>;
        return ColorSort::template cmp<InputColorT,InputColorT>(palette[i0], palette[i1]);
    });

    // pass sequentially over colors to group them according to input threeshold
    using AttrT = typename Palette::value_type::Scalar;
    int currentId (0);
    AttrT ref (palette[deref[0]].template getAs<space>()(dim));

    for(int& id : deref ){
        AttrT current = palette[id].template getAs<space>()(dim);
        if (S(std::abs(current - ref)) >= threshold) { // not same group
            ++ currentId;
        }
        ids[id]  = currentId;
        ref      = current;
    }

    return currentId+1;
}


} // namespace PaletteProcessing

#endif // PALETTEPROCESSING_HPP
