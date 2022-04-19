#ifndef PSE_CPSPACE_EXPLORATION_P_H
#define PSE_CPSPACE_EXPLORATION_P_H

#include "pse.h"
#include "pse_ref_count.h"
#include "pse_drv.h"

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_cpspace_exploration_ctxt_t {
  struct pse_cpspace_exploration_ctxt_params_t params;
  struct pse_eval_ctxt_t ctxt;

  struct pse_cpspace_instance_t icps;

  pse_drv_exploration_id_t drv_ctxt_id;
  struct pse_drv_t drv; /* A copy in order to keep the locally used driver */
  pse_ref_t ref;
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_CPSPACE_EXPLORATION_CTXT_NULL_                                     \
  { PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL_, PSE_EVAL_CTXT_NULL_,            \
    PSE_CPSPACE_INSTANCE_NULL_,                                                \
    PSE_DRV_EXPLORATION_ID_INVALID_, PSE_DRV_NULL_, 0 }

static const struct pse_cpspace_exploration_ctxt_t PSE_CPSPACE_EXPLORATION_CTXT_NULL =
  PSE_CPSPACE_EXPLORATION_CTXT_NULL_;

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

LOCAL_SYMBOL void
pseConstrainedParameterSpaceExplorationContextDestroy
  (struct pse_cpspace_exploration_ctxt_t* ctxt);

#endif /* PSE_CPSPACE_EXPLORATION_P_H */
