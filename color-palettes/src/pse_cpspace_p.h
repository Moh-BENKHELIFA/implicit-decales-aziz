#ifndef PSE_CPSPACE_H
#define PSE_CPSPACE_H

#include "pse.h"
#include "pse_ref_count.h"

struct pse_device_t;
struct pse_pspace_t;

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_cpspace_relshp_t {
  struct pse_cpspace_relshp_params_t params;
  enum pse_cpspace_relshp_state_t state;
  pse_clt_relshps_group_uid_t clt_group_uid;
};
struct pse_relshps_group_t {
  pse_clt_relshps_group_uid_t key;
  pse_relshp_id_t* ids; /* stretchy buffer */
};
struct pse_values_entry_t {
  struct pse_cpspace_values_t* key;
};
struct pse_exploration_ctxt_entry_t {
  struct pse_cpspace_exploration_ctxt_t* key;
};

struct pse_cpspace_t {
  pse_clt_pspace_uid_t* pspaces_uid;
  struct pse_pspace_params_t* pspaces;

  /* TODO: instead of using ***_used buffers, we should have a hash map allowing
   * to get the pointer on the structure by using the id. This way, we could
   * have the buffer of ppoints packed, and the ids uncorrelated from this
   * buffer. */

  struct pse_ppoint_params_t* ppoints;
  pse_ppoint_id_t* ppoints_used;
  pse_ppoint_id_t* ppoints_free;

  struct pse_cpspace_relshp_t* relshps;
  pse_relshp_id_t* relshps_used;
  pse_relshp_id_t* relshps_free;

  struct pse_relshps_group_t* relshps_groups; /* ds hash map */

  struct pse_relshp_cost_func_params_t* functors;
  pse_relshp_cost_func_id_t* functors_used;
  pse_relshp_cost_func_id_t* functors_free;

  struct pse_values_entry_t* values; /* ds hash map */
  struct pse_exploration_ctxt_entry_t* exp_ctxts; /* ds hash map */

  struct pse_device_t* dev;
  pse_ref_t ref;
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_CPSPACE_RELSHP_NULL_                                               \
  { PSE_CPSPACE_RELSHP_PARAMS_NULL_, PSE_CPSPACE_RELSHP_STATE_ENABLED,         \
    PSE_CLT_RELSHPS_GROUP_UID_INVALID_ }
#define PSE_RELSHPS_GROUP_NULL_                                                \
  { PSE_CLT_RELSHPS_GROUP_UID_INVALID_, NULL }
#define PSE_CPSPACE_NULL_                                                      \
  { NULL, NULL, /* pspaces */                                                  \
    NULL, NULL, NULL, /* ppoints */                                            \
    NULL, NULL, NULL, NULL, /* relshps */                                      \
    NULL, NULL, NULL, /* functors */                                           \
    NULL, NULL, /* values & exploration ctxts */                               \
    NULL, 0 }

static const struct pse_cpspace_relshp_t PSE_CPSPACE_RELSHP_NULL =
  PSE_CPSPACE_RELSHP_NULL_;
static const struct pse_relshps_group_t PSE_RELSHPS_GROUP_NULL =
  PSE_RELSHPS_GROUP_NULL_;
static const struct pse_cpspace_t PSE_CPSPACE_NULL =
  PSE_CPSPACE_NULL_;

/******************************************************************************
 *
 * PRIVATE API - Constrained Parameter Space
 *
 ******************************************************************************/

/*! Called directly by the device destroy function. Else it's on the last ref
 * release. */
LOCAL_SYMBOL void
pseConstrainedParameterSpaceDestroy
  (struct pse_cpspace_t* cps);

LOCAL_SYMBOL enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesHas
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids);

LOCAL_SYMBOL enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesHasNot
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids);

LOCAL_SYMBOL const struct pse_pspace_params_t*
pseConstrainedParameterSpaceParameterSpaceGet
  (struct pse_cpspace_t* cps,
   const pse_clt_pspace_uid_t uid);

LOCAL_SYMBOL bool
pseConstrainedParameterSpaceParametricPointExists
  (struct pse_cpspace_t* cps,
   pse_ppoint_id_t id);

#endif /* PSE_CPSPACE_H */
