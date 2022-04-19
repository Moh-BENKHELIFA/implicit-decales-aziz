#include "pse_eigen_drv.h"
#include "pse_eigen_exploration.h"

#include <pse.h>

#include <stretchy_buffer.h>

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static PSE_FINLINE enum pse_res_t
pseEigenDriverExplorationFind
  (struct pse_eigen_device_t* dev,
   struct pse_eigen_cps_exploration_t* exp,
   size_t* idx)
{
  size_t i;
  assert(dev && exp && idx);
  for(i = 0; i < sb_count(dev->explorations); ++i) {
    if( dev->explorations[i] == exp ) {
      *idx = i;
      return RES_OK;
    }
  }
  return RES_NOT_FOUND;
}

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

enum pse_res_t
pseEigenDriverClean
  (pse_drv_handle_t self)
{
  struct pse_eigen_device_t* eigen_dev = (struct pse_eigen_device_t*)self;
  struct pse_logger_t* logger = NULL;
  size_t i;
  if( !eigen_dev )
    return RES_BAD_ARG;
  logger = eigen_dev->logger;

  /* Clean the explorations */
  for(i = 0; i < sb_count(eigen_dev->explorations); ++i) {
    PSE_CALL(pseEigenExplorationDestroy(eigen_dev->explorations[i]));
  }
  sb_free(eigen_dev->explorations);

  PSE_FREE(eigen_dev->allocator, eigen_dev);
  PSE_LOG(logger, DEBUG, "Eigen driver cleaned\n");
  return RES_OK;
}

enum pse_res_t
pseEigenDriverCapacityIsManaged
  (pse_drv_handle_t self,
   enum pse_device_capacity_t cap,
   enum pse_type_t type)
{
  enum pse_res_t res = RES_NOT_SUPPORTED;
  if(  !self
    || (PSEC_FIRST__ > cap)
    || (cap > PSEC_COUNT__)
    || (PSE_TYPE_FIRST__ > type)
    || (type > PSE_TYPE_COUNT__) )
    return RES_BAD_ARG;

  switch(cap) {
    case PSEC_PPOINT_ATTRIB_COORDINATES: {
      res =
           (type == PSE_TYPE_IEEE_754_FLOAT_32)
        || (type == PSE_TYPE_IEEE_754_FLOAT_64)
        ? RES_OK : res;
    } break;
    case PSEC_PPOINT_ATTRIB_LOCK_STATUS: {
      res = (type == PSE_TYPE_BOOL_8) ? RES_OK : res;
    } break;
    default: break;
  }

  return res;
}

enum pse_res_t
pseEigenDriverConstrainedParameterSpaceExplorationPrepare
  (pse_drv_handle_t self,
   struct pse_cpspace_exploration_ctxt_t* exp_ctxt,
   struct pse_cpspace_instance_t* icps,
   const struct pse_cpspace_exploration_ctxt_params_t* params,
   pse_drv_exploration_id_t* out_exp)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_device_t* edev = (struct pse_eigen_device_t*)self;
  if( !edev || !exp_ctxt || !icps || !out_exp )
    return RES_BAD_ARG;

  sb_push(edev->explorations, NULL);
  PSE_CALL_OR_GOTO(res,error, pseEigenExplorationCreate
    (edev, exp_ctxt, icps, params, &sb_last(edev->explorations)));

  *out_exp = (pse_drv_exploration_id_t)sb_last(edev->explorations);

exit:
  return res;
error:
  sb_pop(edev->explorations);
  goto exit;
}

enum pse_res_t
pseEigenDriverExplorationClean
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t in_exp)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_device_t* edev = (struct pse_eigen_device_t*)self;
  struct pse_eigen_cps_exploration_t* exp = NULL;
  size_t idx;
  if( !edev || (in_exp == PSE_DRV_EXPLORATION_ID_INVALID) )
    return RES_BAD_ARG;

  exp = (struct pse_eigen_cps_exploration_t*)in_exp;
  PSE_CALL_OR_RETURN(res, pseEigenDriverExplorationFind(edev, exp, &idx));
  PSE_CALL_OR_RETURN(res, pseEigenExplorationDestroy(exp));
  sb_delat(edev->explorations, idx);
  return RES_OK;
}

enum pse_res_t
pseEigenDriverExplorationSolve
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t in_exp,
   struct pse_cpspace_values_data_t* smpls)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_device_t* edev = (struct pse_eigen_device_t*)self;
  struct pse_eigen_cps_exploration_t* exp = NULL;
  if( !edev || !smpls || (in_exp == PSE_DRV_EXPLORATION_ID_INVALID) )
    return RES_BAD_ARG;

  /* TODO: to avoid a linear search each time, we do not check that the
   * expolration is one of ours. Can we do a simple check? */
  exp = (struct pse_eigen_cps_exploration_t*)in_exp;
  PSE_TRY_CALL_OR_RETURN(res, pseEigenExplorationSolve(exp, smpls));
  return res;
}

enum pse_res_t
pseEigenDriverExplorationIterativeSolveBegin
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t in_exp,
   struct pse_cpspace_values_data_t* smpls)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_device_t* edev = (struct pse_eigen_device_t*)self;
  struct pse_eigen_cps_exploration_t* exp = NULL;
  if( !edev || !smpls || (in_exp == PSE_DRV_EXPLORATION_ID_INVALID) )
    return RES_BAD_ARG;

  /* TODO: to avoid a linear search each time, we do not check that the
   * expolration is one of ours. Can we do a simple check? */
  exp = (struct pse_eigen_cps_exploration_t*)in_exp;
  PSE_TRY_CALL_OR_RETURN(res, pseEigenExplorationSolveBegin(exp, smpls));
  return res;
}

enum pse_res_t
pseEigenDriverExplorationIterativeSolveStep
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t in_exp)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_device_t* edev = (struct pse_eigen_device_t*)self;
  struct pse_eigen_cps_exploration_t* exp = NULL;
  if( !edev || (in_exp == PSE_DRV_EXPLORATION_ID_INVALID) )
    return RES_BAD_ARG;

  /* TODO: to avoid a linear search each time, we do not check that the
   * expolration is one of ours. Can we do a simple check? */
  exp = (struct pse_eigen_cps_exploration_t*)in_exp;
  PSE_TRY_CALL_OR_RETURN(res, pseEigenExplorationSolveStep(exp));
  return res;
}

enum pse_res_t
pseEigenDriverExplorationIterativeSolveEnd
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t in_exp)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_device_t* edev = (struct pse_eigen_device_t*)self;
  struct pse_eigen_cps_exploration_t* exp = NULL;
  if( !edev || (in_exp == PSE_DRV_EXPLORATION_ID_INVALID) )
    return RES_BAD_ARG;

  /* TODO: to avoid a linear search each time, we do not check that the
   * expolration is one of ours. Can we do a simple check? */
  exp = (struct pse_eigen_cps_exploration_t*)in_exp;
  PSE_CALL_OR_RETURN(res, pseEigenExplorationSolveEnd(exp));
  return res;
}

enum pse_res_t
pseEigenDriverExplorationLastResultsRetreive
  (pse_drv_handle_t self,
   pse_drv_exploration_id_t in_exp,
   struct pse_cpspace_values_data_t* where,
   struct pse_cpspace_exploration_extra_results_t* extra)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_device_t* eigen_dev = (struct pse_eigen_device_t*)self;
  struct pse_eigen_cps_exploration_t* exp = NULL;
  if( !eigen_dev || !where || (in_exp == PSE_DRV_EXPLORATION_ID_INVALID) )
    return RES_BAD_ARG;

  /* TODO: to avoid a linear search each time, we do not check that the
   * expolration is one of ours. Can we do a simple check? */
  exp = (struct pse_eigen_cps_exploration_t*)in_exp;
  PSE_CALL_OR_RETURN(res, pseEigenExplorationLastResultsRetreive
    (exp, where, extra));
  return res;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

static enum pse_res_t
pseDriverEigenEntryPoint
  (struct pse_device_t* dev,
   struct pse_drv_params_t* params,
   struct pse_drv_t* drv)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_device_t* eigen_dev = NULL;
  if( !dev || !params || !drv )
    return RES_BAD_ARG;

  eigen_dev = PSE_TYPED_ALLOC(params->allocator, struct pse_eigen_device_t);
  PSE_VERIFY_OR_ELSE(eigen_dev != NULL, res = RES_MEM_ERR; goto error);
  *eigen_dev = PSE_EIGEN_DEVICE_NULL;
  eigen_dev->clt_dev = dev;
  eigen_dev->allocator = params->allocator;
  eigen_dev->logger = params->logger;

  /* Fill the API structure */
  drv->self = (pse_drv_handle_t)eigen_dev;
  drv->clean = pseEigenDriverClean;
  drv->is_capacity_managed = pseEigenDriverCapacityIsManaged;
  drv->cpspace_exploration_prepare = pseEigenDriverConstrainedParameterSpaceExplorationPrepare;
  drv->exploration_clean = pseEigenDriverExplorationClean;
  drv->exploration_solve = pseEigenDriverExplorationSolve;
  drv->exploration_solve_iterative_begin = pseEigenDriverExplorationIterativeSolveBegin;
  drv->exploration_solve_iterative_step = pseEigenDriverExplorationIterativeSolveStep;
  drv->exploration_solve_iterative_end = pseEigenDriverExplorationIterativeSolveEnd;
  drv->exploration_last_results_retreive = pseEigenDriverExplorationLastResultsRetreive;

  PSE_LOG(eigen_dev->logger, DEBUG, "Eigen driver loaded\n");

exit:
  return res;
error:
  PSE_FREE(params->allocator, eigen_dev);
  goto exit;
}

pse_drv_entrypoint_cb PSE_DRV_ENTRYPOINT_SYMBOL =
  pseDriverEigenEntryPoint;
