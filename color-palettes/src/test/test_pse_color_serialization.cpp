#include <pse/serialization/pse_serializer_yaml.hpp>
#include <pse/serialization/pse_slz_cps.hpp>
#include <pse/serialization/pse_slz_cost_generic.hpp>
#include <pse/color/serialization/pse_slz_color.hpp>
#include <pse/color/serialization/pse_slz_cost_color.hpp>
#include <pse/color/serialization/pse_slz_cost_color_energy.hpp>

#include <pse/color/pse_color.hpp>
#include <pse/color/pse_cost_color.hpp>
#include <pse/color/pse_cost_color_energy.hpp>

#include <iostream>

#define TEST(truth) if(!(truth)) { assert(false); exit(1); }

using RealColor = RGBColor<float>;
using TestCPS = ConstrainedParameterSpace<RealColor>;
using ParametricPointFunctor =
  ColorDiscrepancy::AnchorCCTDiscrepancyFunctor<RealColor>;
using ConstraintFunctor =
  ColorDiscrepancy::DistanceDiscrepancyFunctor<RealColor>;
using GlobalConstraintFunctor =
  ColorDiscrepancy::GlobalAnchorLAB_LuminanceDiscrepancyFunctor<RealColor>;

struct TestState {
  int8_t vi8;
  int16_t vi16;
  int32_t vi32;
  int64_t vi64;
  uint8_t vui8;
  uint16_t vui16;
  uint32_t vui32;
  uint64_t vui64;
  float vf;
  double vd;
  RealColor color;
  TestCPS* cps;
};

class Test {
public:
  INTERPOLIB_TYPENAME_DECLARE("Test")

  explicit Test(const TestState& ini_state )
    : state(ini_state)
  {}
  virtual ~Test() { delete state.cps; }

  struct TestState state;
};

//! \brief Constant storing a reproductible state, i.e. with values that fit the
//! underlying types. E.g., float and double values do not use more significant
//! digits than the maximum their respective type allows.
static const TestState STATE_DEFAULT
  { INT8_MIN + 4, INT16_MIN + 5, INT32_MIN + 6, INT64_MIN + 7,
    UINT8_MAX - 4, UINT16_MAX - 5, UINT32_MAX - 6, UINT64_MAX - 7,
    974.555, 6677895.54665469, {1, 0, 0}, NULL };
static const TestState STATE_NULL
  { 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, {0,0,0}, NULL };

SERIALIZER_CUSTOM_VALUE_BEGIN(TestState, v)
  SERIALIZE_VALUE("vi8", v.vi8);
  SERIALIZE_VALUE("vi16", v.vi16);
  SERIALIZE_VALUE("vi32", v.vi32);
  SERIALIZE_VALUE("vi64", v.vi64);
  SERIALIZE_VALUE("vui8", v.vui8);
  SERIALIZE_VALUE("vui16", v.vui16);
  SERIALIZE_VALUE("vui32", v.vui32);
  SERIALIZE_VALUE("vui64", v.vui64);
  SERIALIZE_VALUE("vf", v.vf);
  SERIALIZE_VALUE("vd", v.vd);
  SERIALIZE_TYPED_OBJECT("color", v.color);
  SERIALIZE_TYPED_OBJECT_PTR("cps", v.cps);
SERIALIZER_CUSTOM_VALUE_END()

SERIALIZER_TYPED_OBJECT_BEGIN(serializeTest, Test, v)
  SERIALIZE_VALUE("state", v.state);
SERIALIZER_TYPED_OBJECT_END()

static inline ESRet
testSerializationFunctionsRegister
  (Serializer& s)
{
  ESRet ret = ESRet_OK;
  SCALL_OR_GOTO(ret,exit, colorTypedObjectSerializationFunctionsRegister<float>(s));
  SCALL_OR_GOTO(ret,exit, cpsTypedObjectSerializationFunctionsRegister<RealColor>(s));
  SCALL_OR_GOTO(ret,exit, colorDiscrepenciesTypedObjectSerializationFunctionsRegister<RealColor>(s));
  SCALL_OR_GOTO(ret,exit, colorEnergyDiscrepenciesTypedObjectSerializationFunctionsRegister<RealColor>(s));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Test, serializeTest));
exit:
  return ret;
}

static inline bool
compare_state
  (struct TestState& s1,
   struct TestState& s2)
{
  if( !( s1.vi8 == s2.vi8
      && s1.vi16 == s2.vi16
      && s1.vi32 == s2.vi32
      && s1.vi64 == s2.vi64
      && s1.vui8 == s2.vui8
      && s1.vui16 == s2.vui16
      && s1.vui32 == s2.vui32
      && s1.vui64 == s2.vui64
      && s1.vf == s2.vf
      && s1.vd == s2.vd
      && s1.color == s2.color
      && s1.cps->getLayersCount() == s2.cps->getLayersCount()
      && s1.cps->getAllParametricPointsCount() == s2.cps->getAllParametricPointsCount()
      && s1.cps->getAllPairingsCount() == s2.cps->getAllPairingsCount()
      && s1.cps->getAllConstraintsCount() == s2.cps->getAllConstraintsCount()
      && s1.cps->getAllGlobalConstraintsCount() == s2.cps->getAllGlobalConstraintsCount()) )
    return false;

  // Now check precisely the CPS contents
  std::vector<TestCPS::ParametricPointParams> s1ppparams;
  std::vector<TestCPS::ParametricPointParams> s2ppparams;
  CHECK_OR_DO(ERet_OK == s1.cps->getAllParametricPointsParams(s1ppparams),
    return false);
  CHECK_OR_DO(ERet_OK == s2.cps->getAllParametricPointsParams(s2ppparams),
    return false);
  CHECK_OR_DO(s1ppparams.size() == s2ppparams.size(), return false);
  for(ParametricPointId id = 0; id < s1ppparams.size(); ++id) {
    TestCPS::ParametricPointParams& s1pp = s1ppparams[id];
    TestCPS::ParametricPointParams& s2pp = s2ppparams[id];
    if(  s1pp._value != s2pp._value
      || s1pp._importance != s2pp._importance
      || s1pp._own_constraints_functors_count != s2pp._own_constraints_functors_count)
      return false;
    for(size_t f = 0; f < s1pp._own_constraints_functors_count; ++f) {
      TestCPS::UnaryConstraintFunctor* f1 = s1pp._own_constraints_functors[f];
      TestCPS::UnaryConstraintFunctor* f2 = s2pp._own_constraints_functors[f];
      if( f1->getTypeHash() != f2->getTypeHash() )
        return false;
      // TODO: check functors parameters!
    }
  }

  std::vector<TestCPS::PairingParams> s1pparams;
  std::vector<TestCPS::PairingParams> s2pparams;
  CHECK_OR_DO(ERet_OK == s1.cps->getAllPairingsParams(s1pparams),
    return false);
  CHECK_OR_DO(ERet_OK == s2.cps->getAllPairingsParams(s2pparams),
    return false);
  CHECK_OR_DO(s1pparams.size() == s2pparams.size(), return false);
  for(PairingId id = 0; id < s1pparams.size(); ++id) {
    TestCPS::PairingParams& s1p = s1pparams[id];
    TestCPS::PairingParams& s2p = s2pparams[id];
    if(  s1p.first != s2p.first
      || s1p.second != s2p.second )
      return false;
  }

  std::vector<TestCPS::ConstraintParams> s1cparams;
  std::vector<TestCPS::ConstraintParams> s2cparams;
  CHECK_OR_DO(ERet_OK == s1.cps->getAllConstraintsParams(s1cparams),
    return false);
  CHECK_OR_DO(ERet_OK == s2.cps->getAllConstraintsParams(s2cparams),
    return false);
  CHECK_OR_DO(s1cparams.size() == s2cparams.size(), return false);
  for(ConstraintId id = 0; id < s1cparams.size(); ++id) {
    TestCPS::ConstraintParams& s1c = s1cparams[id];
    TestCPS::ConstraintParams& s2c = s2cparams[id];
    if(  s1c._ppoints.first != s2c._ppoints.first
      || s1c._ppoints.second != s2c._ppoints.second
      || s1c._functors_count != s2c._functors_count )
      return false;
    for(size_t f = 0; f < s1c._functors_count; ++f) {
      TestCPS::BinaryConstraintFunctor* f1 = s1c._functors[f];
      TestCPS::BinaryConstraintFunctor* f2 = s2c._functors[f];
      if( f1->getTypeHash() != f2->getTypeHash() )
        return false;
      // TODO: check functors parameters
    }
  }

  std::vector<TestCPS::GlobalConstraintParams> s1gcparams;
  std::vector<TestCPS::GlobalConstraintParams> s2gcparams;
  CHECK_OR_DO(ERet_OK == s1.cps->getAllGlobalConstraintsParams(s1gcparams),
    return false);
  CHECK_OR_DO(ERet_OK == s2.cps->getAllGlobalConstraintsParams(s2gcparams),
    return false);
  CHECK_OR_DO(s1gcparams.size() == s2gcparams.size(), return false);
  for(GlobalConstraintId id = 0; id < s1gcparams.size(); ++id) {
    TestCPS::GlobalConstraintParams& s1gc = s1gcparams[id];
    TestCPS::GlobalConstraintParams& s2gc = s2gcparams[id];
    TestCPS::NaryConstraintFunctor* f1 = s1gc._functor;
    TestCPS::NaryConstraintFunctor* f2 = s2gc._functor;
    if( (f1 != nullptr) != (f2 != nullptr) )
      return false;
    if( f1 ) {
      if( f1->getTypeHash() != f2->getTypeHash() )
        return false;
      // TODO: check functors parameters
    }
  }
  return true;
}

static void
init_cps
  (TestCPS& cps)
{
  std::vector<TestCPS::UnaryConstraintFunctor*> ppfs {
    new ParametricPointFunctor{},
    new ParametricPointFunctor{},
    new ParametricPointFunctor{},
    new ParametricPointFunctor{}
  };

  ParametricPointIdList ppids;
  std::vector<TestCPS::ParametricPointParams> ppparams_src {
    {{1.0f, 0.5f, 0.5f}, 1.0f, 1, &ppfs[0]},
    {{0.0f, 0.0f, 1.0f}, 1.0f, 1, &ppfs[1]}
  };
  std::vector<TestCPS::ParametricPointParams> ppparams_dst {
    {{0.0f, 1.0f, 0.0f}, 1.0f, 1, &ppfs[2]},
    {{1.0f, 1.0f, 1.0f}, 1.0f, 1, &ppfs[3]}
  };
  TEST(ERet_OK == cps.addParametricPoints(0, ppparams_src, ppids));
  TEST(ERet_OK == cps.addParametricPoints(1, ppparams_dst, ppids));

  PairingIdList pids;
  std::vector<TestCPS::PairingParams> pparams {
    {ppids[0], ppids[2]},
    {ppids[1], ppids[3]}
  };
  TEST(ERet_OK == cps.addPairings(pparams, pids));

  std::vector<TestCPS::BinaryConstraintFunctor*> cfs {
    new ConstraintFunctor{},
    new ConstraintFunctor{}
  };

  ConstraintIdList cids;
  std::vector<TestCPS::ConstraintParams> cparams {
    {{ppids[0], ppids[1]}, 1, &cfs[0]},
    {{ppids[2], ppids[3]}, 1, &cfs[1]}
  };
  TEST(ERet_OK == cps.addConstraints(cparams, cids));

  std::vector<TestCPS::NaryConstraintFunctor*> gcfs {
    new GlobalConstraintFunctor{},
    new GlobalConstraintFunctor{}
  };

  GlobalConstraintIdList gcids;
  std::vector<TestCPS::GlobalConstraintParams> gcparams_src {
    {gcfs[0]}
  };
  std::vector<TestCPS::GlobalConstraintParams> gcparams_dst {
    {gcfs[1]}
  };
  TEST(ERet_OK == cps.addGlobalConstraints(0, gcparams_src, gcids));
  TEST(ERet_OK == cps.addGlobalConstraints(1, gcparams_dst, gcids));

}

template<typename T, typename SERIALIZER>
static void
serialize
  (SERIALIZER& s,
   T params,
   Test*& st)
{
  SCALL(s.beginStream(params));
  SCALL(serializeTypedObjectPtr(s, "bob", st));
  SCALL(s.endStream());
}

int main() {
  Factory f;
  FACTORY_ADD_TYPE(f, RealColor);
  FACTORY_ADD_TYPE(f, TestCPS);
  FACTORY_ADD_TYPE(f, ParametricPointFunctor);
  FACTORY_ADD_TYPE(f, ConstraintFunctor);
  FACTORY_ADD_TYPE(f, GlobalConstraintFunctor);
  FACTORY_ADD_TYPE_SIMPLE(f, Test, {STATE_NULL});

  Test* sw = new Test{ STATE_DEFAULT };
  Test* sr = nullptr;
  sw->state.cps = new TestCPS(2);
  init_cps(*sw->state.cps);

  /*
  SerializerQtXML s;
  TEST(ESRet_OK == s.addFactory(&f));
  TEST(ESRet_OK == testSerializationFunctionsRegister(s));
  QString xml_buffer;
  QXmlStreamWriter xmlw(&xml_buffer);
  serialize<QXmlStreamWriter&>(s,xmlw,sw);
  std::cout << xml_buffer.toStdString() << std::flush;
  QXmlStreamReader xmlr(xml_buffer);
  serialize<QXmlStreamReader&>(s,xmlr,sr);
  TEST(compare_state(sw->state,sr->state));
  delete sr;
  sr = nullptr;

  TEST(s.removeFactory(&f) == ESRet_OK);
  */

  SerializerYAML sy;
  TEST(ESRet_OK == sy.addFactory(&f));
  TEST(ESRet_OK == testSerializationFunctionsRegister(sy));
  std::string yaml_buffer;
  serialize<std::string&>(sy,yaml_buffer,sw);
  std::cout << yaml_buffer << std::flush;
  serialize<const std::string&>(sy,yaml_buffer,sr);
  TEST(compare_state(sw->state,sr->state));
  delete sr;
  sr = nullptr;

  delete sw;

  return 0;
}
