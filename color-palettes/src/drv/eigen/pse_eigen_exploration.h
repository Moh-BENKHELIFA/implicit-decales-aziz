#ifndef PSE_EIGEN_EXPLORATION_H
#define PSE_EIGEN_EXPLORATION_H

#include "pse_eigen_api.h"

PSE_API_BEGIN

struct pse_cpspace_exploration_ctxt_t;
struct pse_cpspace_instance_t;
struct pse_cpspace_exploration_ctxt_params_t;
struct pse_cpspace_exploration_extra_results_t;
struct pse_cpspace_values_data_t;

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_eigen_cps_exploration_t;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_EIGEN_API enum pse_res_t
pseEigenExplorationCreate
  (struct pse_eigen_device_t* dev,
   struct pse_cpspace_exploration_ctxt_t* ctxt,
   const struct pse_cpspace_instance_t* cpsi,
   const struct pse_cpspace_exploration_ctxt_params_t* params,
   struct pse_eigen_cps_exploration_t** exp);

PSE_EIGEN_API enum pse_res_t
pseEigenExplorationDestroy
  (struct pse_eigen_cps_exploration_t* exp);

PSE_EIGEN_API enum pse_res_t
pseEigenExplorationSolveBegin
  (struct pse_eigen_cps_exploration_t* exp,
   const struct pse_cpspace_values_data_t* smpls);

PSE_EIGEN_API enum pse_res_t
pseEigenExplorationSolveStep
  (struct pse_eigen_cps_exploration_t* exp);

PSE_EIGEN_API enum pse_res_t
pseEigenExplorationSolveEnd
  (struct pse_eigen_cps_exploration_t* exp);

PSE_EIGEN_API enum pse_res_t
pseEigenExplorationSolve
  (struct pse_eigen_cps_exploration_t* exp,
   const struct pse_cpspace_values_data_t* smpls);

PSE_EIGEN_API enum pse_res_t
pseEigenExplorationLastResultsRetreive
  (struct pse_eigen_cps_exploration_t* exp,
   struct pse_cpspace_values_data_t* where,
   struct pse_cpspace_exploration_extra_results_t* extra);

PSE_API_END

#endif /* PSE_EIGEN_EXPLORATION_H */
