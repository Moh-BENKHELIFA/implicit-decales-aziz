#ifndef DISCREPANCY_COLOR_LABEL_MAP_HPP
#define DISCREPANCY_COLOR_LABEL_MAP_HPP

#include <pse/pse_literal_string.hpp>

#include <pse/color/pse_cost_color.hpp>
#include <pse/color/pse_cost_color_energy.hpp>

#include <array>
#include <map>

namespace ColorDiscrepancy {

//// Array storing the unary discrepancy names used in the GUI
constexpr std::array<Utils::LiteralString, 30> UnaryDiscrLabels =
{
    Utils::LiteralString("Linear Hue"),
    Utils::LiteralString("Linear Saturation"),
    Utils::LiteralString("Linear Value"),
    Utils::LiteralString("Linear Luminance"),
    Utils::LiteralString("Linear a (Lab)"),
    Utils::LiteralString("Linear b (Lab)"),
    Utils::LiteralString("Linear"),
    Utils::LiteralString("Linear Hue (unsigned)"),
    Utils::LiteralString("Linear Saturation (unsigned)"),
    Utils::LiteralString("Linear Value (unsigned)"),
    Utils::LiteralString("Linear Luminance (unsigned)"),
    Utils::LiteralString("Linear a (Lab - unsigned)"),
    Utils::LiteralString("Linear b (Lab - unsigned)"),
    Utils::LiteralString("Anchor Hue"),
    Utils::LiteralString("Anchor Saturation"),
    Utils::LiteralString("Anchor Value"),
    Utils::LiteralString("Anchor Luminance"),
    Utils::LiteralString("Anchor a (Lab)"),
    Utils::LiteralString("Anchor b (Lab)"),
    Utils::LiteralString("Anchor"),
    Utils::LiteralString("Anchor Hue (unsigned)"),
    Utils::LiteralString("Anchor Saturation (unsigned)"),
    Utils::LiteralString("Anchor Value (unsigned)"),
    Utils::LiteralString("Anchor Luminance (unsigned)"),
    Utils::LiteralString("Anchor a (Lab - unsigned)"),
    Utils::LiteralString("Anchor b (Lab - unsigned)"),
    Utils::LiteralString("Anchor Temperature"),
    Utils::LiteralString("Anchor Manifold"),
    Utils::LiteralString("Anchor Manifold (Hue)"),
    Utils::LiteralString("Improve Energy efficiency")
};

//// Array storing the binary discrepancy names used in the GUI
constexpr std::array<Utils::LiteralString, 14> BinaryDiscrLabels =
{
    Utils::LiteralString("Hue"),
    Utils::LiteralString("Saturation"),
    Utils::LiteralString("Value"),
    Utils::LiteralString("Luminance"),
    Utils::LiteralString("Lab-a"),
    Utils::LiteralString("Lab-b"),
    Utils::LiteralString("Distance (ambient space)"),
    Utils::LiteralString("Hue (unsigned)"),
    Utils::LiteralString("Saturation (unsigned)"),
    Utils::LiteralString("Value (unsigned)"),
    Utils::LiteralString("Luminance (unsigned)"),
    Utils::LiteralString("Lab-a (unsigned)"),
    Utils::LiteralString("Lab-b (unsigned)"),
    Utils::LiteralString("Relative L")
};

//// Array storing the binary discrepancy names used in the GUI
constexpr std::array<Utils::LiteralString, 2> GlobalDiscrLabels =
{
    Utils::LiteralString("Luminance (Lab)"),
    //Utils::LiteralString("Saturation (HSV)"),
    //Utils::LiteralString("Value (HSV)"),
    Utils::LiteralString("Energy consumption")/*,
    Utils::LiteralString("Color Temperature")*/
};

struct DiscrepancyLabelMap {
public:
    using LabelHashT   = std::size_t;
    using TagHashT     = std::size_t;
    using IdT          = int;
private:
    using LabelToIdMap = std::map<LabelHashT, int>;
    using TagToIdMap   = std::map<TagHashT,   int>;
    using IdToTagMap   = std::map<int, TagHashT>;

    // stores link between Labels hash keys, to Labels ids in UnaryDiscrLabels
    LabelToIdMap UnaryLabelToIdMap, BinaryLabelToIdMap, GlobalLabelToIdMap;
    // stores link between Tags hash keys, to Labels ids in UnaryDiscrLabels
    TagToIdMap   UnaryTagToIdMap,   BinaryTagToIdMap,   GlobalTagToIdMap;
    // stores link between Labels ids in UnaryDiscrLabels, to Tags hash keys
    IdToTagMap   UnaryIdToTagMap,   BinaryIdToTagMap,   GlobalIdToTagMap;

public:
    inline DiscrepancyLabelMap() {

#define GET_TAG(T) T<Test_BezierInterpolation::Color>::getClassTypeName()
#define GET_HASH(T) T<Test_BezierInterpolation::Color>::getClassTypeHash()

    for (int i = 0; i!= UnaryDiscrLabels.size(); ++i)
        UnaryLabelToIdMap[UnaryDiscrLabels[i].hash()] = i;
    for (int i = 0; i!= BinaryDiscrLabels.size(); ++i)
        BinaryLabelToIdMap[BinaryDiscrLabels[i].hash()] = i;
    for (int i = 0; i!= GlobalDiscrLabels.size(); ++i)
        GlobalLabelToIdMap[GlobalDiscrLabels[i].hash()] = i;

#define ADD_ENTRIES(M1,M2,T,id) \
    M1[GET_HASH(T)] = id;       \
    M2[id] = GET_HASH(T);
#define ADD_UNARY_ENTRIES(T,id)  ADD_ENTRIES(UnaryTagToIdMap,UnaryIdToTagMap,T,id)
#define ADD_BINARY_ENTRIES(T,id) ADD_ENTRIES(BinaryTagToIdMap,BinaryIdToTagMap,T,id)
#define ADD_GLOBAL_ENTRIES(T,id) ADD_ENTRIES(GlobalTagToIdMap,GlobalIdToTagMap,T,id)

    //using namespace Discrepancy;
    using namespace ColorDiscrepancy;
    using namespace EnergyDiscrepancy;

    ADD_UNARY_ENTRIES(LinearDevHSV_HueDiscrepancyFunctor,0);
    ADD_UNARY_ENTRIES(LinearDevHSV_SaturationDiscrepancyFunctor,1);
    ADD_UNARY_ENTRIES(LinearDevHSV_ValueDiscrepancyFunctor,2);
    ADD_UNARY_ENTRIES(LinearDevLAB_LuminanceDiscrepancyFunctor,3);
    ADD_UNARY_ENTRIES(LinearDevLAB_aDiscrepancyFunctor,4);
    ADD_UNARY_ENTRIES(LinearDevLAB_bDiscrepancyFunctor,5);
    ADD_UNARY_ENTRIES(LinearDeviationDiscrepancyFunctor,6);
    ADD_UNARY_ENTRIES(LinearDevHSV_HueUDiscrepancyFunctor,7);
    ADD_UNARY_ENTRIES(LinearDevHSV_SaturationUDiscrepancyFunctor,8);
    ADD_UNARY_ENTRIES(LinearDevHSV_ValueUDiscrepancyFunctor,9);
    ADD_UNARY_ENTRIES(LinearDevLAB_LuminanceUDiscrepancyFunctor,10);
    ADD_UNARY_ENTRIES(LinearDevLAB_aUDiscrepancyFunctor,11);
    ADD_UNARY_ENTRIES(LinearDevLAB_bUDiscrepancyFunctor,12);
    ADD_UNARY_ENTRIES(AnchorHSV_HueDiscrepancyFunctor,13);
    ADD_UNARY_ENTRIES(AnchorHSV_SaturationDiscrepancyFunctor,14);
    ADD_UNARY_ENTRIES(AnchorHSV_ValueDiscrepancyFunctor,15);
    ADD_UNARY_ENTRIES(AnchorLAB_LuminanceDiscrepancyFunctor,16);
    ADD_UNARY_ENTRIES(AnchorLAB_aDiscrepancyFunctor,17);
    ADD_UNARY_ENTRIES(AnchorLAB_bDiscrepancyFunctor,18);
    ADD_UNARY_ENTRIES(AnchorDiscrepancyFunctor,19);
    ADD_UNARY_ENTRIES(AnchorHSV_HueUDiscrepancyFunctor,20);
    ADD_UNARY_ENTRIES(AnchorHSV_SaturationUDiscrepancyFunctor,21);
    ADD_UNARY_ENTRIES(AnchorHSV_ValueUDiscrepancyFunctor,22);
    ADD_UNARY_ENTRIES(AnchorLAB_LuminanceUDiscrepancyFunctor,23);
    ADD_UNARY_ENTRIES(AnchorLAB_aUDiscrepancyFunctor,24);
    ADD_UNARY_ENTRIES(AnchorLAB_bUDiscrepancyFunctor,25);
    ADD_UNARY_ENTRIES(AnchorCCTDiscrepancyFunctor,26);
    ADD_UNARY_ENTRIES(PointSetDeviationDiscrepancyFunctor,27);
    ADD_UNARY_ENTRIES(PointSetDevHSV_HueUDiscrepancyFunctor, 28);
    ADD_UNARY_ENTRIES(RGBMaxDiscrepancyFunctor,29);

    ADD_BINARY_ENTRIES(HSV_HueDiscrepancyFunctor,0);
    ADD_BINARY_ENTRIES(HSV_SaturationDiscrepancyFunctor,1);
    ADD_BINARY_ENTRIES(HSV_ValueDiscrepancyFunctor,2);
    ADD_BINARY_ENTRIES(LAB_LuminanceDiscrepancyFunctor,3);
    ADD_BINARY_ENTRIES(LAB_aDiscrepancyFunctor,4);
    ADD_BINARY_ENTRIES(LAB_bDiscrepancyFunctor,5);
    ADD_BINARY_ENTRIES(DistanceDiscrepancyFunctor,6);
    ADD_BINARY_ENTRIES(HSV_HueUDiscrepancyFunctor,7);
    ADD_BINARY_ENTRIES(HSV_SaturationUDiscrepancyFunctor,8);
    ADD_BINARY_ENTRIES(HSV_ValueUDiscrepancyFunctor,9);
    ADD_BINARY_ENTRIES(LAB_LuminanceUDiscrepancyFunctor,10);
    ADD_BINARY_ENTRIES(LAB_aUDiscrepancyFunctor,11);
    ADD_BINARY_ENTRIES(LAB_bUDiscrepancyFunctor,12);
    ADD_BINARY_ENTRIES(RelativeLuminanceDiscrepancyFunctor,13);

    ADD_GLOBAL_ENTRIES(GlobalAnchorLAB_LuminanceDiscrepancyFunctor, 0);
    //ADD_GLOBAL_ENTRIES(GlobalAnchorHSV_SaturationDiscrepancyFunctor, 1);
    //ADD_GLOBAL_ENTRIES(GlobalAnchorHSV_ValueDiscrepancyFunctor, 2);
    ADD_GLOBAL_ENTRIES(RGBMaxGlobalAnchorDiscrepancyFunctor, 1);
    //ADD_GLOBAL_ENTRIES(CCTGlobalAnchorDiscrepancyFunctor, 4);

#undef ADD_ENTRIES
#undef ADD_UNARY_ENTRIES
#undef ADD_BINARY_ENTRIES
#undef ADD_GLOBAL_ENTRIES
#undef GET_HASH
#undef GET_TAG
}

    static inline constexpr size_t countUnary()  { return UnaryDiscrLabels.size(); }
    static inline constexpr size_t countBinary() { return BinaryDiscrLabels.size(); }
    static inline constexpr size_t countGlobal() { return GlobalDiscrLabels.size(); }

    /**!
     * Generates methods of the form
     *   \code
     *  const typename LabelToIdMap::mapped_type &
     *  queryUnaryLabelToIdMap( const typename LabelToIdMap::key_type& key ) const
     *  {
     *      return UnaryLabelToIdMap.at(key);
     *  }
     *  \endcode
     *  by calling GENERATE_QUERY_FUN(LabelToIdMap,Unary).
     */
#define GENERATE_SIMPLE_QUERY_FUN(MapType,MapPrefix)                                  \
    inline const typename MapType::mapped_type&  query ## MapPrefix ## MapType \
      (const typename MapType::key_type & key) const                           \
    {  return MapPrefix ## MapType.at(key); }

    GENERATE_SIMPLE_QUERY_FUN(LabelToIdMap,Unary)
    GENERATE_SIMPLE_QUERY_FUN(LabelToIdMap,Binary)
    GENERATE_SIMPLE_QUERY_FUN(LabelToIdMap,Global)

    GENERATE_SIMPLE_QUERY_FUN(TagToIdMap,Unary)
    GENERATE_SIMPLE_QUERY_FUN(TagToIdMap,Binary)
    GENERATE_SIMPLE_QUERY_FUN(TagToIdMap,Global)

    GENERATE_SIMPLE_QUERY_FUN(IdToTagMap,Unary)
    GENERATE_SIMPLE_QUERY_FUN(IdToTagMap,Binary)
    GENERATE_SIMPLE_QUERY_FUN(IdToTagMap,Global)

#undef GENERATE_SIMPLE_QUERY_FUN

    /**!
     *  Generate complex queries: Label to Hash (passing by Id)
     *   \code
     *   const typename IdToTagMap::mapped_type &
     *   queryUnaryLabelToHashMap( const typename LabelToIdMap::key_type& key ) const
     *   {
     *       return queryUnaryIdToTagMap(queryUnaryLabelToIdMap(key));
     *   }
     *   \endcode
     */
#define GENERATE_DOUBLE_QUERY_FUN(FirstMapType,SecondMapType,MapPrefix,OpName)        \
    inline const typename SecondMapType::mapped_type&  query ## MapPrefix ## OpName \
      (const typename FirstMapType::key_type & key) const                           \
    {  return query ## MapPrefix ## SecondMapType(query ## MapPrefix ## FirstMapType(key)); }

    GENERATE_DOUBLE_QUERY_FUN(LabelToIdMap,IdToTagMap,Unary, LabelToHashMap)
    GENERATE_DOUBLE_QUERY_FUN(LabelToIdMap,IdToTagMap,Binary,LabelToHashMap)
    GENERATE_DOUBLE_QUERY_FUN(LabelToIdMap,IdToTagMap,Global,LabelToHashMap)

#undef GENERATE_DOUBLE_QUERY_FUN

    Utils::LiteralString queryUnaryTagToLabel(const TagToIdMap::key_type &key){
        return UnaryDiscrLabels[ queryUnaryTagToIdMap(key) ];
    }
    Utils::LiteralString queryBinaryTagToLabel(const TagToIdMap::key_type &key){
        return BinaryDiscrLabels[ queryBinaryTagToIdMap(key) ];
    }
    Utils::LiteralString queryGlobalTagToLabel(const TagToIdMap::key_type &key){
        return GlobalDiscrLabels[ queryGlobalTagToIdMap(key) ];
    }

};

} // namespace ColorDiscrepancy

#endif // DISCREPANCY_COLOR_LABEL_MAP_HPP
