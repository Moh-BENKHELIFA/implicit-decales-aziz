#include <iostream>
#include <Eigen/Dense>

#include <pse/pse_literal_string.hpp>
#include <pse/pse_common.hpp>
#include <pse/color/pse_color.hpp>
#include <pse/color/pse_cost_color.hpp>

using Scalar = float;
using ParametricPoint = RGBColor<Scalar>;

#define GET_TAG(T) T<ParametricPoint>::getClassTypeName()
#define GET_HASH(T) T<ParametricPoint>::getClassTypeHash()

#define ALL_FUNCTORS                                                           \
  FUNCTOR(ColorDiscrepancy::AnchorHSV_HueUDiscrepancyFunctor)                  \
  FUNCTOR(ColorDiscrepancy::AnchorHSV_SaturationUDiscrepancyFunctor)           \
  FUNCTOR(ColorDiscrepancy::AnchorHSV_ValueUDiscrepancyFunctor)                \
  FUNCTOR(ColorDiscrepancy::AnchorLAB_LuminanceUDiscrepancyFunctor)            \
  FUNCTOR(ColorDiscrepancy::LinearDevHSV_HueDiscrepancyFunctor)                \
  FUNCTOR(ColorDiscrepancy::LinearDevHSV_SaturationDiscrepancyFunctor)         \
  FUNCTOR(ColorDiscrepancy::LinearDevHSV_ValueDiscrepancyFunctor)              \
  FUNCTOR(ColorDiscrepancy::LinearDevLAB_LuminanceDiscrepancyFunctor)          \
  FUNCTOR(ColorDiscrepancy::LinearDeviationDiscrepancyFunctor)                 \
  FUNCTOR(ColorDiscrepancy::LinearDevHSV_HueUDiscrepancyFunctor)               \
  FUNCTOR(ColorDiscrepancy::LinearDevHSV_SaturationUDiscrepancyFunctor)        \
  FUNCTOR(ColorDiscrepancy::LinearDevHSV_ValueUDiscrepancyFunctor)             \
  FUNCTOR(ColorDiscrepancy::LinearDevLAB_LuminanceUDiscrepancyFunctor)         \
  FUNCTOR(ColorDiscrepancy::HSV_HueDiscrepancyFunctor)                         \
  FUNCTOR(ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor)                  \
  FUNCTOR(ColorDiscrepancy::HSV_ValueDiscrepancyFunctor)                       \
  FUNCTOR(ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor)                   \
  FUNCTOR(ColorDiscrepancy::DistanceDiscrepancyFunctor)                        \
  FUNCTOR(ColorDiscrepancy::HSV_HueUDiscrepancyFunctor)                        \
  FUNCTOR(ColorDiscrepancy::HSV_SaturationUDiscrepancyFunctor)                 \
  FUNCTOR(ColorDiscrepancy::HSV_ValueUDiscrepancyFunctor)                      \
  FUNCTOR(ColorDiscrepancy::LAB_LuminanceUDiscrepancyFunctor)                  \
  FUNCTOR(Discrepancy::ConstantBinaryDiscrepancyFunctor)

void testDiscrTags() {
    #define FUNCTOR(f) << GET_TAG(f) << " " << GET_HASH(f) << std::endl
    std::cout << "Testing hashes: \n"
        ALL_FUNCTORS;
    #undef FUNCTOR
}

inline void increment_by_tag(size_t& counter, const std::string& tag) {
  #define FUNCTOR(f) if(GET_TAG(f) == tag) { ++counter; } else
  ALL_FUNCTORS
  {}
  #undef FUNCTOR
}
inline void increment_by_hash(size_t& counter, const std::size_t hash) {
  #define FUNCTOR(f) if(GET_HASH(f) == hash) { ++counter; } else
  ALL_FUNCTORS
  {}
  #undef FUNCTOR
}


int main(int /*argc*/, char */*argv*/[])
{
    using namespace Utils;

//    constexpr LiteralString str1 = LiteralString("concatenated_to");
//    constexpr LiteralString str2 = LiteralString("_something");
//    constexpr LiteralString str3 = LiteralString("_else");
////    constexpr const LiteralString strres = LiteralString("_something", &str1);
//    const LiteralString strres2 (str2 + str3);

//    std::cout << str1 << std::endl;
//    std::cout << str2 << std::endl;
//    //std::cout << str1 + str2 << std::endl;
//    std::cout << str1 + str2 + str3 << std::endl;
////    std::cout << strres << std::endl;
//    std::cout << strres2 << std::endl;

    testDiscrTags();

    std::vector<std::string> all_tags;
    std::vector<std::size_t> all_hashes;
    #define FUNCTOR(f)                                                         \
      all_tags.push_back(GET_TAG(f));                                          \
      all_hashes.push_back(GET_HASH(f));
    ALL_FUNCTORS
    #undef FUNCTOR

    constexpr std::size_t N = 1000000;
    size_t i;
    std::array<int, N> ridx;
    size_t count_tags = 0;
    size_t count_hashes = 0;

    for(i = 0; i < ridx.size(); ++i) {
        ridx[i] = rand() % all_tags.size();
    }
    for(i = 0; i < ridx.size(); ++i) {
      increment_by_tag(count_tags, all_tags[ridx[i]]);
      increment_by_hash(count_hashes, all_hashes[ridx[i]]);
    }
    std::cout << "Tags counter: " << count_tags << std::endl
              << "Hashes counter: " << count_hashes << std::endl;
}
