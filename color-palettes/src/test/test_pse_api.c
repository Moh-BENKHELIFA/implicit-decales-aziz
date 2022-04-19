#include "test_utils.h"

#include <pse.h>

/* TODO: test around cost functors how many things have changed. */
/* TODO: test the contexts init and clean by using global counters. */
/* TODO: test optional parameters (contexts). */

struct Thing {
  float f;
  int i;
  double d;
};

static enum pse_res_t
thingCost
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  size_t i;
  (void)eval_ctxt, (void)eval_coords;
  for(i = 0; i < eval_relshps->count; ++i) {
    costs[i*3+0] = 123.0;
    costs[i*3+1] = 50.0;
    costs[i*3+2] = -13.25;
  }
  return RES_OK;
}

static enum pse_res_t
thingAttribGet
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   void* attrib_values)
{
  struct Thing* values = (struct Thing*)ctxt;
  pse_real_t* attribs = (pse_real_t*)attrib_values;
  size_t i;
  assert(attrib == PSE_POINT_ATTRIB_COORDINATES);
  assert(as_type == PSE_TYPE_REAL);
  (void)attrib, (void)as_type;

  for(i = 0; i < count; ++i) {
    attribs[i*3+0] = (pse_real_t)values[values_idx[i]].f;
    attribs[i*3+1] = (pse_real_t)values[values_idx[i]].i;
    attribs[i*3+2] = (pse_real_t)values[values_idx[i]].d;
  }
  return RES_OK;
}

static enum pse_res_t
thingAttribSet
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   const void* attrib_values)
{
  struct Thing* values = (struct Thing*)ctxt;
  const pse_real_t* attribs = (pse_real_t*)attrib_values;
  size_t i;
  assert(attrib == PSE_POINT_ATTRIB_COORDINATES);
  assert(as_type == PSE_TYPE_REAL);
  (void)attrib, (void)as_type;

  for(i = 0; i < count; ++i) {
    values[values_idx[i]].f =  (float)attribs[i*3+0];
    values[values_idx[i]].i =    (int)attribs[i*3+1];
    values[values_idx[i]].d = (double)attribs[i*3+2];
  }
  return RES_OK;

}

#define THING_DEFAULT_ { 0.0f, 0, 0.0 }

int main(int argc, char** argv) {
  struct pse_logger_t* logger = &PSE_LOGGER_STDOUT;

  struct Thing ppsmpls[] = {
    { 34489.31f, 998, 6548.1 },
    { 9005.1f, 2, 97816543.2574 },
    { 0.879546f, 6697432, 0.0000541 }
  };
  struct Thing resvalues[] = {
    THING_DEFAULT_,
    THING_DEFAULT_,
    THING_DEFAULT_
  };

  struct pse_device_params_t dparams = PSE_DEVICE_PARAMS_NULL;
  pse_clt_pspace_uid_t psuids_invalid[] = { PSE_CLT_PSPACE_UID_INVALID_ };
  pse_clt_pspace_uid_t psuids[] = { PSE_CLT_PSPACE_UID_INVALID_ };
  pse_clt_ppoint_variation_uid_t ppvuids[] = { PSE_CLT_PPOINT_VARIATION_UID_INVALID_ };
  struct pse_pspace_params_t psparams[] = { PSE_PSPACE_PARAMS_NULL_ };
  struct pse_cpspace_params_t cpsparams = PSE_CPSPACE_PARAMS_NULL;
  struct pse_relshp_cost_func_params_t cfparams[] = {
    PSE_RELSHP_COST_FUNC_PARAMS_NULL_
  };
  struct pse_ppoint_params_t ppparams[] = {
    PSE_PPOINT_PARAMS_NULL_,
    PSE_PPOINT_PARAMS_NULL_,
    PSE_PPOINT_PARAMS_NULL_
  };
  struct pse_cpspace_relshp_params_t rparams[] = {
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_
  };
  struct pse_cpspace_relshp_params_t crparams[] = {
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_
  };
  struct pse_cpspace_exploration_ctxt_params_t exparams =
    PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL;
  struct pse_cpspace_exploration_samples_t exsmpls =
    PSE_CPSPACE_EXPLORATION_SAMPLES_NULL;

  struct pse_device_t* dev = NULL;
  struct pse_cpspace_t* cps = NULL;
  struct pse_cpspace_values_t* smpls = NULL;
  struct pse_cpspace_values_t* vals = NULL;

  struct pse_pspace_point_attrib_component_t ppacomps[] = {
    { PSE_TYPE_FLOAT },
    { PSE_TYPE_INT_32 },
    { PSE_TYPE_DOUBLE }
  };
  struct pse_cpspace_values_data_t valsd = PSE_CPSPACE_VALUES_DATA_NULL;
  struct pse_cpspace_values_data_t* vdata = NULL;
  struct pse_cpspace_values_data_t* vdatac = NULL;
  struct pse_cpspace_values_data_t* vdatainv = &valsd;
  const struct pse_cpspace_values_lock_params_t* valslp = NULL;
  pse_relshp_cost_func_id_t cfids[] = {
    PSE_RELSHP_COST_FUNC_ID_INVALID_
  };
  pse_ppoint_id_t ppids[] = {
    PSE_PPOINT_ID_INVALID_,
    PSE_PPOINT_ID_INVALID_,
    PSE_PPOINT_ID_INVALID_
  };
  pse_ppoint_id_t pppairs[3][2] = {
    { PSE_PPOINT_ID_INVALID_, PSE_PPOINT_ID_INVALID_ },
    { PSE_PPOINT_ID_INVALID_, PSE_PPOINT_ID_INVALID_ },
    { PSE_PPOINT_ID_INVALID_, PSE_PPOINT_ID_INVALID_ }
  };
  pse_relshp_id_t irids[] = {
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_
  };
  pse_relshp_id_t rids[] = {
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_
  };
  pse_relshp_id_t crids[] = {
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_
  };
  enum pse_cpspace_relshp_state_t rss[] = {
    PSE_CPSPACE_RELSHP_STATE_DISABLED,
    PSE_CPSPACE_RELSHP_STATE_DISABLED,
    PSE_CPSPACE_RELSHP_STATE_DISABLED,
    PSE_CPSPACE_RELSHP_STATE_DISABLED,
    PSE_CPSPACE_RELSHP_STATE_DISABLED,
    PSE_CPSPACE_RELSHP_STATE_DISABLED,
    PSE_CPSPACE_RELSHP_STATE_DISABLED,
    PSE_CPSPACE_RELSHP_STATE_DISABLED,
    PSE_CPSPACE_RELSHP_STATE_DISABLED
  };
  enum pse_cpspace_relshps_group_state_t rgs =
    PSE_CPSPACE_RELSHPS_GROUP_STATE_DISABLED;
  struct pse_cpspace_exploration_ctxt_t* exctxt = NULL;
  const pse_clt_relshps_group_uid_t irguid = PSE_CLT_RELSHPS_GROUP_UID_INVALID;
  const pse_clt_relshps_group_uid_t rguid = 0;

  size_t i, j, k;

  (void)argc, (void)argv;

  /****************************************************************************
   * Test API - Device
   ****************************************************************************/
  dparams.logger = logger;
  dparams.logger->level = PSE_LOGGER_LEVEL_DEBUG;

  CHECK(pseDeviceCreate(NULL, NULL), RES_BAD_ARG);
  CHECK(pseDeviceCreate(NULL, &dev), RES_BAD_ARG);
  CHECK(pseDeviceCreate(&dparams, NULL), RES_BAD_ARG);
  CHECK(pseDeviceCreate(&dparams, &dev), RES_BAD_ARG);

  dparams.backend_drv_filepath = PSE_LIB_NAME("pse-drv-does-not-exist");
  CHECK(pseDeviceCreate(&dparams, &dev), RES_IO_ERR);

  dparams.backend_drv_filepath = PSE_LIB_NAME("pse-drv-eigen-ref");
  CHECK(pseDeviceCreate(&dparams, &dev), RES_OK);

  CHECK(pseDeviceDestroy(NULL), RES_BAD_ARG);
  CHECK(pseDeviceDestroy(dev), RES_OK);

  CHECK(pseDeviceCreate(&dparams, &dev), RES_OK);

  CHECK(pseDeviceCapacityIsManaged
    (NULL, PSEC_NONE, PSE_TYPE_NONE), RES_BAD_ARG);
  CHECK(pseDeviceCapacityIsManaged
    (dev, PSEC_NONE, PSE_TYPE_NONE), RES_BAD_ARG);
  CHECK(pseDeviceCapacityIsManaged
    (NULL, PSEC_PPOINT_ATTRIB_COORDINATES, PSE_TYPE_REAL), RES_BAD_ARG);
  CHECK(pseDeviceCapacityIsManaged
    (dev, PSEC_PPOINT_ATTRIB_COORDINATES, PSE_TYPE_REAL), RES_OK);

  /****************************************************************************
   * Test API - Constrained Parameter Space
   ****************************************************************************/
  CHECK(pseConstrainedParameterSpaceCreate(NULL, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceCreate(dev, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceCreate(NULL, &cpsparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceCreate(NULL, NULL, &cps), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceCreate(dev, &cpsparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceCreate(dev, NULL, &cps), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceCreate(NULL, &cpsparams, &cps), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceCreate(dev, &cpsparams, &cps), RES_OK);

  CHECK(pseConstrainedParameterSpaceRefAdd(NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRefAdd(cps), RES_OK);

  CHECK(pseConstrainedParameterSpaceRefSub(NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRefSub(cps), RES_OK);

  /****************************************************************************
   * Test API - Parameter Space
   ****************************************************************************/
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (cps, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (NULL, 0, psuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (cps, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (cps, 0, psuids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (NULL, 1, psuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (cps, 1, psuids), RES_BAD_ARG);

  psuids[0] = 1;
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (cps, 1, psuids), RES_NOT_FOUND);
  psuids[0] = PSE_CLT_PSPACE_UID_INVALID;

  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (NULL, 0, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 0, NULL, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (NULL, 1, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (NULL, 0, psuids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (NULL, 0, NULL, psparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 0, psuids, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 0, NULL, psparams), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (NULL, 1, psuids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (NULL, 1, NULL, psparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (NULL, 0, psuids, psparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, psuids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, NULL, psparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (NULL, 1, psuids, psparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, psuids, psparams), RES_BAD_ARG);

  psuids[0] = 1;
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, psuids, psparams), RES_BAD_ARG);

  psparams[0].ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components_count = 3;
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, psuids, psparams), RES_BAD_ARG);

  psparams[0].ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components = ppacomps;
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, psuids, psparams), RES_OK);

  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (cps, 1, psuids_invalid), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (cps, 1, psuids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesForget
    (cps, 1, psuids), RES_NOT_FOUND);

  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, psuids_invalid, psparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, psuids, psparams), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 1, psuids, psparams), RES_ALREADY_EXISTS);

  psuids[0] = PSE_CLT_PSPACE_UID_INVALID;
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 0, NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 0, NULL, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 1, NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 0, psuids, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 0, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 0, NULL, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 0, psuids, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 0, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 0, NULL, 0, ppvuids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 1, psuids, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 1, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 1, NULL, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 0, psuids, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 0, psuids, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 0, NULL, 1, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, NULL, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 1, psuids, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 1, psuids, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 0, psuids, 1, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (NULL, 1, psuids, 1, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 1, ppvuids), RES_BAD_ARG);

  psuids[0] = 1;
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 0, ppvuids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 1, ppvuids), RES_BAD_ARG);

  ppvuids[0] = 1;
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 1, ppvuids), RES_NOT_FOUND);

  psuids[0] = PSE_CLT_PSPACE_UID_INVALID;
  ppvuids[0] = PSE_CLT_PPOINT_VARIATION_UID_INVALID;
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 0, NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 0, NULL, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 1, NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 0, psuids, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 0, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 0, NULL, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 0, psuids, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 0, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 0, NULL, 0, ppvuids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 1, psuids, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 1, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 1, NULL, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 0, psuids, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 0, psuids, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 0, NULL, 1, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, psuids, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, NULL, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 1, psuids, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 1, psuids, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 0, psuids, 1, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, psuids, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, psuids, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (NULL, 1, psuids, 1, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, psuids, 1, ppvuids), RES_BAD_ARG);

  psuids[0] = 1;
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, psuids, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, psuids, 0, ppvuids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, psuids, 1, ppvuids), RES_BAD_ARG);

  ppvuids[0] = 1;
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, psuids, 1, ppvuids), RES_OK);

  psuids[0] = PSE_CLT_PSPACE_UID_INVALID;
  ppvuids[0] = PSE_CLT_PPOINT_VARIATION_UID_INVALID;
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 0, NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 0, NULL, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 1, NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 0, psuids, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 0, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 0, NULL, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 0, psuids, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 0, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 0, NULL, 0, ppvuids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 1, psuids, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 1, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 1, NULL, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 0, psuids, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 0, psuids, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 0, NULL, 1, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, psuids, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, NULL, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 1, psuids, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 1, psuids, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 0, psuids, 1, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, psuids, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, psuids, 0, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (NULL, 1, psuids, 1, ppvuids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, psuids, 1, ppvuids), RES_BAD_ARG);

  psuids[0] = 1;
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, psuids, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, psuids, 0, ppvuids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, psuids, 1, ppvuids), RES_BAD_ARG);

  ppvuids[0] = 1;
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsAdd
    (cps, 1, psuids, 1, ppvuids), RES_OK);

  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 1, ppvuids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemove
    (cps, 1, psuids, 1, ppvuids), RES_NOT_FOUND);
  CHECK(pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
    (cps, 1, psuids, 1, ppvuids), RES_OK);

  /****************************************************************************
   * Test API - Constrained Parameter Space Parametric Points
   ****************************************************************************/
  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (cps, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (NULL, 3, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (NULL, 0, ppids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (cps, 3, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (cps, 0, ppids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (NULL, 3, ppids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (cps, 3, ppids), RES_BAD_ARG);

  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (NULL, 0, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 0, NULL, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (NULL, 3, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (NULL, 0, ppparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (NULL, 0, NULL, ppids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 3, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 0, ppparams, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 0, NULL, ppids), RES_OK);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (NULL, 3, ppparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (NULL, 3, NULL, ppids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (NULL, 0, ppparams, ppids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 3, ppparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 3, NULL, ppids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (NULL, 3, ppparams, ppids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 3, ppparams, ppids), RES_OK);

  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (cps, 3, ppids), RES_OK);

  CHECK(pseConstrainedParameterSpaceParametricPointsClear(NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsClear(cps), RES_OK);

  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 3, ppparams, ppids), RES_OK);

  CHECK(pseConstrainedParameterSpaceParametricPointsClear(cps), RES_OK);

  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 3, ppparams, ppids), RES_OK);

  /****************************************************************************
   * Test API - Constrained Parameter Space Cost Functors
   ****************************************************************************/
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (cps, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (NULL, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (NULL, 0, cfids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (cps, 1, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (cps, 0, cfids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (NULL, 1, cfids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (cps, 1, cfids), RES_BAD_ARG);

  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (NULL, 0, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 0, NULL, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (NULL, 1, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (NULL, 0, cfparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (NULL, 0, NULL, cfids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 0, cfparams, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 0, NULL, cfids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (NULL, 1, cfparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (NULL, 1, NULL, cfids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (NULL, 0, cfparams, cfids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, cfparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, NULL, cfids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 0, cfparams, cfids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, cfparams, cfids), RES_BAD_ARG);

  cfparams[0].compute = thingCost;
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, cfparams, cfids), RES_BAD_ARG);

  cfparams[0].cost_arity_mode = PSE_COST_ARITY_MODE_PER_RELATIONSHIP;
  cfparams[0].costs_count = 3;
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, cfparams, cfids), RES_BAD_ARG);

  cfparams[0].expected_pspace = psuids[0];
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, cfparams, cfids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (cps, 1, cfids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, cfparams, cfids), RES_OK);

  /****************************************************************************
   * Test API - Constrained Parameter Space Relationships
   ****************************************************************************/
  CHECK(pseConstrainedParameterSpaceRelationshipsRemove
    (NULL, 0, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemove
    (cps, 0, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemove
    (NULL, 9, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemove
    (NULL, 0, rids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemove
    (cps, 9, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemove
    (cps, 0, rids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemove
    (NULL, 9, rids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemove
    (cps, 9, rids), RES_BAD_ARG);

  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (NULL, irguid, 0, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 0, NULL, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (NULL, irguid, 9, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (NULL, irguid, 0, rparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (NULL, irguid, 0, NULL, rids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 9, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 0, rparams, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 0, NULL, rids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (NULL, irguid, 9, rparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (NULL, irguid, 9, NULL, rids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (NULL, irguid, 0, rparams, rids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 9, rparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 9, NULL, rids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (NULL, irguid, 9, rparams, rids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 9, rparams, rids), RES_BAD_ARG);

  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(NULL), 0);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 0);

  pppairs[0][0] = ppids[0];
  pppairs[0][1] = ppids[1];
  pppairs[1][0] = ppids[0];
  pppairs[1][1] = ppids[2];
  pppairs[2][0] = ppids[1];
  pppairs[2][1] = ppids[2];

  for(j = 0; j < 3; ++j) {
    for(i = 0; i < 3; ++i) {
      k = i + j*3;
      rparams[k].kind = PSE_RELSHP_KIND_INCLUSIVE;
      rparams[k].ppoints_count = 2;
      rparams[k].ppoints_id = pppairs[j];
      rparams[k].cnstrs.funcs_count = 1;
      rparams[k].cnstrs.funcs = cfids;  /* Same cost function for everyone */
    }
  }

  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 9, rparams, rids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 9);

  CHECK(pseConstrainedParameterSpaceRelationshipsRemove
    (cps, 9, rids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 0);

  CHECK(pseConstrainedParameterSpaceRelationshipsClear(NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsClear(cps), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 0);

  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 9, rparams, rids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 9);

  CHECK(pseConstrainedParameterSpaceRelationshipsClear(cps), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 0);

  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, rguid, 9, rparams, rids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 9);

  CHECK(pseConstrainedParameterSpaceRelationshipsRemoveByGroup
    (NULL, irguid), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemoveByGroup
    (cps, irguid), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemoveByGroup
    (NULL, rguid), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemoveByGroup
    (cps, rguid), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 0);

  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, rguid+0, 3, &rparams[0], &rids[0]), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 3);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, rguid+1, 3, &rparams[3], &rids[3]), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 6);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, rguid+2, 3, &rparams[6], &rids[6]), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 9);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemoveByGroup
    (cps, rguid+666), RES_NOT_FOUND);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemoveByGroup
    (cps, rguid+1), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 6);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemoveByGroup
    (cps, rguid+2), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 3);
  CHECK(pseConstrainedParameterSpaceRelationshipsRemoveByGroup
    (cps, rguid), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsCountGet(cps), 0);

  /* With destroyed functors **************************************************/
  CHECK(pseConstrainedParameterSpaceRelationshipsClear(cps), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (cps, 1, cfids), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 9, rparams, rids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, cfparams, cfids), RES_OK);
  for(j = 0; j < 3; ++j) {
    for(i = 0; i < 3; ++i) {
      k = i + j*3;
      rparams[k].cnstrs.funcs = cfids;  /* We use the new id */
    }
  }
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 9, rparams, rids), RES_OK);

  /* With destroyed ppoints ***************************************************/
  CHECK(pseConstrainedParameterSpaceRelationshipsClear(cps), RES_OK);
  CHECK(pseConstrainedParameterSpaceParametricPointsRemove
    (cps, 1, &ppids[1]), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, irguid, 9, rparams, rids), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceParametricPointsClear(cps), RES_OK);
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 3, ppparams, ppids), RES_OK);
  pppairs[0][0] = ppids[0];
  pppairs[0][1] = ppids[1];
  pppairs[1][0] = ppids[0];
  pppairs[1][1] = ppids[2];
  pppairs[2][0] = ppids[1];
  pppairs[2][1] = ppids[2];
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, rguid, 9, rparams, rids), RES_OK);
  /****************************************************************************/

  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (NULL, 0, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (cps, 0, NULL, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (NULL, 4, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (NULL, 0, crids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (NULL, 0, NULL, crparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (cps, 4, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (cps, 0, crids, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (cps, 0, NULL, crparams), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (NULL, 4, crids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (NULL, 4, NULL, crparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (NULL, 0, crids, crparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (cps, 4, crids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (cps, 4, NULL, crparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (NULL, 4, crids, crparams), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (cps, 4, crids, crparams), RES_BAD_ARG);

  i = 0; j = 1;
  crids[i] = rids[j]; ++i; j+=2;
  crids[i] = rids[j]; ++i; j+=2;
  crids[i] = rids[j]; ++i; j+=2;
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (cps, 4, crids, crparams), RES_BAD_ARG);
  crids[i] = rids[j];
  CHECK(pseConstrainedParameterSpaceRelationshipsParamsGet
    (cps, 4, crids, crparams), RES_OK);
  i = 0; j = 1;
  for(; i < 4; ++i, j+=2) {
    CHECK(rparams[j].kind, crparams[i].kind);
    CHECK(rparams[j].ppoints_count, crparams[i].ppoints_count);
    CHECK(rparams[j].variations_count, crparams[i].variations_count);
    CHECK(rparams[j].cnstrs.funcs_count, crparams[i].cnstrs.funcs_count);
    for(k = 0; k < rparams[j].ppoints_count; ++k) {
      CHECK(rparams[j].ppoints_id[k], crparams[i].ppoints_id[k]);
    }
    for(k = 0; k < rparams[i].variations_count; ++k) {
      CHECK(rparams[j].variations[k], crparams[i].variations[k]);
    }
    for(k = 0; k < rparams[i].cnstrs.funcs_count; ++k) {
      CHECK(rparams[j].cnstrs.funcs[k], crparams[i].cnstrs.funcs[k]);
    }
  }

  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (NULL, 0, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 0, NULL, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (NULL, 9, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (NULL, 0, rids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (NULL, 0, NULL, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 9, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 0, rids, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 0, NULL, rss), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (NULL, 9, rids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (NULL, 9, NULL, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 9, rids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 9, NULL, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (NULL, 9, rids, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 9, irids, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 9, rids, rss), RES_OK);
  for(i = 0; i < 9; ++i) {
    CHECK(rss[i], PSE_CPSPACE_RELSHP_STATE_ENABLED);
  }

  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (NULL, irguid, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (cps, irguid, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (NULL, rguid, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (NULL, irguid, &rgs), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (cps, rguid, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (cps, irguid, &rgs), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (NULL, rguid, &rgs), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (cps, rguid, &rgs), RES_OK);
  CHECK(rgs, PSE_CPSPACE_RELSHPS_GROUP_STATE_ENABLED);

  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (NULL, 0, NULL, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (cps, 0, NULL, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (NULL, 1, NULL, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (NULL, 0, rids, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (cps, 1, NULL, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (cps, 0, rids, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (NULL, 1, rids, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (cps, 1, irids, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (cps, 1, rids, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 9, rids, rss), RES_OK);
  CHECK(rss[0], PSE_CPSPACE_RELSHP_STATE_DISABLED);
  for(i = 1; i < 9; ++i) {
    CHECK(rss[i], PSE_CPSPACE_RELSHP_STATE_ENABLED);
  }

  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (cps, rguid, &rgs), RES_OK);
  CHECK(rgs, PSE_CPSPACE_RELSHPS_GROUP_STATE_ENABLED_PARTIALLY);

  CHECK(pseConstrainedParameterSpaceRelationshipsSameStateSet
    (cps, 9, rids, PSE_CPSPACE_RELSHP_STATE_DISABLED), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 9, rids, rss), RES_OK);
  for(i = 0; i < 9; ++i) {
    CHECK(rss[i], PSE_CPSPACE_RELSHP_STATE_DISABLED);
  }
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (cps, rguid, &rgs), RES_OK);
  CHECK(rgs, PSE_CPSPACE_RELSHPS_GROUP_STATE_DISABLED);

  CHECK(pseConstrainedParameterSpaceRelationshipsStateSetByGroup
    (NULL, irguid, PSE_CPSPACE_RELSHP_STATE_ENABLED), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSetByGroup
    (cps, irguid, PSE_CPSPACE_RELSHP_STATE_ENABLED), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSetByGroup
    (NULL, rguid, PSE_CPSPACE_RELSHP_STATE_ENABLED), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSetByGroup
    (cps, rguid, PSE_CPSPACE_RELSHP_STATE_ENABLED), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 9, rids, rss), RES_OK);
  for(i = 0; i < 9; ++i) {
    CHECK(rss[i], PSE_CPSPACE_RELSHP_STATE_ENABLED);
  }
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (cps, rguid, &rgs), RES_OK);
  CHECK(rgs, PSE_CPSPACE_RELSHPS_GROUP_STATE_ENABLED);

  crids[0] = rids[1];
  crids[1] = rids[3];
  crids[2] = rids[5];
  crids[3] = rids[7];
  rss[0] = PSE_CPSPACE_RELSHP_STATE_DISABLED;
  rss[1] = PSE_CPSPACE_RELSHP_STATE_DISABLED;
  rss[2] = PSE_CPSPACE_RELSHP_STATE_DISABLED;
  rss[3] = PSE_CPSPACE_RELSHP_STATE_DISABLED;
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (NULL, 0, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (cps, 0, NULL, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (NULL, 4, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (NULL, 0, crids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (NULL, 0, NULL, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (cps, 4, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (cps, 0, crids, NULL), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (cps, 0, NULL, rss), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (NULL, 4, crids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (NULL, 4, NULL, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (NULL, 0, crids, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (cps, 4, crids, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (cps, 4, NULL, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (NULL, 4, crids, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (cps, 4, irids, rss), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateSet
    (cps, 4, crids, rss), RES_OK);
  CHECK(pseConstrainedParameterSpaceRelationshipsStateGet
    (cps, 9, rids, rss), RES_OK);
  for(i = 0; i < 9; ++i) {
    CHECK(rss[i], (i % 2) == 1
      ? PSE_CPSPACE_RELSHP_STATE_DISABLED
      : PSE_CPSPACE_RELSHP_STATE_ENABLED);
  }
  CHECK(pseConstrainedParameterSpaceRelationshipsGroupStateGet
    (cps, rguid, &rgs), RES_OK);
  CHECK(rgs, PSE_CPSPACE_RELSHPS_GROUP_STATE_ENABLED_PARTIALLY);

  /****************************************************************************
   * Test API - Constrained Parameter Space Values
   ****************************************************************************/
  CHECK(pseConstrainedParameterSpaceValuesCreate(NULL, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesCreate(NULL, &valsd, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesCreate(NULL, NULL, &vals), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, &valsd, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, NULL, &vals), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesCreate(NULL, &valsd, &vals), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, &valsd, &vals), RES_BAD_ARG);

  valsd.storage = PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_PER_ATTRIB;
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, &valsd, &vals), RES_BAD_ARG);

  valsd.pspace = psuids[0];
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, &valsd, &vals), RES_BAD_ARG);

  valsd.as.per_attrib.accessors[PSE_POINT_ATTRIB_COORDINATES].get = thingAttribGet;
  valsd.as.per_attrib.accessors[PSE_POINT_ATTRIB_COORDINATES].set = thingAttribSet;
  valsd.as.per_attrib.accessors[PSE_POINT_ATTRIB_COORDINATES].ctxt = (void*)resvalues;
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, &valsd, &vals), RES_OK);

  CHECK(pseConstrainedParameterSpaceValuesRefAdd(NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesRefAdd(vals), RES_OK);

  CHECK(pseConstrainedParameterSpaceValuesRefSub(NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesRefSub(vals), RES_OK);

  valslp = &PSE_CPSPACE_VALUES_LOCK_PARAMS_READ;
  CHECK(pseConstrainedParameterSpaceValuesLock(NULL, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesLock(vals, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesLock(NULL, valslp, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesLock(NULL, NULL, &vdata), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesLock(vals, valslp, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesLock(vals, NULL, &vdata), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesLock(NULL, valslp, &vdata), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesLock(vals, valslp, &vdata), RES_OK);
  /* TODO: this call may return OK when many readers will be accepted at the
   * same time */
  CHECK(pseConstrainedParameterSpaceValuesLock(vals, valslp, &vdata), RES_BUSY);
  vdatac = vdata;

  CHECK(pseConstrainedParameterSpaceValuesUnlock(NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesUnlock(vals, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesUnlock(NULL, &vdata), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesUnlock(vals, &vdatainv), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesUnlock(vals, &vdata), RES_OK);
  CHECK(pseConstrainedParameterSpaceValuesUnlock(vals, &vdata), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceValuesUnlock(vals, &vdatac), RES_INVALID);

  valslp = &PSE_CPSPACE_VALUES_LOCK_PARAMS_WRITE;
  CHECK(pseConstrainedParameterSpaceValuesLock(vals, valslp, &vdata), RES_OK);
  CHECK(pseConstrainedParameterSpaceValuesLock(vals, valslp, &vdata), RES_BUSY);
  vdatac = vdata;
  CHECK(pseConstrainedParameterSpaceValuesUnlock(vals, &vdata), RES_OK);
  CHECK(pseConstrainedParameterSpaceValuesUnlock(vals, &vdatac), RES_INVALID);

  valslp = &PSE_CPSPACE_VALUES_LOCK_PARAMS_READ_WRITE;
  CHECK(pseConstrainedParameterSpaceValuesLock(vals, valslp, &vdata), RES_OK);
  CHECK(pseConstrainedParameterSpaceValuesLock(vals, valslp, &vdata), RES_BUSY);
  vdatac = vdata;
  CHECK(pseConstrainedParameterSpaceValuesUnlock(vals, &vdata), RES_OK);
  CHECK(pseConstrainedParameterSpaceValuesUnlock(vals, &vdatac), RES_INVALID);

  valsd.as.per_attrib.accessors[PSE_POINT_ATTRIB_COORDINATES].ctxt = (void*)ppsmpls;
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, &valsd, &smpls), RES_OK);

  /****************************************************************************
   * Test API - Constrained Parameter Space Exploration
   ****************************************************************************/
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (NULL, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (cps, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (NULL, &exparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (NULL, NULL, &exctxt), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (cps, &exparams, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (cps, NULL, &exctxt), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (NULL, &exparams, &exctxt), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (cps, &exparams, &exctxt), RES_BAD_ARG);

  exparams.pspace.explore_in = psuids[0];
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (cps, &exparams, &exctxt), RES_OK);

  CHECK(pseConstrainedParameterSpaceExplorationContextRefAdd
    (NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationContextRefAdd
    (exctxt), RES_OK);

  CHECK(pseConstrainedParameterSpaceExplorationContextRefSub
    (NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationContextRefSub
    (exctxt), RES_OK);

  CHECK(pseConstrainedParameterSpaceExplorationRelationshipsAllContextsClean
    (NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationRelationshipsAllContextsClean
    (exctxt), RES_OK);

  CHECK(pseConstrainedParameterSpaceExplorationRelationshipsAllContextsInit
    (NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationRelationshipsAllContextsInit
    (exctxt, NULL), RES_OK);

  CHECK(pseConstrainedParameterSpaceExplorationSolve
    (NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationSolve
    (exctxt, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationSolve
    (NULL, &exsmpls), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationSolve
    (exctxt, &exsmpls), RES_BAD_ARG);

  CHECK(pseConstrainedParameterSpaceExplorationIterativeSolveBegin
    (NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationIterativeSolveBegin
    (exctxt, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationIterativeSolveBegin
    (NULL, &exsmpls), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationIterativeSolveBegin
    (exctxt, &exsmpls), RES_BAD_ARG);

  exsmpls.values = smpls;
  CHECK(pseConstrainedParameterSpaceExplorationSolve
    (exctxt, &exsmpls), RES_OK);
  /* TODO: we should check the returned code RES_BUSY, RES_NOT_FOUND and
   * RES_NOT_CONVERGED */

  CHECK(pseConstrainedParameterSpaceExplorationIterativeSolveBegin
    (exctxt, &exsmpls), RES_NOT_CONVERGED);
  /* TODO: we should check the returned code RES_BUSY, RES_NOT_FOUND */

  CHECK(pseConstrainedParameterSpaceExplorationIterativeSolveStep
    (NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationIterativeSolveStep
    (exctxt), RES_OK);
  /* TODO: we should check the returned code RES_BUSY, RES_NOT_READY */

  CHECK(pseConstrainedParameterSpaceExplorationIterativeSolveEnd
    (NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationIterativeSolveEnd
    (exctxt), RES_OK);
  /* TODO: we should check the returned code RES_BUSY, RES_NOT_READY */

  CHECK(pseConstrainedParameterSpaceExplorationLastResultsRetreive
    (NULL, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationLastResultsRetreive
    (exctxt, NULL, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationLastResultsRetreive
    (NULL, vals, NULL), RES_BAD_ARG);
  CHECK(pseConstrainedParameterSpaceExplorationLastResultsRetreive
    (exctxt, vals, NULL), RES_OK);

  /****************************************************************************
   * Test API - Clean
   ****************************************************************************/

  CHECK(pseDeviceDestroy(dev), RES_OK);

  return 0;
}
