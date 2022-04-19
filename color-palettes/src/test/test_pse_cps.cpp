#include <pse/pse_cps.hpp>

#include <iostream>

#define TEST(truth) if(!(truth)) { assert(false); return 1; }

struct RGBf {
  static const std::string getClassTypeName() { return "RGBf"; }
  typedef float Scalar;
  float r, g, b;
};

static constexpr LayerId SRC_PALETTE = 0;
static constexpr LayerId DST_PALETTE = 1;
static constexpr LayerId INVALID_PALETTE = 2;
static constexpr size_t PALETTES_COUNT = 2;

using CPS = ConstrainedParameterSpace<RGBf>;
using PPParams = CPS::ParametricPointParams;
using PParams = CPS::PairingParams;
using CParams = CPS::ConstraintParams;
using GCParams = CPS::GlobalConstraintParams;

int main() {
  CPS space(PALETTES_COUNT);

  TEST(space.hasLayer(SRC_PALETTE));
  TEST(space.hasLayer(DST_PALETTE));
  TEST(!space.hasLayer(INVALID_PALETTE));

  std::vector<PPParams> ppparams_invalid; // empty
  std::vector<PPParams> ppparams_src;
  std::vector<PPParams> ppparams_dst;
  ParametricPointIdList ppids;

  ppparams_src.push_back({{0.0f, 0.0f, 0.0f}, 1.0f, 0, nullptr});
  ppparams_src.push_back({{0.5f, 0.5f, 0.5f}, 1.0f, 0, nullptr});
  ppparams_src.push_back({{1.0f, 1.0f, 1.0f}, 1.0f, 0, nullptr});
  TEST(space.addParametricPoints(SRC_PALETTE, ppparams_src, ppids) == ERet_OK);
  TEST(ppids.size() == 3);

  ppparams_dst.push_back({{1.0f, 0.0f, 0.0f}, 1.0f, 0, nullptr});
  ppparams_dst.push_back({{0.0f, 1.0f, 0.0f}, 1.0f, 0, nullptr});
  ppparams_dst.push_back({{0.0f, 0.0f, 1.0f}, 1.0f, 0, nullptr});
  TEST(space.addParametricPoints(DST_PALETTE, ppparams_dst, ppids) == ERet_OK);
  TEST(ppids.size() == 6);

  TEST(space.addParametricPoints(INVALID_PALETTE, ppparams_invalid, ppids) == ERet_BadArg);
  TEST(space.addParametricPoints(SRC_PALETTE, ppparams_invalid, ppids) == ERet_BadArg);
  TEST(space.addParametricPoints(INVALID_PALETTE, ppparams_src, ppids) == ERet_BadArg);

  TEST(space.getAllParametricPointsCount() == ppids.size());
  TEST(space.getLayerParametricPointsCount(SRC_PALETTE) == 3);
  TEST(space.getLayerParametricPointsCount(DST_PALETTE) == 3);
  TEST(space.getAllParametricPointsFunctorsCount() == 0);
  TEST(space.getLayerParametricPointsFunctorsCount(SRC_PALETTE) == 0);
  TEST(space.getLayerParametricPointsFunctorsCount(DST_PALETTE) == 0);
  for(auto id: ppids) {
    TEST(space.hasParametricPoint(id));
  }
  TEST(!space.hasParametricPoint(ParametricPointId_INVALID));

  std::vector<PParams> pparams_invalid; // empty
  std::vector<PParams> pparams;
  PairingIdList pids;

  pparams.push_back({ppids[0], ppids[3]});
  pparams.push_back({ppids[1], ppids[4]});
  pparams.push_back({ppids[2], ppids[5]});
  TEST(space.addPairings(pparams, pids) == ERet_OK);
  TEST(pids.size() == 3);

  TEST(space.addPairings(pparams_invalid, pids) == ERet_BadArg);

  TEST(space.getAllPairingsCount() == pids.size());
  TEST(space.getLayerPairingsCount(SRC_PALETTE) == pids.size());
  TEST(space.getLayerPairingsCount(DST_PALETTE) == pids.size());
  for(auto id: pids) {
    TEST(space.hasPairing(id));
  }
  TEST(!space.hasPairing(PairingId_INVALID));

  std::vector<CParams> cparams_invalid; // empty
  std::vector<CParams> cparams;
  ConstraintIdList cids;

  cparams.push_back({{ppids[0], ppids[1]}, 0, nullptr});
  cparams.push_back({{ppids[1], ppids[2]}, 0, nullptr});
  cparams.push_back({{ppids[3], ppids[4]}, 0, nullptr});
  cparams.push_back({{ppids[4], ppids[5]}, 0, nullptr});
  TEST(space.addConstraints(cparams, cids) == ERet_OK);
  TEST(cids.size() == 4);

  TEST(space.addConstraints(cparams_invalid, cids) == ERet_BadArg);

  TEST(space.getAllConstraintsCount() == cids.size());
  TEST(space.getLayerConstraintsCount(SRC_PALETTE) == 2);
  TEST(space.getLayerConstraintsCount(DST_PALETTE) == 2);
  TEST(space.getAllConstraintsFunctorsCount() == 0);
  TEST(space.getLayerConstraintsFunctorsCount(SRC_PALETTE) == 0);
  TEST(space.getLayerConstraintsFunctorsCount(DST_PALETTE) == 0);
  for(auto id: cids) {
    TEST(space.hasConstraint(id));
  }
  TEST(!space.hasConstraint(ConstraintId_INVALID));

  std::vector<GCParams> gcparams_invalid; // empty
  std::vector<GCParams> gcparams;
  GlobalConstraintIdList gcids;

  gcparams.push_back({nullptr});
  TEST(space.addGlobalConstraints(SRC_PALETTE, gcparams, gcids) == ERet_OK);
  TEST(gcids.size() == 1);
  TEST(space.addGlobalConstraints(DST_PALETTE, gcparams, gcids) == ERet_OK);
  TEST(gcids.size() == 2);

  TEST(space.addGlobalConstraints(INVALID_PALETTE, gcparams_invalid, gcids) == ERet_BadArg);
  TEST(space.addGlobalConstraints(SRC_PALETTE, gcparams_invalid, gcids) == ERet_BadArg);
  TEST(space.addGlobalConstraints(INVALID_PALETTE, gcparams, gcids) == ERet_BadArg);

  TEST(space.getAllGlobalConstraintsCount() == gcids.size());
  TEST(space.getLayerGlobalConstraintsCount(SRC_PALETTE) == 1);
  TEST(space.getLayerGlobalConstraintsCount(DST_PALETTE) == 1);
  TEST(space.getAllGlobalConstraintsFunctorsCount() == 0);
  TEST(space.getLayerGlobalConstraintsFunctorsCount(SRC_PALETTE) == 0);
  TEST(space.getLayerGlobalConstraintsFunctorsCount(DST_PALETTE) == 0);
  for(auto id: gcids) {
    TEST(space.hasGlobalConstraint(id));
  }
  TEST(!space.hasGlobalConstraint(GlobalConstraintId_INVALID));

  return 0;
}
