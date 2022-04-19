#include <pse.h>
#include <pse_allocator.h>
#include <pse_logger.h>

#include <stdio.h>

/*! [ps_constants] */
#define R2_UID          1 /* any value will do the job, it's just an UID */
/*! [ps_constants] */
/*! [cfunc_constants] */
#define MIN_DISTANCE    10.0
/*! [cfunc_constants] */

/*! [vals_attrib_get] */
static enum pse_res_t
attribGet
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   void* attrib_values)
{
  size_t i;
  (void)as_type;

  switch(attrib) {
    case PSE_POINT_ATTRIB_COORDINATES: {
      const pse_real_t* src = (const pse_real_t*)ctxt;
      pse_real_t* dst = (pse_real_t*)attrib_values;
      assert(as_type == PSE_TYPE_REAL);
      for(i = 0; i < count; ++i) {
        dst[i*2+0] = src[values_idx[i]*2+0];
        dst[i*2+1] = src[values_idx[i]*2+1];
      }
    } break;
    case PSE_POINT_ATTRIB_LOCK_STATUS: {
      bool* dst = (bool*)attrib_values;
      assert(as_type == PSE_TYPE_BOOL_8);
      for(i = 0; i < count; ++i) {
        dst[i] = (i == 0);
      }
    } break;
    default: assert(false); /* No other attributes */
  }
  return RES_OK;
}
/*! [vals_attrib_get] */
/*! [vals_attrib_set] */
static enum pse_res_t
attribSet
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   const void* attrib_values)
{
  size_t i;
  (void)as_type;

  switch(attrib) {
    case PSE_POINT_ATTRIB_COORDINATES: {
      const pse_real_t* src = (const pse_real_t*)attrib_values;
      pse_real_t* dst = (pse_real_t*)ctxt;
      assert(as_type == PSE_TYPE_REAL);
      for(i = 0; i < count; ++i) {
        dst[values_idx[i]*2+0] = src[i*2+0];
        dst[values_idx[i]*2+1] = src[i*2+1];
      }
    } break;
    default: assert(false); /* No other attribute can be set */
  }
  return RES_OK;
}
/*! [vals_attrib_set] */

/*! [cfunc_compute] */
static enum pse_res_t
distCostCompute
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  size_t i;
  assert(eval_coords->pspace_uid == R2_UID);
  (void)eval_ctxt, (void)eval_coords;

  /* By construction, we know that we have 1 relationship per pair of points.
   * We iterate on all relationships and we compute 1 cost per relationship. */
  for(i = 0; i < eval_relshps->count; ++i) {
    const struct pse_eval_relshp_data_t* rd = eval_relshps->data[i];
    const pse_ppoint_id_t pid1 = rd->ppoints[0];
    const pse_ppoint_id_t pid2 = rd->ppoints[1];
    const pse_real_t p1x = eval_coords->coords[pid1*2+0];
    const pse_real_t p1y = eval_coords->coords[pid1*2+1];
    const pse_real_t p2x = eval_coords->coords[pid2*2+0];
    const pse_real_t p2y = eval_coords->coords[pid2*2+1];
    const pse_real_t dx = p2x - p1x;
    const pse_real_t dy = p2y - p1y;
    /* We know that we have built relationships on pairs of parametric points.*/
    assert(rd->ppoints_count == 2);
    /* This computation will make cost equal to 0 if the distance between the
     * two points is greater or equal to MIN_DISTANCE. */
    costs[i] = PSE_MAX(0.0, MIN_DISTANCE - PSE_REAL_SQRT(dx*dx + dy*dy));
  }
  return RES_OK;
}
/*! [cfunc_compute] */

int main(int argc, char* argv[])
{
  /*! [device_decl] */
  struct pse_device_params_t devp = PSE_DEVICE_PARAMS_NULL;
  struct pse_device_t* dev = NULL;
  /*! [device_decl] */
  /*! [cps_decl] */
  struct pse_cpspace_params_t cpsp = PSE_CPSPACE_PARAMS_NULL;
  struct pse_cpspace_t* cps = NULL;
  /*! [cps_decl] */
  /*! [ps_decl] */
  struct pse_pspace_params_t psp = PSE_PSPACE_PARAMS_NULL;
  pse_clt_pspace_uid_t R2 = R2_UID;
  struct pse_pspace_point_attrib_component_t ppcomps[] = {
    { PSE_TYPE_REAL }, { PSE_TYPE_REAL } /* coordinates in R2 as 2 real values */
  };
  struct pse_pspace_point_attrib_component_t lscomps[] = {
    { PSE_TYPE_BOOL_8 } /* Lock status is made of only one boolean */
  };
  /*! [ps_decl] */
  /*! [pp_decl] */
  struct pse_ppoint_params_t ppp[] = {
    PSE_PPOINT_PARAMS_NULL_,
    PSE_PPOINT_PARAMS_NULL_,
    PSE_PPOINT_PARAMS_NULL_
  };
  pse_ppoint_id_t ppids[] = {
    PSE_PPOINT_ID_INVALID_,
    PSE_PPOINT_ID_INVALID_,
    PSE_PPOINT_ID_INVALID_
  };
  /*! [pp_decl] */
  /*! [cfunc_decl] */
  struct pse_relshp_cost_func_params_t cfp = PSE_RELSHP_COST_FUNC_PARAMS_NULL;
  pse_relshp_cost_func_id_t cfid = PSE_RELSHP_COST_FUNC_ID_INVALID;
  /*! [cfunc_decl] */
  /*! [relshps_decl] */
  pse_ppoint_id_t pppairs[][2] = {
    { PSE_PPOINT_ID_INVALID_, PSE_PPOINT_ID_INVALID_ },
    { PSE_PPOINT_ID_INVALID_, PSE_PPOINT_ID_INVALID_ },
    { PSE_PPOINT_ID_INVALID_, PSE_PPOINT_ID_INVALID_ }
  };
  struct pse_cpspace_relshp_params_t rp[] = {
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_
  };
  pse_relshp_id_t rids[] = {
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_
  };
  /*! [relshps_decl] */
  /*! [vals_decl] */
  pse_real_t raw_values[][2] = {
    { 0.0, 0.0 },
    { 0.0, 0.0 },
    { 0.0, 0.0 }
  };
  struct pse_cpspace_values_t* smpls = NULL;
  struct pse_cpspace_values_t* res = NULL;
  struct pse_cpspace_values_data_t smplsd = PSE_CPSPACE_VALUES_DATA_NULL;
  struct pse_cpspace_values_data_t resd = PSE_CPSPACE_VALUES_DATA_NULL;
  /*! [vals_decl] */
  /*! [explc_decl] */
  struct pse_cpspace_exploration_ctxt_params_t ectxtp =
    PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL;
  struct pse_cpspace_exploration_ctxt_t* ectxt = NULL;
  /*! [explc_decl] */
  /*! [expl_decl] */
  struct pse_cpspace_exploration_samples_t esmpls =
    PSE_CPSPACE_EXPLORATION_SAMPLES_NULL;
  /*! [expl_decl] */
  size_t i, j;

  (void)argc, (void)argv;

  /*! [device_setup] */
  devp.allocator = &PSE_ALLOCATOR_DEFAULT;
  devp.logger = &PSE_LOGGER_STDOUT; /* default: no logger */
  devp.backend_drv_filepath = PSE_LIB_NAME("pse-drv-eigen-ref");
  /*! [device_setup] */
  /*! [device_create] */
  PSE_CALL(pseDeviceCreate(&devp, &dev));
  /*! [device_create] */

  /*! [cps_setup] */
  /* Nothing to setup for the constrained parameter space for now, there is no
   * specific parameters. */
  /*! [cps_setup] */

  /*! [cps_create] */
  PSE_CALL(pseConstrainedParameterSpaceCreate(dev, &cpsp, &cps));
  /*! [cps_create] */

  /*! [ps_setup] */
  psp.ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components_count = 2;
  psp.ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components = ppcomps;
  psp.ppoint_params.attribs[PSE_POINT_ATTRIB_LOCK_STATUS].components_count = 1;
  psp.ppoint_params.attribs[PSE_POINT_ATTRIB_LOCK_STATUS].components = lscomps;
  /*! [ps_setup] */

  /*! [ps_create] */
  PSE_CALL(pseConstrainedParameterSpaceParameterSpacesDeclare(cps, 1, &R2, &psp));
  /*! [ps_create] */

  /*! [pp_setup] */
  /* Nothing to setup for the parametric points for now, there is no specific
   * parameters. */
  /*! [pp_setup] */
  /*! [pp_create] */
  PSE_CALL(pseConstrainedParameterSpaceParametricPointsAdd(cps, 3, ppp, ppids));
  /*! [pp_create] */

  /*! [cfunc_setup] */
  cfp.expected_pspace = R2;
  cfp.compute = distCostCompute;
  /* We tell the optimizer that this cost functor will generate 1 cost per
   * relationship. */
  cfp.costs_count = 1;
  cfp.cost_arity_mode = PSE_COST_ARITY_MODE_PER_RELATIONSHIP;
  /*! [cfunc_setup] */
  /*! [cfunc_create] */
  PSE_CALL(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, &cfp, &cfid));
  /*! [cfunc_create] */

  /*! [relshps_setup] */
  /* We build the pairs of parametric points */
  pppairs[0][0] = ppids[0];
  pppairs[0][1] = ppids[1];
  pppairs[1][0] = ppids[0];
  pppairs[1][1] = ppids[2];
  pppairs[2][0] = ppids[1];
  pppairs[2][1] = ppids[2];

  /* We build the relationships parameters using the parametric point pairs */
  for(i = 0; i < 3; ++i) {
    rp[i].kind = PSE_RELSHP_KIND_INCLUSIVE;
    rp[i].ppoints_count = 2;
    rp[i].ppoints_id = pppairs[i];
    rp[i].cnstrs.funcs_count = 1;
    rp[i].cnstrs.funcs = &cfid; /* distance cost functor for everyone */
  }
  /*! [relshps_setup] */
  /*! [relshps_create] */
  PSE_CALL(pseConstrainedParameterSpaceRelationshipsAdd(cps, 0, 3, rp, rids));
  /*! [relshps_create] */

  /*! [vals_setup] */
  /* Read only values for samples */
  smplsd.pspace = R2;
  smplsd.storage = PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_GLOBAL;
  smplsd.as.global.accessors.get = attribGet;
  smplsd.as.global.accessors.set = NULL;
  smplsd.as.global.accessors.ctxt = raw_values;

  /* Write only values for results */
  resd.pspace = R2;
  resd.storage = PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_GLOBAL;
  resd.as.global.accessors.get = NULL;
  resd.as.global.accessors.set = attribSet;
  resd.as.global.accessors.ctxt = raw_values;
  /*! [vals_setup] */
  /*! [vals_create] */
  PSE_CALL(pseConstrainedParameterSpaceValuesCreate(cps, &smplsd, &smpls));
  PSE_CALL(pseConstrainedParameterSpaceValuesCreate(cps, &resd, &res));
  /*! [vals_create] */

  /*! [explc_setup] */
  ectxtp.pspace.explore_in = R2;
  /*! [explc_setup] */
  /*! [explc_create] */
  PSE_CALL(pseConstrainedParameterSpaceExplorationContextCreate
    (cps, &ectxtp, &ectxt));
  /*! [explc_create] */

  /*! [expl_setup] */
  raw_values[0][0] = 0.0;
  raw_values[0][1] = 0.0;
  raw_values[1][0] = 1.0;
  raw_values[1][1] = 1.0;
  raw_values[2][0] = 2.0;
  raw_values[2][1] = 2.0;
  esmpls.values = smpls;
  /*! [expl_setup] */
  /*! [expl_print_initial] */
  for(i = 0; i < 3; ++i) {
    fprintf
      (stdout, "Point %d to optimize: {%f, %f}\n",
       (int)i, raw_values[i][0], raw_values[i][1]);
  }
  for(i = 0; i < 2; ++i) {
    for(j = i+1; j < 3; ++j) {
      const pse_real_t dx = raw_values[j][0] - raw_values[i][0];
      const pse_real_t dy = raw_values[j][1] - raw_values[i][1];
      const pse_real_t dist = PSE_REAL_SQRT(dx*dx + dy*dy);
      fprintf(stdout, "Distance between %d and %d: %f\n", (int)i, (int)j, dist);
    }
  }
  /*! [expl_print_initial] */
  /*! [expl_solve] */
  /* We expect convergence, so we expect RES_OK as the function result */
  PSE_CALL(pseConstrainedParameterSpaceExplorationSolve(ectxt, &esmpls));
  /*! [expl_solve] */

  /*! [expl_get_results] */
  PSE_CALL(pseConstrainedParameterSpaceExplorationLastResultsRetreive
    (ectxt, res, NULL));
  /* After this call, the 'attribSet' callback has been called, so the
   * res_values is filled! */
  /*! [expl_get_results] */
  /*! [expl_print_optimized] */
  for(i = 0; i < 3; ++i) {
    fprintf
      (stdout, "Result point %d: {%f, %f}\n",
       (int)i, raw_values[i][0], raw_values[i][1]);
  }
  for(i = 0; i < 2; ++i) {
    for(j = i+1; j < 3; ++j) {
      const pse_real_t dx = raw_values[j][0] - raw_values[i][0];
      const pse_real_t dy = raw_values[j][1] - raw_values[i][1];
      const pse_real_t dist = PSE_REAL_SQRT(dx*dx + dy*dy);
      fprintf(stdout, "Distance between %d and %d: %f\n", (int)i, (int)j, dist);
    }
  }
  /*! [expl_print_optimized] */

  /*! [explc_destroy] */
  PSE_CALL(pseConstrainedParameterSpaceExplorationContextRefSub(ectxt));
  /*! [explc_destroy] */
  /*! [vals_destroy] */
  PSE_CALL(pseConstrainedParameterSpaceValuesRefSub(res));
  PSE_CALL(pseConstrainedParameterSpaceValuesRefSub(smpls));
  /*! [vals_destroy] */
  /*! [relshps_destroy] */
  PSE_CALL(pseConstrainedParameterSpaceRelationshipsClear(cps));
  /*! [relshps_destroy] */
  /*! [cfunc_destroy] */
  PSE_CALL(pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
    (cps, 1, &cfid));
  /*! [cfunc_destroy] */
  /*! [pp_destroy] */
  PSE_CALL(pseConstrainedParameterSpaceParametricPointsClear(cps));
  /*! [pp_destroy] */

  /*! [ps_destroy] */
  PSE_CALL(pseConstrainedParameterSpaceParameterSpacesForget(cps, 1, &R2));
  /*! [ps_destroy] */

  /*! [cps_destroy] */
  PSE_CALL(pseConstrainedParameterSpaceRefSub(cps));
  /*! [cps_destroy] */
  /*! [device_destroy] */
  PSE_CALL(pseDeviceDestroy(dev));
  /*! [device_destroy] */

  return 0;
}
