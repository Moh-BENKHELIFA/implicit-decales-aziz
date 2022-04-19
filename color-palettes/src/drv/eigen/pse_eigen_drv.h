#ifndef PSE_EIGEN_DRV_H
#define PSE_EIGEN_DRV_H

#include "pse_eigen_api.h"

#include <pse_drv.h>

PSE_API_BEGIN

struct pse_eigen_cps_data_t;
struct pse_eigen_cps_exploration_t;
struct pse_cpspace_exploration_ctxt_params_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

struct pse_eigen_device_t {
  struct pse_device_t* clt_dev;
  struct pse_allocator_t* allocator;
  struct pse_logger_t* logger;

  struct pse_eigen_cps_exploration_t** explorations; /* stretchy buffer */
};

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_EIGEN_DEVICE_NULL_                                                 \
  { NULL, NULL, NULL, NULL }

static const struct pse_eigen_device_t PSE_EIGEN_DEVICE_NULL =
  PSE_EIGEN_DEVICE_NULL_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_EIGEN_API enum pse_res_t
pseEigenDriverClean
  (pse_drv_handle_t self);

PSE_EIGEN_API enum pse_res_t
pseEigenDriverCapacityIsManaged
  (pse_drv_handle_t self,
   enum pse_device_capacity_t cap,
   enum pse_type_t type);

PSE_EIGEN_API enum pse_res_t
pseEigenDriverConstrainedParameterSpaceExplorationPrepare
  (pse_drv_handle_t self,
   struct pse_cpspace_exploration_ctxt_t* exp_ctxt,
   struct pse_cpspace_instance_t* icps,
   const struct pse_cpspace_exploration_ctxt_params_t* params,
   pse_drv_exploration_id_t* exp);

PSE_EIGEN_API enum pse_res_t
pseEigenDriverExplorationClean
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t exp);

PSE_EIGEN_API enum pse_res_t
pseEigenDriverExplorationSolve
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t exp,
   struct pse_cpspace_values_data_t* smpls);

PSE_EIGEN_API enum pse_res_t
pseEigenDriverExplorationIterativeSolveBegin
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t exp,
   struct pse_cpspace_values_data_t* smpls);

PSE_EIGEN_API enum pse_res_t
pseEigenDriverExplorationIterativeSolveStep
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t exp);

PSE_EIGEN_API enum pse_res_t
pseEigenDriverExplorationIterativeSolveEnd
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t exp);

PSE_EIGEN_API enum pse_res_t
pseEigenDriverExplorationLastResultsRetreive
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t exp,
   struct pse_cpspace_values_data_t* where,
   struct pse_cpspace_exploration_extra_results_t* extra);

/******************************************************************************
 *
 * PUBLIC ENTRYPOINT
 *
 ******************************************************************************/

PSE_EIGEN_API pse_drv_entrypoint_cb PSE_DRV_ENTRYPOINT_SYMBOL;

PSE_API_END

#endif /* PSE_EIGEN_DRV_H */
