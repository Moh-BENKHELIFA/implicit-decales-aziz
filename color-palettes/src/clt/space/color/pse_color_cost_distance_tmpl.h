#ifndef PSE_COLOR_SPACE_NAME
# error PSE_COLOR_SPACE_NAME must be defined with the name of the color space
#endif

#ifndef PSE_COLOR_SPACE_COMPS_COUNT
# error PSE_COLOR_SPACE_COMPS_COUNT must be defined to the number of components
#endif

#ifndef PSE_L1DISTANCE_KIND_FROM_CFUNC_UID
# error PSE_L1DISTANCE_KIND_FROM_CFUNC_UID must be defined to get the L1 distance function kind from the cost functor UID
#endif

#ifdef PSE_COLOR_SPACE_FILTER_COMPONENTS
# ifndef PSE_COLOR_SPACE_FILTER_SIGNED
#   error PSE_COLOR_SPACE_FILTER_SIGNED must be defined to filter signed distances
# endif
# ifndef PSE_COLOR_SPACE_FILTER_UNSIGNED
#   error PSE_COLOR_SPACE_FILTER_UNSIGNED must be defined to filter signed distances
# endif

#define PSE_COLOR_COMP_USE_FILTERING
#else
# define PSE_COLOR_SPACE_FILTER_SIGNED    NULL
# define PSE_COLOR_SPACE_FILTER_UNSIGNED  NULL
#endif

#define PSE_CLT_COST_CTXT_T                                                    \
  PSE_CONCAT(PSE_CONCAT                                                        \
    (pse_clt_cost,PSE_COLOR_SPACE_COMPS_COUNT),_ctxt_t)
#define PSE_FUNC_COST_CONTEXTS_INIT_FROM_L1DISTANCE                            \
  PSE_CONCAT(PSE_CONCAT                                                        \
    (pseColorCost,PSE_COLOR_SPACE_COMPS_COUNT),FuncContextsInitFromL1Distance)

#define PSE_CTXT_FUNC_NAME(FuncName)                                           \
  PSE_CONCAT(PSE_CONCAT(pseColor,PSE_COLOR_SPACE_NAME),FuncName)

#define PSE_FUNC_NAME(FuncName)                                                \
  PSE_CONCAT(PSE_CONCAT(PSE_CONCAT                                             \
    (pseColorCostFuncCompute_,PSE_COLOR_SPACE_NAME),_),FuncName)

/*! TODO: make this more generic by putting most of the work in pse-clt library
 * when it's possible. We will have to manage filters and distance params.
 */

enum pse_res_t
PSE_CTXT_FUNC_NAME(CostFuncContextsInit)
  (pse_clt_cost_func_config_t user_cfunc_config,
   const pse_clt_cost_func_uid_t user_cfunc_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps,
   pse_clt_cost_func_ctxt_init_params_t user_init_params)
{
  enum pse_clt_L1_distance_kind_t kind = PSE_CLT_L1_DISTANCE_KIND_SIGNED;
  struct pse_colors_t* colors = (struct pse_colors_t*)user_init_params;
#ifdef PSE_COLOR_COMP_USE_FILTERING
  const bool must_filter[] = PSE_COLOR_SPACE_FILTER_COMPONENTS;
#endif

  (void)user_cfunc_config;
  (void)eval_ctxt;

  kind = PSE_L1DISTANCE_KIND_FROM_CFUNC_UID(user_cfunc_uid);
  return pseColorCost3FuncContextsInitFromL1Distance
    (colors, kind, eval_relshps,
  #ifdef PSE_COLOR_COMP_USE_FILTERING
     must_filter,
     PSE_COLOR_SPACE_FILTER_SIGNED,
     PSE_COLOR_SPACE_FILTER_UNSIGNED
  #else
     NULL, NULL, NULL /* no filtering */
  #endif
    );
}

enum pse_res_t
PSE_CTXT_FUNC_NAME(CostFuncContextsClean)
  (pse_clt_cost_func_config_t user_cfunc_config,
   const pse_clt_cost_func_uid_t user_cfunc_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps)
{
  (void)user_cfunc_config, (void)user_cfunc_uid;
  (void)eval_ctxt, (void)eval_relshps;
  /* Nothing to do right now */
  return RES_OK;
}

enum pse_res_t
PSE_FUNC_NAME(L1DistanceSignedPerComponent)
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  const size_t value_comps_count = PSE_COLOR_SPACE_COMPS_COUNT;
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data = NULL;
  struct pse_color_constraint_distance_config_t* cfg = NULL;
  struct PSE_CLT_COST_CTXT_T* ctxt = NULL;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i, j, c = 0;
#ifdef PSE_COLOR_COMP_USE_FILTERING
  const bool must_filter[] = PSE_COLOR_SPACE_FILTER_COMPONENTS;
#endif
  (void)eval_ctxt;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    cfg = (struct pse_color_constraint_distance_config_t*)eval_relshps->configs[i];
    ctxt = (struct PSE_CLT_COST_CTXT_T*)eval_relshps->ctxts[i];
    assert(ctxt->ref.count <= value_comps_count);
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    for(j = 0; j < ctxt->ref.count; ++j) {
      const size_t comp_idx = ctxt->comps_idx[j];
      curr_L1 = pseCltL1SignedDistanceComputeAt
        (&values[ppid1*value_comps_count],
         &values[ppid2*value_comps_count],
         comp_idx,
#ifdef PSE_COLOR_COMP_USE_FILTERING
         must_filter[comp_idx] ? PSE_COLOR_SPACE_FILTER_SIGNED : NULL
#else
         NULL
#endif
        );

      /* TODO: could we also put it as a generic feature enabled/disabled? That
       * means we will have 2 version of the functions: one with threshold
       * managed, and another one without. But we need to ensure that all the
       * relationships will have their threshold disabled. */
      if( cfg->dist_params.threshold > PSE_REAL_EPS ) {
        curr_L1 = (curr_L1 < 0)
          ? PSE_MIN(cfg->dist_params.threshold + curr_L1, 0)
          : PSE_MAX(cfg->dist_params.threshold - curr_L1, 0);
      }
      costs[c++] = cfg->as_weighted.weight * (curr_L1 - ctxt->comps[j].ref);
    }
  }
  return RES_OK;
}

enum pse_res_t
PSE_FUNC_NAME(L1DistanceUnsignedPerComponent)
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  const size_t value_comps_count = PSE_COLOR_SPACE_COMPS_COUNT;
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data = NULL;
  struct pse_color_constraint_distance_config_t* cfg = NULL;
  struct PSE_CLT_COST_CTXT_T* ctxt = NULL;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i, j, c = 0;
#ifdef PSE_COLOR_COMP_USE_FILTERING
  const bool must_filter[] = PSE_COLOR_SPACE_FILTER_COMPONENTS;
#endif
  (void)eval_ctxt;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    cfg = (struct pse_color_constraint_distance_config_t*)eval_relshps->configs[i];
    ctxt = (struct PSE_CLT_COST_CTXT_T*)eval_relshps->ctxts[i];
    assert(ctxt->ref.count <= value_comps_count);
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    for(j = 0; j < ctxt->ref.count; ++j) {
      const size_t comp_idx = ctxt->comps_idx[j];
      curr_L1 = pseCltL1UnsignedDistanceComputeAt
        (&values[ppid1*value_comps_count],
         &values[ppid2*value_comps_count],
         comp_idx,
      #ifdef PSE_COLOR_COMP_USE_FILTERING
         must_filter[comp_idx] ? PSE_COLOR_SPACE_FILTER_UNSIGNED : NULL
      #else
         NULL
      #endif
        );
      if( cfg->dist_params.threshold > PSE_REAL_EPS ) {
        curr_L1 = PSE_MAX(cfg->dist_params.threshold - curr_L1, 0);
      }
      costs[c++] = cfg->as_weighted.weight * (curr_L1 - ctxt->comps[j].ref);
    }
  }
  return RES_OK;
}

enum pse_res_t
PSE_FUNC_NAME(L1DistanceSigned)
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  const size_t value_comps_count = PSE_COLOR_SPACE_COMPS_COUNT;
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data;
  struct pse_color_constraint_distance_config_t* cfg = NULL;
  struct PSE_CLT_COST_CTXT_T* ctxt;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i, c = 0;
#ifdef PSE_COLOR_COMP_USE_FILTERING
  const bool must_filter[] = PSE_COLOR_SPACE_FILTER_COMPONENTS;
#endif
  (void)eval_ctxt;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    cfg = (struct pse_color_constraint_distance_config_t*)eval_relshps->configs[i];
    ctxt = (struct PSE_CLT_COST_CTXT_T*)eval_relshps->ctxts[i];
    assert(ctxt->ref.count <= value_comps_count);
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    curr_L1 = pseCltL1SignedDistanceSumCompute
      (&values[ppid1*value_comps_count],
       &values[ppid2*value_comps_count],
       ctxt->ref.count, ctxt->comps_idx,
    #ifdef PSE_COLOR_COMP_USE_FILTERING
       must_filter, PSE_COLOR_SPACE_FILTER_SIGNED
    #else
       NULL, NULL
    #endif
       );
    if( cfg->dist_params.threshold > PSE_REAL_EPS ) {
      curr_L1 = (curr_L1 < 0)
        ? PSE_MIN(cfg->dist_params.threshold + curr_L1, 0)
        : PSE_MAX(cfg->dist_params.threshold - curr_L1, 0);
    }
    costs[c++] = cfg->as_weighted.weight * (curr_L1 - ctxt->comps[0].ref);
  }
  return RES_OK;
}

enum pse_res_t
PSE_FUNC_NAME(L1DistanceUnsigned)
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  const size_t value_comps_count = PSE_COLOR_SPACE_COMPS_COUNT;
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data;
  struct pse_color_constraint_distance_config_t* cfg = NULL;
  struct PSE_CLT_COST_CTXT_T* ctxt;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i, c = 0;
#ifdef PSE_COLOR_COMP_USE_FILTERING
  const bool must_filter[] = PSE_COLOR_SPACE_FILTER_COMPONENTS;
#endif
  (void)eval_ctxt;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    cfg = (struct pse_color_constraint_distance_config_t*)eval_relshps->configs[i];
    ctxt = (struct PSE_CLT_COST_CTXT_T*)eval_relshps->ctxts[i];
    assert(ctxt->ref.count <= value_comps_count);
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    curr_L1 = pseCltL1UnsignedDistanceSumCompute
      (&values[ppid1*value_comps_count],
       &values[ppid2*value_comps_count],
       ctxt->ref.count, ctxt->comps_idx,
    #ifdef PSE_COLOR_COMP_USE_FILTERING
       must_filter, PSE_COLOR_SPACE_FILTER_UNSIGNED
    #else
       NULL, NULL
    #endif
       );
    if( cfg->dist_params.threshold > PSE_REAL_EPS ) {
      curr_L1 = PSE_MAX(cfg->dist_params.threshold - curr_L1, 0);
    }
    costs[c++] = cfg->as_weighted.weight * (curr_L1 - ctxt->comps[0].ref);
  }
  return RES_OK;
}

#undef PSE_COLOR_COMP_USE_FILTERING
#undef PSE_FUNC_NAME
#undef PSE_CTXT_FUNC_NAME
#undef PSE_FUNC_COST_CONTEXTS_INIT_FROM_L1DISTANCE
#undef PSE_CLT_COST_CTXT_T
#undef PSE_COLOR_SPACE_FILTER_SIGNED
#undef PSE_COLOR_SPACE_FILTER_UNSIGNED
#undef PSE_COLOR_SPACE_FILTER_COMPONENTS
#undef PSE_L1DISTANCE_KIND_FROM_CFUNC_UID
#undef PSE_COLOR_SPACE_COMPS_COUNT
#undef PSE_COLOR_SPACE_NAME
