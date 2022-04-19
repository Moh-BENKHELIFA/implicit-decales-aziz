#include "test_utils.h"

#include <clt/pse_clt_cost_L1.h>

struct pse_clt_cost3_ctxt_t {
  struct pse_clt_costN_ctxt_ref_t ref;
  struct pse_clt_cost1_ctxt_t comps[3];
  size_t comps_idx[3];
};

#define PSE_CLT_COST3_CTXT_NULL_                                            \
  { PSE_CLT_COSTN_CTXT_REF_NULL_, {                                         \
    PSE_CLT_COST1_CTXT_DEFAULT_,                                            \
    PSE_CLT_COST1_CTXT_DEFAULT_,                                            \
    PSE_CLT_COST1_CTXT_DEFAULT_                                             \
  }, {0,1,2} }

static const struct pse_clt_cost3_ctxt_t PSE_CLT_COST3_CTXT_NULL =
  PSE_CLT_COST3_CTXT_NULL_;

int main() {
  const pse_real_t val1[3] = {  0, 1, 2 };
  const pse_real_t val2[3] = { 10, 9, 8 };
  struct pse_clt_cost1_ctxt_t ctxt1 = PSE_CLT_COST1_CTXT_DEFAULT;
  struct pse_clt_cost3_ctxt_t ctxt3 = PSE_CLT_COST3_CTXT_NULL;
  size_t idx[3] = { 0, 1, 2 };

  CHECK(pseCltL1SignedDistanceComputeAt(val1, val2, 0, NULL), 10);
  CHECK(pseCltL1SignedDistanceComputeAt(val1, val2, 1, NULL), 8);
  CHECK(pseCltL1SignedDistanceComputeAt(val1, val2, 2, NULL), 6);

  CHECK(pseCltCost1InitFromL1DistanceSignedAt
    (&ctxt1, 0, 20, val1, val2, NULL), RES_OK);
  CHECK(ctxt1.weight, 20);
  CHECK(ctxt1.ref, 10);
  CHECK(pseCltCost1InitFromL1DistanceSignedAt
    (&ctxt1, 1, 30, val1, val2, NULL), RES_OK);
  CHECK(ctxt1.weight, 30);
  CHECK(ctxt1.ref, 8);
  CHECK(pseCltCost1InitFromL1DistanceSignedAt
    (&ctxt1, 2, 40, val1, val2, NULL), RES_OK);
  CHECK(ctxt1.weight, 40);
  CHECK(ctxt1.ref, 6);

  pseCltCostContextRefInitFromBuffer
    (&ctxt3.ref, 3, ctxt3.comps, ctxt3.comps_idx);
  CHECK(pseCltCostRefInitFromL1DistanceSignedPerComponent
    (&ctxt3.ref, 50, val1, val2, NULL, NULL), RES_OK);
  CHECK(ctxt3.comps[0].weight, 50);
  CHECK(ctxt3.comps[0].ref, 10);
  CHECK(ctxt3.comps[1].weight, 50);
  CHECK(ctxt3.comps[1].ref, 8);
  CHECK(ctxt3.comps[2].weight, 50);
  CHECK(ctxt3.comps[2].ref, 6);

  ctxt3 = PSE_CLT_COST3_CTXT_NULL;
  CHECK(pseCltCost1InitFromL1DistanceSignedPerComponent
    (3, ctxt3.comps, idx, 50, val1, val2, NULL, NULL), RES_OK);
  CHECK(ctxt3.comps[0].weight, 50);
  CHECK(ctxt3.comps[0].ref, 10);
  CHECK(ctxt3.comps[1].weight, 50);
  CHECK(ctxt3.comps[1].ref, 8);
  CHECK(ctxt3.comps[2].weight, 50);
  CHECK(ctxt3.comps[2].ref, 6);

  /* TODO: test pseCltCostFuncCompute1_L1 */
  /* TODO: test pseCltCostFuncComputeRefN_L1 */

  return 0;
}
