#include "pse_cpspace_exploration_p.h"
#include "pse_cpspace_values_p.h"
#include "pse_cpspace_p.h"
#include "pse_device_p.h"

#include "stb_ds.h"
#include "stretchy_buffer.h"

#include <string.h>

/******************************************************************************
 *
 * Helper functions
 *
 ******************************************************************************/

static PSE_INLINE enum pse_res_t
pseRelationshipContextsCreate
  (struct pse_allocator_t* alloc,
   struct pse_clt_type_info_t* type_info,
   const size_t count,
   pse_clt_cost_func_ctxt_t* ctxts)
{
  enum pse_res_t res = RES_OK;
  size_t ctxt_adjusted_size = 0;
  void* buff = NULL;
  size_t i;
  assert(alloc && type_info && (count > 0) && ctxts);
  assert(type_info->memsize > 0);

  /* Here, we allocate all contexts at once, and we get the different pointers
   * by iterating on this buffer. Later, to delete it, we will use the first
   * context to call the memfree function. This a little tricky but this allow
   * us to have all the contexts contiguous in memory and treated by batch
   * operations. */

  /* Allocate the context */
  /* TODO: use a buddy allocator for each type info in order to avoid memory
   * fragmentation and ensure better compactness */
  buff = PSE_ALLOC_ARRAY_ALIGNED
    (alloc, type_info->memsize, type_info->memalign, count);
  PSE_VERIFY_OR_ELSE(buff != NULL, res = RES_MEM_ERR; goto error);
  ctxt_adjusted_size = pseAllocationArrayElemSizeAdjust
    (type_info->memsize, pseAllocationAlignementAdjust(type_info->memalign));

  /* Build the ctxts list */
  for(i = 0; i < count; ++i) {
    pse_clt_cost_func_ctxt_t curr =
      (pse_clt_cost_func_ctxt_t)((uintptr_t)buff + i * ctxt_adjusted_size);
    ctxts[i] = curr;

    /* Memory initialization by copy, if needed */
    if( type_info->memdefault )
      (void)memcpy(curr, type_info->memdefault, type_info->memsize);
  }

  /* Memory initialization by function, if needed */
  if( type_info->meminit ) {
    PSE_CALL_OR_GOTO(res,error, type_info->meminit
      (type_info->user_ctxt, type_info->type_id, count, ctxts));
  }

exit:
  return res;
error:
  PSE_FREE(alloc, buff);
  goto exit;
}

static PSE_INLINE void
pseRelationshipContextsDestroy
  (struct pse_allocator_t* alloc,
   struct pse_clt_type_info_t* type_info,
   const size_t count,
   pse_clt_cost_func_ctxt_t* ctxts)
{
  assert(alloc && type_info && (count > 0) && ctxts);

  /* Memory clean by function, if needed */
  if( type_info->memclean ) {
    PSE_CALL(type_info->memclean
      (type_info->user_ctxt, type_info->type_id, count, ctxts));
  }

  /* Free the memory of the context. See pseRelationshipContextsCreate for an
   * explanation of this call. */
  PSE_FREE(alloc, ctxts[0]);
}

static PSE_INLINE void
pseConstrainedParameterSpaceInstanceClean
  (struct pse_allocator_t* alloc,
   struct pse_cpspace_instance_t* inst)
{
  size_t i,j, count;

  /* Clean relationships */
  count = hmlenu(inst->relshps);
  for(i = 0; i < count; ++i) {
    struct pse_cpspace_instance_relshp_data_t* ird = &inst->relshps[i];
    sb_free(ird->eval_data.ppoints);
  }
  hmfree(inst->relshps);

  /* Clean cost functors */
  count = hmlenu(inst->cfuncs);
  for(i = 0; i < count; ++i) {
    struct pse_cpspace_instance_cost_func_data_t* icfd = &inst->cfuncs[i];
    for(j = 0; j < icfd->variations_count; ++j) {
      struct pse_cpspace_instance_variated_cost_func_data_t* ivcfd =
        &icfd->variations[j];
      if( ivcfd->relshps_count == 0 )
        continue; /* Nothing to do */

      /* Destroy the contexts */
      pseRelationshipContextsDestroy
        (alloc, &icfd->params.ctxt_type_info,
         sb_count(ivcfd->relshps_ctxts),
         ivcfd->relshps_ctxts);

      sb_free(ivcfd->relshps_ids);
      sb_free(ivcfd->relshps_data);
      sb_free(ivcfd->relshps_ctxts);
      sb_free(ivcfd->relshps_configs);
    }
    sb_free(icfd->variations);
  }
  hmfree(inst->cfuncs);

  /* Clean parametric spaces */
  sb_free(inst->pspaces);
}

static PSE_INLINE enum pse_res_t
pseConstrainedParameterSpaceInstanciate
  (struct pse_allocator_t* alloc,
   const struct pse_cpspace_exploration_variations_params_t* evarsp,
   struct pse_cpspace_t* cps,
   struct pse_cpspace_instance_t* inst)
{
  enum pse_res_t res = RES_OK;
  size_t i,j,k,l, ipp, count;
  assert(alloc && evarsp && cps && inst);

  *inst = PSE_CPSPACE_INSTANCE_NULL;
  inst->cps = cps;

  /* Prepare the buffer for the pspaces, we will adjust it later in this
   * function */
  inst->pspaces_count = sb_count(cps->pspaces_uid);
  sb_setn(inst->pspaces, inst->pspaces_count);
  for(i = 0; i < inst->pspaces_count; ++i) {
    struct pse_cpspace_instance_pspace_data_t* psd = &inst->pspaces[i];
    psd->key = cps->pspaces_uid[i];
    for(j = 0; j < PSE_POINT_ATTRIB_COUNT_; ++j) {
      psd->ppoint_attribs_comps_count[j] =
        cps->pspaces[i].ppoint_params.attribs[j].components_count;
    }
  }

  /* We keep the parametric points */
  inst->ppoints_count = sb_count(cps->ppoints_used);
  sb_setn(inst->ppoints, inst->ppoints_count);
  for(i = 0; i < inst->ppoints_count; ++i) {
    inst->ppoints[i] = cps->ppoints_used[i];
  }

  /* Then create the cost functors data to be used later */
  count = sb_count(cps->functors_used);
  inst->cfuncs_count = count;
  for(i = 0; i < count; ++i) {
    const pse_relshp_cost_func_id_t fid = cps->functors_used[i];
    struct pse_relshp_cost_func_params_t* rcfp = &cps->functors[fid];
    struct pse_cpspace_instance_cost_func_data_t icfd =
      PSE_CPSPACE_INSTANCE_COST_FUNC_DATA_NULL;
    icfd.key = fid;
    icfd.params = *rcfp;

    /* We add a variated instance of the cost functor with an invalid variation
     * id to store the "no-variation" case. */
    sb_setn(icfd.variations, 1);
    icfd.variations[0] = PSE_CPSPACE_INSTANCE_VARIATED_COST_FUNC_DATA_NULL;

    hmputs(inst->cfuncs, icfd);
  }
  assert(hmlenu(inst->cfuncs) == inst->cfuncs_count);

  /* Then do a first pass to associate the relationships to functors and to
   * instanciate the list of parametric points involved in the relationships. */
  /* TODO: we should extract the ppoints of the instance from the ppoints used
   * in relationships, as we could have some ppoints that are not used by any
   * relationship and that will take memory space for nothing. */
  count = sb_count(cps->relshps_used);
  inst->relshps_count = 0;
  for(i = 0; i < count; ++i) {
    const pse_relshp_id_t rid = cps->relshps_used[i];
    const struct pse_cpspace_relshp_t* r = &cps->relshps[rid];
    const struct pse_cpspace_relshp_params_t* rp = &r->params;
    struct pse_cpspace_instance_relshp_data_t new_rd;

    /* Remove disabled relationships */
    if( r->state == PSE_CPSPACE_RELSHP_STATE_DISABLED )
      continue;

    ++inst->relshps_count;
    new_rd = PSE_CPSPACE_INSTANCE_RELSHP_DATA_NULL;
    new_rd.key = rid;

    /* Precompute the list of the ppoint ids involved in this relationship */
    assert(hmgeti(inst->relshps,rid) < 0);
    switch(rp->kind) {
      case PSE_RELSHP_KIND_INCLUSIVE: {
        /* Simple case where we have to only copy the ppoint ids as they are
         * the ones involved in this relationship */
        new_rd.eval_data.ppoints_count = rp->ppoints_count;
        sb_setn(new_rd.eval_data.ppoints, new_rd.eval_data.ppoints_count);
        for(j = 0; j < rp->ppoints_count; ++j) {
          new_rd.eval_data.ppoints[j] = rp->ppoints_id[j];
        }
      } break;
      case PSE_RELSHP_KIND_EXCLUSIVE: {
        /* Hard case where we have to deduce the involved ppoint by getting
         * all of them expect the ones listed in the relationship */
        /* TODO: optimize for specific case where there is no excluded ppoint */
        const size_t ppoints_count = sb_count(cps->ppoints);
        const size_t involved_ppoints_count = ppoints_count - rp->ppoints_count;
        bool found = false;
        assert(ppoints_count >= rp->ppoints_count);

        new_rd.eval_data.ppoints_count = involved_ppoints_count;
        sb_setn(new_rd.eval_data.ppoints, new_rd.eval_data.ppoints_count);
        ipp = 0;
        for(j = 0; j < ppoints_count && ipp < involved_ppoints_count; ++j) {
          /* Check if we have to skip this ppoint */
          const pse_ppoint_id_t ppid = (pse_ppoint_id_t)j;
          found = false;
          for(k = 0; k < rp->ppoints_count; ++k) {
            if( rp->ppoints_id[k] == ppid ) {
              found = true;
              break;
            }
          }
          if( !found ) {
            /* The current ppoint is not excluded from the relationship, keep
             * it! */
            new_rd.eval_data.ppoints[ipp++] = ppid;
          }
        }
        assert(j == ppoints_count);
        assert(ipp == involved_ppoints_count);
      } break;
      default: break;
    }

    /* Add this new relationship instance */
    hmputs(inst->relshps,new_rd);
  }
  assert(hmlenu(inst->relshps) == inst->relshps_count);
  assert(inst->relshps_count <= count);

  /* Associate the relationship to its cost functors, including variations. We
   * do that as a second pass in order to ensure that the relationships data
   * will not move in memory, as we keep pointers on them. */
  for(i = 0; i < count; ++i) {
    const pse_relshp_id_t rid = cps->relshps_used[i];
    const struct pse_cpspace_relshp_t* r = &cps->relshps[rid];
    const struct pse_cpspace_relshp_params_t* rp = &r->params;
    struct pse_cpspace_instance_relshp_data_t* ird = NULL;

    /* Do not treat disabled relationships */
    if( r->state == PSE_CPSPACE_RELSHP_STATE_DISABLED )
      continue;

    ird = &hmgets(inst->relshps, rid);
    for(j = 0; j < rp->cnstrs.funcs_count; ++j) {
      const pse_relshp_cost_func_id_t rcfid = rp->cnstrs.funcs[j];
      struct pse_pspace_params_t* psp = NULL;
      struct pse_cpspace_instance_cost_func_data_t* icfd =
        &hmgets(inst->cfuncs, rcfid);
      pse_clt_cost_func_ctxt_config_t rcfc = rp->cnstrs.ctxts_config
        ? rp->cnstrs.ctxts_config[j]
        : NULL;

      /* First, get the parameter space params in order to know which variations
       * are appliable. */
      for(k = 0; k < inst->pspaces_count; ++k) {
        if( inst->pspaces[k].key == icfd->params.expected_pspace ) {
          psp = &cps->pspaces[k];
          break;
        }
      }
      assert(psp != NULL);

      /* We add the relationship to all variated instance of the cost functor.*/
      for(k = 0; k < rp->variations_count+1; ++k) {
        const pse_clt_ppoint_variation_uid_t varuid = (k == 0)
          ? PSE_CLT_PPOINT_VARIATION_UID_INVALID
          : rp->variations[k-1];
        struct pse_cpspace_instance_variated_cost_func_data_t* ivcfd = NULL;
        if( varuid == PSE_CLT_PPOINT_VARIATION_UID_INVALID ) {
          /* No-variation case, get the first variated instance that must be the
           * one storing information. */
          ivcfd = &icfd->variations[0];
        } else {
          /* We have a specific variation, so we have to check if the current
           * parameter space can apply it. If not, we skip it. */
          bool appliable = false;
          for(l = 0; l < psp->variations_count; ++l) {
            if( psp->variations[l] == varuid ) {
              appliable = true;
              break;
            }
          }
          if( !appliable )
            continue; /* Skip the variation */
          /* We also check that it's a variation that we want to explore, or
           * else it is useless to precompute things about it! */
          for(l = 0; l < evarsp->count; ++l) {
            if( evarsp->to_explore[l] == varuid ) {
              appliable = true;
              break;
            }
          }
          if( !appliable )
            continue; /* Skip the variation */

          /* Now we know we can and want to apply the variation. We have to get
           * the associated variated instance of the cost functor. */
          for(l = 1; l < sb_count(icfd->variations); ++l) {
            if( icfd->variations[l].uid == varuid ) {
              /* It already exists */
              ivcfd = &icfd->variations[l];
              break;
            }
          }
          if( !ivcfd ) {
            /* It's the first time we have to add a relationship in this
             * variated instance cost functor. Create it! */
            ivcfd = sb_add(icfd->variations, 1);
            *ivcfd = PSE_CPSPACE_INSTANCE_VARIATED_COST_FUNC_DATA_NULL;
            ivcfd->uid = varuid;
          }
        }
        assert(ivcfd != NULL);

        sb_push(ivcfd->relshps_ids, ird->key);
        sb_push(ivcfd->relshps_data, &ird->eval_data);
        sb_push(ivcfd->relshps_ctxts, NULL); /* created later */
        sb_push(ivcfd->relshps_configs, rcfc);
      }
    }
  }

  /* Finally, create the relationship contexts at once per cost functor. */
  /* TODO: here, we create a context per variation. We should use only one
   * context for all possible variations! */
  count = hmlenu(inst->cfuncs);
  for(i = 0; i < count; ++i) {
    struct pse_cpspace_instance_cost_func_data_t* icfd = &inst->cfuncs[i];
    icfd->variations_count = sb_count(icfd->variations);
    assert(icfd->variations_count >= 1);

    for(j = 0; j < icfd->variations_count; ++j) {
      struct pse_cpspace_instance_variated_cost_func_data_t* ivcfd =
        &icfd->variations[j];
      ivcfd->relshps_count = sb_count(ivcfd->relshps_ids);
      if( ivcfd->relshps_count <= 0 )
        continue; /* No relationship for this cost function */
      if( icfd->params.ctxt_type_info.memsize <= 0 )
        continue; /* No context for this cost function */

      /* Create the context for future evaluation */
      assert(sb_count(ivcfd->relshps_ctxts) == ivcfd->relshps_count);
      PSE_CALL_OR_GOTO(res,error, pseRelationshipContextsCreate
        (alloc, &icfd->params.ctxt_type_info,
         sb_count(ivcfd->relshps_ctxts),
         ivcfd->relshps_ctxts));
    }
  }

exit:
  return res;
error:
  /* TODO */
  assert(false);
  goto exit;
}

static PSE_INLINE void
pseConstrainedParameterSpaceExplorationContextRefRelease
  (pse_ref_t* ref)
{
  struct pse_cpspace_exploration_ctxt_t* ctxt =
    PSE_CONTAINER_OF(ref, struct pse_cpspace_exploration_ctxt_t, ref);
  struct pse_cpspace_t* cps = ctxt->ctxt.cps;

  pseConstrainedParameterSpaceExplorationContextDestroy(ctxt);
  (void)hmdel(cps->exp_ctxts,ctxt);
  PSE_CALL(pseConstrainedParameterSpaceRefSub(cps));
}

/******************************************************************************
 *
 * PRIVATE API - Constrained Parameter Space Exploration
 *
 ******************************************************************************/

void
pseConstrainedParameterSpaceExplorationContextDestroy
  (struct pse_cpspace_exploration_ctxt_t* ctxt)
{
  struct pse_allocator_t* alloc = NULL;
  assert(ctxt);
  PSE_CALL(pseConstrainedParameterSpaceExplorationRelationshipsAllContextsClean
    (ctxt));
  if( ctxt->drv_ctxt_id != PSE_DRV_EXPLORATION_ID_INVALID ) {
    PSE_CALL(ctxt->drv.exploration_clean(ctxt->drv.self, ctxt->drv_ctxt_id));
  }
  alloc = ctxt->ctxt.dev->allocator;
  sb_free(ctxt->params.variations.to_explore);
  pseConstrainedParameterSpaceInstanceClean(alloc, &ctxt->icps);
  PSE_FREE(alloc, ctxt);
}

/******************************************************************************
 *
 * PUBLIC API - Constrained Parameter Space Exploration
 *
 ******************************************************************************/

enum pse_res_t
pseConstrainedParameterSpaceExplorationContextCreate
  (struct pse_cpspace_t* cps,
   struct pse_cpspace_exploration_ctxt_params_t* params,
   struct pse_cpspace_exploration_ctxt_t** out_ctxt)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_exploration_ctxt_t* ctxt = NULL;
  size_t i, count;
  if( !cps || !params || !out_ctxt )
    return RES_BAD_ARG;
  if(  (params->pspace.explore_in == PSE_CLT_PSPACE_UID_INVALID)
    || (!params->pspace.convert && (sb_count(cps->pspaces_uid) > 1)) )
    return RES_BAD_ARG;
  if(  params->variations.count
    && (!params->variations.to_explore || !params->variations.apply) )
    return RES_BAD_ARG;

  ctxt = PSE_TYPED_ALLOC
    (cps->dev->allocator,
     struct pse_cpspace_exploration_ctxt_t);
  PSE_VERIFY_OR_ELSE(ctxt != NULL, res = RES_MEM_ERR; goto error);
  *ctxt = PSE_CPSPACE_EXPLORATION_CTXT_NULL;
  pseRefCountInit(&ctxt->ref);
  ctxt->params = *params;
  ctxt->ctxt.dev = cps->dev;
  ctxt->ctxt.cps = cps;
  ctxt->ctxt.exp_ctxt = ctxt;
  ctxt->drv_ctxt_id = PSE_DRV_EXPLORATION_ID_INVALID;
  ctxt->drv = cps->dev->drv;

  /* Keep a copy of variations */
  count = ctxt->params.variations.count;
  if( count > 0 ) {
    ctxt->params.variations.to_explore = NULL;
    sb_setn(ctxt->params.variations.to_explore, count);
    for(i = 0; i < count; ++i) {
      ctxt->params.variations.to_explore[i] = params->variations.to_explore[i];
    }
  }

  /* Now instanciate the CPS locally. */
  /* TODO: ensure this instanciation makes us robust to CPS modifications during
   * the life of this exploration context. */
  PSE_CALL_OR_GOTO(res,error, pseConstrainedParameterSpaceInstanciate
    (cps->dev->allocator, &ctxt->params.variations, cps, &ctxt->icps));

  /* Setup the exploration context on the driver */
  PSE_CALL_OR_GOTO(res,error, ctxt->drv.cpspace_exploration_prepare
    (ctxt->drv.self, ctxt, &ctxt->icps, params, &ctxt->drv_ctxt_id));

  /* Keep the context in the cps */
  {
    struct pse_exploration_ctxt_entry_t entry;
    entry.key = ctxt;
    hmputs(cps->exp_ctxts, entry);
    /* Keep a reference on the cps as we need it */
    PSE_CALL(pseConstrainedParameterSpaceRefAdd(cps));
  }

  *out_ctxt = ctxt;

exit:
  return res;
error:
  if( NULL != ctxt ) {
    pseConstrainedParameterSpaceInstanceClean(cps->dev->allocator, &ctxt->icps);
    PSE_FREE(cps->dev->allocator, ctxt);
  }
  goto exit;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationContextRefAdd
  (struct pse_cpspace_exploration_ctxt_t* ctxt)
{
  if( !ctxt )
    return RES_BAD_ARG;
  pseRefCountInc(&ctxt->ref);
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationContextRefSub
  (struct pse_cpspace_exploration_ctxt_t* ctxt)
{
  if( !ctxt )
    return RES_BAD_ARG;
  pseRefCountDec
    (&ctxt->ref, pseConstrainedParameterSpaceExplorationContextRefRelease);
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationContextParamsGet
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_cpspace_exploration_ctxt_params_t* params)
{
  if( !ctxt || !params )
    return RES_BAD_ARG;
  *params = ctxt->params;
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationRelationshipsAllContextsInit
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   pse_clt_cost_func_ctxt_init_params_t params)
{
  enum pse_res_t res = RES_OK;
  struct pse_eval_relshps_t eval_relshps = PSE_EVAL_RELSHPS_NULL;
  size_t i,j, count;
  if( !ctxt )
    return RES_BAD_ARG;

  /* TODO: we should have only one context per relationship, for all variations.
   * Right now, we have a contexte per relationship and per variation. */
  count = hmlenu(ctxt->icps.cfuncs);
  for(i = 0; i < count; ++i) {
    struct pse_cpspace_instance_cost_func_data_t* icfd = &ctxt->icps.cfuncs[i];
    /* First, check if this function have initialization mechanism */
    if( !icfd->params.ctxts_init )
      continue;

    for(j = 0; j < icfd->variations_count; ++j) {
      struct pse_cpspace_instance_variated_cost_func_data_t* ivcfd =
        &icfd->variations[j];
      assert(sb_count(ivcfd->relshps_ids) == ivcfd->relshps_count);
      assert(sb_count(ivcfd->relshps_data) == ivcfd->relshps_count);
      assert(sb_count(ivcfd->relshps_ctxts) == ivcfd->relshps_count);
      assert(sb_count(ivcfd->relshps_configs) == ivcfd->relshps_count);

      /* Stop if we have no relationships */
      eval_relshps.count = ivcfd->relshps_count;
      if( eval_relshps.count == 0 )
        continue;

      /* Initialize all relationship contexts related to this cost function, at
       * once. */
      eval_relshps.ids = ivcfd->relshps_ids;
      eval_relshps.data = ivcfd->relshps_data;
      eval_relshps.ctxts = ivcfd->relshps_ctxts;
      eval_relshps.configs = ivcfd->relshps_configs;
      PSE_CALL_OR_RETURN(res, icfd->params.ctxts_init
        (icfd->params.user_config, icfd->params.uid,
         &ctxt->ctxt, &eval_relshps, params));
    }
  }
  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationRelationshipsAllContextsClean
  (struct pse_cpspace_exploration_ctxt_t* ctxt)
{
  enum pse_res_t res = RES_OK;
  struct pse_eval_relshps_t eval_relshps = PSE_EVAL_RELSHPS_NULL;
  size_t i,j, count;
  if( !ctxt )
    return RES_BAD_ARG;

  /* TODO: we should have only one context per relationship, for all variations.
   * Right now, we have a contexte per relationship and per variation. */
  count = hmlenu(ctxt->icps.cfuncs);
  for(i = 0; i < count; ++i) {
    struct pse_cpspace_instance_cost_func_data_t* icfd = &ctxt->icps.cfuncs[i];
    /* First, check if this function have initialization mechanism */
    if( !icfd->params.ctxts_clean )
      continue;

    for(j = 0; j < icfd->variations_count; ++j) {
      struct pse_cpspace_instance_variated_cost_func_data_t* ivcfd =
        &icfd->variations[j];
      assert(sb_count(ivcfd->relshps_ids) == ivcfd->relshps_count);
      assert(sb_count(ivcfd->relshps_data) == ivcfd->relshps_count);
      assert(sb_count(ivcfd->relshps_ctxts) == ivcfd->relshps_count);
      assert(sb_count(ivcfd->relshps_configs) == ivcfd->relshps_count);

      /* Stop if we have no relationships */
      eval_relshps.count = ivcfd->relshps_count;
      if( eval_relshps.count == 0 )
        continue;

      /* Initialize all relationship contexts related to this cost function, at
       * once. */
      eval_relshps.ids = ivcfd->relshps_ids;
      eval_relshps.data = ivcfd->relshps_data;
      eval_relshps.ctxts = ivcfd->relshps_ctxts;
      eval_relshps.configs = ivcfd->relshps_configs;
      PSE_CALL_OR_RETURN(res, icfd->params.ctxts_clean
        (icfd->params.user_config, icfd->params.uid,
         &ctxt->ctxt, &eval_relshps));
    }
  }
  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationSolve
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_cpspace_exploration_samples_t* smpls)
{
  enum pse_res_t res = RES_OK;
  if( !ctxt || !smpls || !smpls->values )
    return RES_BAD_ARG;
  if( ctxt->drv_ctxt_id == PSE_DRV_EXPLORATION_ID_INVALID )
    return RES_NOT_AUTHORIZED;

  PSE_TRY_CALL_OR_RETURN(res, ctxt->drv.exploration_solve
    (ctxt->drv.self, ctxt->drv_ctxt_id, &smpls->values->data));
  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationIterativeSolveBegin
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_cpspace_exploration_samples_t* smpls)
{
  enum pse_res_t res = RES_OK;
  if( !ctxt || !smpls || !smpls->values )
    return RES_BAD_ARG;
  if( ctxt->drv_ctxt_id == PSE_DRV_EXPLORATION_ID_INVALID )
    return RES_NOT_AUTHORIZED;

  PSE_TRY_CALL_OR_RETURN(res, ctxt->drv.exploration_solve_iterative_begin
    (ctxt->drv.self, ctxt->drv_ctxt_id, &smpls->values->data));
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationIterativeSolveStep
  (struct pse_cpspace_exploration_ctxt_t* ctxt)
{
  enum pse_res_t res = RES_OK;
  if( !ctxt )
    return RES_BAD_ARG;
  if( ctxt->drv_ctxt_id == PSE_DRV_EXPLORATION_ID_INVALID )
    return RES_NOT_AUTHORIZED;

  PSE_TRY_CALL_OR_RETURN(res, ctxt->drv.exploration_solve_iterative_step
    (ctxt->drv.self, ctxt->drv_ctxt_id));
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationIterativeSolveEnd
  (struct pse_cpspace_exploration_ctxt_t* ctxt)
{
  enum pse_res_t res = RES_OK;
  if( !ctxt )
    return RES_BAD_ARG;
  if( ctxt->drv_ctxt_id == PSE_DRV_EXPLORATION_ID_INVALID )
    return RES_NOT_AUTHORIZED;

  PSE_CALL_OR_RETURN(res, ctxt->drv.exploration_solve_iterative_end
    (ctxt->drv.self, ctxt->drv_ctxt_id));
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceExplorationLastResultsRetreive
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_cpspace_values_t* where,
   struct pse_cpspace_exploration_extra_results_t* extra)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_values_data_t* data = NULL;
  if( !ctxt || !where )
    return RES_BAD_ARG;
  if( ctxt->drv_ctxt_id == PSE_DRV_EXPLORATION_ID_INVALID )
    return RES_NOT_AUTHORIZED;

  /* We do not use data but we want to lock */
  PSE_CALL_OR_RETURN(res, pseConstrainedParameterSpaceValuesLock
    (where, &PSE_CPSPACE_VALUES_LOCK_PARAMS_WRITE, &data));
  PSE_CALL_OR_RETURN(res, ctxt->drv.exploration_last_results_retreive
    (ctxt->drv.self, ctxt->drv_ctxt_id, &where->data, extra));
  PSE_CALL_OR_RETURN(res, pseConstrainedParameterSpaceValuesUnlock
    (where, &data));
  return RES_OK;
}
