#ifndef PSE_CLT_COST_H
#define PSE_CLT_COST_H

#include "pse_clt_api.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

struct pse_clt_cost1_ctxt_t {
  pse_real_t weight;
  pse_real_t ref;
};

/*! This structure may be used in other structures as an header. For example, if
 * we want 3 components:
 *
 * \code
 * struct pse_clt_cost3_ctxt_t {
 *   struct pse_clt_costN_ctxt_ref_t ref;
 *   struct pse_clt_cost1_ctxt_t comps[3];
 *   size_t comps_idx[3];
 * };
 * \endcode
 *
 * Then, to initialize an object of this type, you can do:
 *
 * \code
 * struct pse_clt_cost3_ctxt_t c;
 * c.ref.count = 3;
 * c.ref.comps = c.comps;
 * c.ref.comps_idx = c.comps_idx;
 * c.comps[0] = ...;
 * c.comps[1] = ...;
 * c.comps[2] = ...;
 * c.comps_idx[0] = ...;
 * c.comps_idx[1] = ...;
 * c.comps_idx[2] = ...;
 * \endcode
 *
 * Using it as an header (first member of the structure) will imply that a
 * pointer on a \p pse_clt_cost3_ctxt_t will also point to a
 * ::pse_clt_costN_ctxt_ref_t.
 */
struct pse_clt_costN_ctxt_ref_t {
  size_t count;
  struct pse_clt_cost1_ctxt_t* comps;
  size_t* comps_idx;
};

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_CLT_COST1_CTXT_DEFAULT_                                            \
  { 1, 0 }
#define PSE_CLT_COSTN_CTXT_REF_NULL_                                           \
  { 0, NULL, NULL }

static const struct pse_clt_cost1_ctxt_t PSE_CLT_COST1_CTXT_DEFAULT =
  PSE_CLT_COST1_CTXT_DEFAULT_;
static const struct pse_clt_costN_ctxt_ref_t PSE_CLT_COSTN_CTXT_REF_NULL =
  PSE_CLT_COSTN_CTXT_REF_NULL_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_CLT_INLINE_API void
pseCltCostContextRefInitFromBuffer
  (struct pse_clt_costN_ctxt_ref_t* ref,
   const size_t count,
   struct pse_clt_cost1_ctxt_t* comps,
   size_t* comps_idx)
{
  assert(ref && comps && comps_idx && (count > 0));
  ref->count = count;
  ref->comps = comps;
  ref->comps_idx = comps_idx;
}

PSE_API_END

#endif /* PSE_CLT_COST_H */
