#include "pse_cpspace_p.h"
#include "pse_device_p.h"
#include "pse_cpspace_exploration_p.h"
#include "pse_cpspace_values_p.h"

#include "stb_ds.h"
#include "stretchy_buffer.h"

/******************************************************************************
 *
 * Helper functions
 *
 ******************************************************************************/

static PSE_FINLINE enum pse_res_t
psePSpacePointAttribValidate
  (const struct pse_pspace_point_attrib_t* attrib)
{
  /* Either count>0 && ptr!=NULL, or count==0 && ptr==NULL */
  return (attrib->components_count != 0)
      == (attrib->components != NULL)
    ? RES_OK
    : RES_BAD_ARG;
}

static PSE_FINLINE enum pse_res_t
psePSpacePointParamsValidate
  (const struct pse_pspace_point_params_t* params)
{
  enum pse_res_t res = RES_OK;
  size_t i;

  /* Ensure we have at list the coordinates */
  if( params->attribs[PSE_POINT_ATTRIB_COORDINATES].components_count == 0 )
    return RES_BAD_ARG;

  /* Then check general validity of each attributes */
  for(i = 0; i < PSE_POINT_ATTRIB_COUNT_; ++i) {
    PSE_TRY_CALL_OR_RETURN(res, psePSpacePointAttribValidate
      (&params->attribs[i]));
  }

  return res;
}

static PSE_FINLINE enum pse_res_t
psePSpaceParamsValidate
  (const struct pse_pspace_params_t* params)
{
  assert(params);
  if( (params->variations_count != 0) && (params->variations != NULL) )
    return RES_BAD_ARG;
  return psePSpacePointParamsValidate(&params->ppoint_params);
}

static PSE_FINLINE enum pse_res_t
psePSpaceParamsCopy
  (const struct pse_pspace_params_t* src,
   struct pse_pspace_params_t* dst)
{
  size_t i, j;
  assert(src && dst);

  /* First, copy everything */
  *dst = *src;

  /* Then, duplicate the components of attributes */
  for(i = 0; i < PSE_POINT_ATTRIB_COUNT_; ++i) {
    struct pse_pspace_point_attrib_t* attr = &dst->ppoint_params.attribs[i];
    attr->components = NULL;
    if( attr->components_count > 0 ) {
      /* Use an intermediate buffer to avoid the const of the attribute
       * structure. */
      struct pse_pspace_point_attrib_component_t* comps = NULL;
      sb_setn(comps, attr->components_count);
      for(j = 0; j < attr->components_count; ++j) {
        comps[j] = src->ppoint_params.attribs[i].components[j];
      }
      attr->components = comps;
    }
  }

  /* Then, duplicate the variations */
  dst->variations = NULL;
  if( dst->variations_count > 0 ) {
    sb_setn(dst->variations, dst->variations_count);
    for(i = 0; i < dst->variations_count; ++i) {
      dst->variations[i] = src->variations[i];
    }
  }

  return RES_OK;
}

static PSE_FINLINE void
psePSpaceParamsClean
  (struct pse_pspace_params_t* ps)
{
  size_t i;
  assert(ps);
  for(i = 0; i < PSE_POINT_ATTRIB_COUNT_; ++i) {
    sb_free(ps->ppoint_params.attribs[i].components);
  }
  sb_free(ps->variations);
}

static PSE_FINLINE enum pse_res_t
pseFunctorsParamsValidate
  (struct pse_cpspace_t* cps,
   const size_t count,
   const struct pse_relshp_cost_func_params_t* params)
{
  size_t i;
  assert(cps && count && params);
  (void)cps;
  for(i = 0; i < count; ++i) {
    const struct pse_relshp_cost_func_params_t* p = &params[i];
    if(  (PSE_CLT_PSPACE_UID_INVALID == p->expected_pspace)
      || (NULL == p->compute)
      || (0 >= p->costs_count) )
      return RES_BAD_ARG;
  }
  return RES_OK;
}

static PSE_FINLINE enum pse_res_t
pseFunctorsIdsValidate
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_cost_func_id_t* ids)
{
  size_t i;
  enum pse_res_t res = RES_OK;
  assert(cps && count && ids);
  for(i = 0; i < count; ++i) {
    const pse_relshp_cost_func_id_t id = ids[i];
    if(  (id == PSE_RELSHP_COST_FUNC_ID_INVALID)
      || (id >= sb_count(cps->functors)) )
      return RES_BAD_ARG;
    PSE_TRY_CALL_OR_RETURN(res, pseFunctorsParamsValidate
      (cps, 1, &cps->functors[id]));
  }
  return RES_OK;
}

static PSE_FINLINE enum pse_res_t
pseParametricPointsParamsValidate
  (struct pse_cpspace_t* cps,
   const size_t count,
   const struct pse_ppoint_params_t* params)
{
  /*size_t i;*/
  assert(cps && count && params);
  (void)cps, (void)count, (void)params;
  /*
  for(i = 0; i < count; ++i) {
    const struct pse_ppoint_params_t* p = &params[i];
    if( false )
      return RES_BAD_ARG;
  }
  */
  return RES_OK;
}

static PSE_FINLINE enum pse_res_t
pseParametricPointsIdsValidate
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_ppoint_id_t* ids)
{
  size_t i;
  enum pse_res_t res = RES_OK;
  assert(cps && count && ids);
  for(i = 0; i < count; ++i) {
    const pse_ppoint_id_t id = ids[i];
    if( !pseConstrainedParameterSpaceParametricPointExists(cps, id) )
      return RES_BAD_ARG;
    PSE_TRY_CALL_OR_RETURN(res, pseParametricPointsParamsValidate
      (cps, 1, &cps->ppoints[id]));
  }
  return RES_OK;
}

static PSE_FINLINE void
pseRelationshipClean
  (struct pse_allocator_t* alloc,
   const struct pse_cpspace_relshp_t* relshp)
{
  assert(alloc && relshp);
  PSE_FREE(alloc, relshp->params.cnstrs.funcs);
  PSE_FREE(alloc, relshp->params.cnstrs.ctxts_config);
  PSE_FREE(alloc, relshp->params.variations);
  PSE_FREE(alloc, relshp->params.ppoints_id);
}

static PSE_FINLINE enum pse_res_t
pseRelationshipCopy
  (struct pse_allocator_t* alloc,
   const struct pse_cpspace_relshp_params_t* src_params,
   const pse_clt_relshps_group_uid_t src_group_uid,
   struct pse_cpspace_relshp_t* dst)
{
  enum pse_res_t res = RES_OK;
  size_t i;
  assert(alloc && src_params && dst);

  /* Copy by value first */
  *dst = PSE_CPSPACE_RELSHP_NULL;
  dst->params = *src_params;
  dst->clt_group_uid = src_group_uid;

  /* Set to NULL the buffer that must be duplicated for easier error management */
  dst->params.ppoints_id = NULL;
  dst->params.variations = NULL;
  dst->params.cnstrs.funcs = NULL;
  dst->params.cnstrs.ctxts_config = NULL;

  /* Now duplicate buffers if needed */
  if( dst->params.ppoints_count > 0 ) {
    dst->params.ppoints_id = PSE_TYPED_ALLOC_ARRAY
      (alloc, pse_ppoint_id_t, dst->params.ppoints_count);
    PSE_VERIFY_OR_ELSE(dst->params.ppoints_id != NULL, res = RES_MEM_ERR; goto error);
    for(i = 0; i < dst->params.ppoints_count; ++i) {
      dst->params.ppoints_id[i] = src_params->ppoints_id[i];
    }
  }
  if( dst->params.variations_count > 0 ) {
    dst->params.variations = PSE_TYPED_ALLOC_ARRAY
      (alloc, pse_clt_ppoint_variation_uid_t, dst->params.variations_count);
    PSE_VERIFY_OR_ELSE(dst->params.variations != NULL, res = RES_MEM_ERR; goto error);
    for(i = 0; i < dst->params.variations_count; ++i) {
      dst->params.variations[i] = src_params->variations[i];
    }
  }
  if( dst->params.cnstrs.funcs_count > 0 ) {
    dst->params.cnstrs.funcs = PSE_TYPED_ALLOC_ARRAY
      (alloc, pse_relshp_cost_func_id_t, dst->params.cnstrs.funcs_count);
    PSE_VERIFY_OR_ELSE(dst->params.cnstrs.funcs != NULL, res = RES_MEM_ERR; goto error);
    for(i = 0; i < dst->params.cnstrs.funcs_count; ++i) {
      dst->params.cnstrs.funcs[i] = src_params->cnstrs.funcs[i];
    }
    if( src_params->cnstrs.ctxts_config ) {
      dst->params.cnstrs.ctxts_config = PSE_TYPED_ALLOC_ARRAY
        (alloc, pse_clt_cost_func_ctxt_config_t, dst->params.cnstrs.funcs_count);
      PSE_VERIFY_OR_ELSE
        (dst->params.cnstrs.ctxts_config != NULL,
        res = RES_MEM_ERR; goto error);
      for(i = 0; i < dst->params.cnstrs.funcs_count; ++i) {
        dst->params.cnstrs.ctxts_config[i] =
          src_params->cnstrs.ctxts_config[i];
      }
    }
  }

exit:
  return res;
error:
  pseRelationshipClean(alloc, dst);
  goto exit;
}

static PSE_FINLINE enum pse_res_t
pseRelationshipsParamsValidate
  (struct pse_cpspace_t* cps,
   const size_t count,
   const struct pse_cpspace_relshp_params_t* params)
{
  size_t i;
  enum pse_res_t res = RES_OK;
  assert(cps && count && params);
  (void)cps;
  for(i = 0; i < count; ++i) {
    const struct pse_cpspace_relshp_params_t* p = &params[i];
    if(  p->kind == PSE_RELSHP_KIND_INCLUSIVE
      && (  (0 == p->ppoints_count)
         || (NULL == p->ppoints_id))
      && (  (0 == p->variations_count)
         || (NULL == p->variations)) )
      return RES_BAD_ARG;

    /* TODO: check ppoints_id unicity */

    if( (0 != p->ppoints_count) && (NULL != p->ppoints_id) ) {
      PSE_TRY_CALL_OR_RETURN(res, pseParametricPointsIdsValidate
        (cps, p->ppoints_count, p->ppoints_id));
    }
    PSE_TRY_CALL_OR_RETURN(res, pseFunctorsIdsValidate
      (cps, p->cnstrs.funcs_count, p->cnstrs.funcs));
  }
  return RES_OK;
}

static PSE_FINLINE enum pse_res_t
pseRelationshipsIdsValidate
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids)
{
  size_t i;
  enum pse_res_t res = RES_OK;
  assert(cps && count && ids);
  for(i = 0; i < count; ++i) {
    const pse_relshp_id_t id = ids[i];
    if(  (id == PSE_RELSHP_ID_INVALID)
      || (id >= sb_count(cps->relshps)) )
      return RES_BAD_ARG;
    PSE_TRY_CALL_OR_RETURN(res, pseRelationshipsParamsValidate
      (cps, 1, &cps->relshps[id].params));
  }
  return RES_OK;
}

static PSE_INLINE void
pseConstrainedParameterSpaceRefRelease
  (pse_ref_t* ref)
{
  struct pse_cpspace_t* cps = PSE_CONTAINER_OF(ref, struct pse_cpspace_t, ref);

  /* Ref counting should not accept to release the CPS before its values and
   * its exploration contexts. */
  assert(hmlenu(cps->values) == 0);
  assert(hmlenu(cps->exp_ctxts) == 0);

  pseConstrainedParameterSpaceDestroy(cps);
}

/******************************************************************************
 *
 * PRIVATE API - Constrained Parameter Space
 *
 ******************************************************************************/

void
pseConstrainedParameterSpaceDestroy
  (struct pse_cpspace_t* cps)
{
  size_t i;
  assert(cps);

  PSE_CALL(pseDeviceConstrainedParameterSpaceUnregister(cps->dev, cps));
  for(i = 0; i < sb_count(cps->relshps_used); ++i) {
    pseRelationshipClean(cps->dev->allocator, &cps->relshps[i]);
  }
  for(i = 0; i < hmlenu(cps->relshps_groups); ++i) {
    sb_free(cps->relshps_groups[i].ids);
  }
  for(i = 0; i < sb_count(cps->pspaces); ++i) {
    psePSpaceParamsClean(&cps->pspaces[i]);
  }
  for(i = 0; i < hmlenu(cps->values); ++i) {
    pseConstrainedParameterSpaceValuesDestroy(cps->values[i].key);
  }
  for(i = 0; i < hmlenu(cps->exp_ctxts); ++i) {
    pseConstrainedParameterSpaceExplorationContextDestroy(cps->exp_ctxts[i].key);
  }
  sb_free(cps->pspaces_uid);
  sb_free(cps->pspaces);
  sb_free(cps->ppoints);
  sb_free(cps->ppoints_used);
  sb_free(cps->ppoints_free);
  sb_free(cps->relshps);
  sb_free(cps->relshps_used);
  sb_free(cps->relshps_free);
  hmfree(cps->relshps_groups);
  sb_free(cps->functors);
  sb_free(cps->functors_used);
  sb_free(cps->functors_free);
  hmfree(cps->values);
  hmfree(cps->exp_ctxts);
  PSE_FREE(cps->dev->allocator, cps);
}

PSE_INLINE enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesHas
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids)
{
  size_t i, j;
  bool found;
  assert(cps && count && uids);

  for(i = 0; i < count; ++i) {
    if( uids[i] == PSE_CLT_PSPACE_UID_INVALID )
      return RES_BAD_ARG;
    found = false;
    for(j = 0; j < sb_count(cps->pspaces_uid); ++j) {
      if( cps->pspaces_uid[j] == uids[i] ) {
        found = true;
        break;
      }
    }
    if( !found )
      return RES_NOT_FOUND;
  }
  return RES_OK;
}

PSE_INLINE enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesHasNot
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids)
{
  size_t i, j;
  assert(cps && count && uids);

  for(i = 0; i < count; ++i) {
    if( uids[i] == PSE_CLT_PSPACE_UID_INVALID )
      return RES_BAD_ARG;
    for(j = 0; j < sb_count(cps->pspaces_uid); ++j) {
      if( cps->pspaces_uid[j] == uids[i] ) {
        return RES_ALREADY_EXISTS;
      }
    }
  }
  return RES_OK;

}

const struct pse_pspace_params_t*
pseConstrainedParameterSpaceParameterSpaceGet
  (struct pse_cpspace_t* cps,
   const pse_clt_pspace_uid_t uid)
{
  size_t i;
  assert(cps && (uid != PSE_CLT_PSPACE_UID_INVALID));
  assert(sb_count(cps->pspaces) == sb_count(cps->pspaces_uid));

  for(i = 0; i < sb_count(cps->pspaces_uid); ++i) {
    if( cps->pspaces_uid[i] == uid )
      return &cps->pspaces[i];
  }
  return NULL;
}

PSE_FINLINE bool
pseConstrainedParameterSpaceParametricPointExists
  (struct pse_cpspace_t* cps,
   pse_ppoint_id_t id)
{
  size_t i;
  assert(cps);
  if(  (id == PSE_PPOINT_ID_INVALID)
    || (id >= sb_count(cps->ppoints)) )
    return false;

  /* TODO: heavy linear search, we should have hashmap somewhere. At least, we
   * should choose between ppoints_free and ppoints_used to go through the
   * smaller one. */
  for(i = 0; i < sb_count(cps->ppoints_free); ++i) {
    if( id == cps->ppoints_free[i] )
      return false;
  }
  return true;
}

/******************************************************************************
 *
 * PUBLIC API - Constrained Parameter Space
 *
 ******************************************************************************/

enum pse_res_t
pseConstrainedParameterSpaceCreate
  (struct pse_device_t* dev,
   const struct pse_cpspace_params_t* params,
   struct pse_cpspace_t** out_cps)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_t* cps = NULL;
  if( !dev || !params || !out_cps )
    return RES_BAD_ARG;

  cps = PSE_TYPED_ALLOC(dev->allocator, struct pse_cpspace_t);
  PSE_VERIFY_OR_ELSE(cps != NULL, res = RES_MEM_ERR; goto error);
  *cps = PSE_CPSPACE_NULL;
  pseRefCountInit(&cps->ref);
  PSE_CALL_OR_GOTO(res,error, pseDeviceConstrainedParameterSpaceRegister(dev, cps));
  cps->dev = dev;
  sb_reserve_more(cps->ppoints, 32);
  sb_reserve_more(cps->ppoints_used, 32);
  sb_reserve_more(cps->ppoints_free, 32);
  sb_reserve_more(cps->relshps, 128);
  sb_reserve_more(cps->relshps_used, 128);
  sb_reserve_more(cps->relshps_free, 16);
  sb_reserve_more(cps->functors, 128);
  sb_reserve_more(cps->functors_used, 128);
  sb_reserve_more(cps->functors_free, 16);

  *out_cps = cps;

exit:
  return res;
error:
  if( NULL != cps ) {
    PSE_CALL(pseDeviceConstrainedParameterSpaceUnregister(dev, cps));
    sb_free(cps->ppoints);
    sb_free(cps->ppoints_used);
    sb_free(cps->ppoints_free);
    sb_free(cps->relshps);
    sb_free(cps->relshps_used);
    sb_free(cps->relshps_free);
    sb_free(cps->functors);
    sb_free(cps->functors_used);
    sb_free(cps->functors_free);
    PSE_FREE(dev->allocator, cps);
  }
  goto exit;
}

enum pse_res_t
pseConstrainedParameterSpaceRefAdd
  (struct pse_cpspace_t* cps)
{
  if( !cps )
    return RES_BAD_ARG;
  pseRefCountInc(&cps->ref);
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceRefSub
  (struct pse_cpspace_t* cps)
{
  if( !cps )
    return RES_BAD_ARG;
  pseRefCountDec(&cps->ref, pseConstrainedParameterSpaceRefRelease);
  return RES_OK;
}

static PSE_INLINE enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesVariationsRemoveImplem
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids,
   const size_t variations_count,
   const pse_clt_ppoint_variation_uid_t* variations,
   const bool fail_if_not_exist)
{
  enum pse_res_t res = RES_OK;
  struct pse_pspace_params_t* ps = NULL;
  bool found = false;
  size_t i, j, k;
  if( !cps || (count && !uids) || (variations_count && !variations) )
    return RES_BAD_ARG;
  if( count <= 0  )
    return RES_OK; /* Nothing to do */

  /* Check for the existence of the given pspace UIDs */
  PSE_TRY_CALL_OR_RETURN(res, pseConstrainedParameterSpaceParameterSpacesHas
    (cps, count, uids));

  if( variations_count <= 0 )
    return RES_OK; /* Nothing to do */

  /* Check validity of the variations UIDs */
  for(i = 0; i < variations_count; ++i) {
    if( variations[i] == PSE_CLT_PPOINT_VARIATION_UID_INVALID )
      return RES_BAD_ARG;
  }

  /* TODO: here, we do lots of linear searchs that could be expensive if we
   * have lots of pspace and lots of variations. Think about using a hash map.*/

  /* We go through all pspace that need to add these variations */
  for(i = 0; i < count; ++i) {
    ps = NULL;
    /* Get the pspace params */
    for(j = 0; j < sb_count(cps->pspaces_uid); ++j) {
      if( uids[i] == cps->pspaces_uid[j] ) {
        ps = &cps->pspaces[j];
        break;
      }
    }
    assert(ps);
    for(j = 0; j < variations_count; ++j) {
      /* First, check if this pspace has this variation */
      found = false;
      for(k = 0; k < ps->variations_count; ++k) {
        if( ps->variations[k] == variations[j] ) {
          found = true;
          break;
        }
      }
      /* If yes, remove it! */
      if( found ) {
        sb_delat(ps->variations, j);
        --ps->variations_count;
      } else if( fail_if_not_exist ) {
        res = RES_NOT_FOUND;
      }
    }
  }
  return res;
}

/******************************************************************************
 *
 * PUBLIC API - Constrained Parameter Space Parameter Space
 *
 ******************************************************************************/

enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesDeclare
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids,
   const struct pse_pspace_params_t* params)
{
  enum pse_res_t res = RES_OK;
  size_t i, j, prev_count;
  if( !cps || (count && (!uids || !params)) )
    return RES_BAD_ARG;
  if( count <= 0 )
    return RES_OK; /* Nothing to do */

  /* Check the validity of the parameters */
  PSE_TRY_CALL_OR_RETURN(res, psePSpaceParamsValidate(params));

  /* Check for the existence of the given UIDs */
  PSE_TRY_CALL_OR_RETURN(res, pseConstrainedParameterSpaceParameterSpacesHasNot
    (cps, count, uids));

  sb_reserve_more(cps->pspaces_uid, count);
  sb_reserve_more(cps->pspaces, count);
  prev_count = sb_count(cps->pspaces);
  for(i = 0; i < count; ++i) {
    sb_push(cps->pspaces_uid, uids[i]);
    sb_push(cps->pspaces, PSE_PSPACE_PARAMS_NULL);
    PSE_CALL_OR_GOTO(res,error, psePSpaceParamsCopy
      (&params[i], &sb_last(cps->pspaces)));
  }

exit:
  return res;
error:
  for(j = 0; j < i; ++j) {
    psePSpaceParamsClean(&cps->pspaces[j+prev_count]);
  }
  sb_setn(cps->pspaces_uid, prev_count);
  sb_setn(cps->pspaces, prev_count);
  goto exit;
}

enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesForget
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids)
{

  enum pse_res_t res = RES_OK;
  size_t i, j;
  if( !cps || (count && !uids) )
    return RES_BAD_ARG;
  if( count <= 0 )
    return RES_OK; /* Nothing to do */

  /* Check for the existence of the given UIDs */
  PSE_TRY_CALL_OR_RETURN(res, pseConstrainedParameterSpaceParameterSpacesHas
    (cps, count, uids));

  for(i = 0; i < count; ++i) {
    for(j = 0; j < sb_count(cps->pspaces_uid); ++j) {
      if( cps->pspaces_uid[j] == uids[i] ) {
        /* We have found the parameter space */
        psePSpaceParamsClean(&cps->pspaces[j]);
        sb_delat(cps->pspaces_uid, j);
        sb_delat(cps->pspaces, j);
        break;
      }
    }
  }
  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesVariationsAdd
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids,
   const size_t variations_count,
   const pse_clt_ppoint_variation_uid_t* variations)
{
  enum pse_res_t res = RES_OK;
  struct pse_pspace_params_t* ps = NULL;
  bool found = false;
  size_t i, j, k;
  if( !cps || (count && !uids) || (variations_count && !variations) )
    return RES_BAD_ARG;
  if( count <= 0  )
    return RES_OK; /* Nothing to do */

  /* Check for the existence of the given pspace UIDs */
  PSE_TRY_CALL_OR_RETURN(res, pseConstrainedParameterSpaceParameterSpacesHas
    (cps, count, uids));

  if( variations_count <= 0 )
    return RES_OK; /* Nothing to do */

  /* Check validity of the variations UIDs */
  for(i = 0; i < variations_count; ++i) {
    if( variations[i] == PSE_CLT_PPOINT_VARIATION_UID_INVALID )
      return RES_BAD_ARG;
  }

  /* TODO: here, we do lots of linear searchs that could be expensive if we
   * have lots of pspace and lots of variations. Think about using a hash map.*/

  /* We go through all pspace that need to add these variations */
  for(i = 0; i < count; ++i) {
    ps = NULL;
    /* Get the pspace params */
    for(j = 0; j < sb_count(cps->pspaces_uid); ++j) {
      if( uids[i] == cps->pspaces_uid[j] ) {
        ps = &cps->pspaces[j];
        break;
      }
    }
    assert(ps);
    /* Reserve memory space and add each variation */
    sb_reserve_more(ps->variations, variations_count);
    for(j = 0; j < variations_count; ++j) {
      /* First, check if this pspace already has this variation */
      found = false;
      for(k = 0; k < ps->variations_count; ++k) {
        if( ps->variations[k] == variations[j] ) {
          found = true;
          break;
        }
      }
      /* If not, add it! */
      if( !found ) {
        sb_push(ps->variations, variations[j]);
        ++ps->variations_count;
      }
    }
  }
  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesVariationsRemove
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids,
   const size_t variations_count,
   const pse_clt_ppoint_variation_uid_t* variations)
{
  return pseConstrainedParameterSpaceParameterSpacesVariationsRemoveImplem
    (cps, count, uids, variations_count, variations, true);
}

enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids,
   const size_t variations_count,
   const pse_clt_ppoint_variation_uid_t* variations)
{
  return pseConstrainedParameterSpaceParameterSpacesVariationsRemoveImplem
    (cps, count, uids, variations_count, variations, false);
}

/******************************************************************************
 *
 * PUBLIC API - Constrained Parameter Space Parametric Point
 *
 ******************************************************************************/

enum pse_res_t
pseConstrainedParameterSpaceParametricPointsAdd
  (struct pse_cpspace_t* cps,
   const size_t count,
   const struct pse_ppoint_params_t* params,
   pse_ppoint_id_t* ids)
{
  enum pse_res_t res = RES_OK;
  size_t i;
  if( !cps || (count && (!params || !ids)) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that the parameters are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseParametricPointsParamsValidate
    (cps, count, params));

  /* TODO: allocate once cps->ppoints_used as we know the size we will add */

  /* First, try to use freed parametric points */
  for(i = 0; i < sb_count(cps->ppoints_free) && i < count; ++i) {
    const pse_ppoint_id_t ppid = cps->ppoints_free[i];
    cps->ppoints[ppid] = params[i];
    ids[i] = ppid;
    sb_push(cps->ppoints_used, ppid);
  }
  /* Remove reused freed parametric points */
  sb_delnat(cps->ppoints_free, 0, i);

  /* Check if we have remaining parametric points to register and create new
   * parametric points if needed */
  if( i < count ) {
    pse_ppoint_id_t ppid = sb_count(cps->ppoints);
    (void)sb_add(cps->ppoints, count-i);
    for(; i < count; ++i) {
      cps->ppoints[ppid] = params[i];
      ids[i] = ppid;
      sb_push(cps->ppoints_used, ppid);
      ++ppid;
    }
  }

  /* Check consistency */
  assert
    (  sb_count(cps->ppoints)
    == sb_count(cps->ppoints_free) + sb_count(cps->ppoints_used));

  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceParametricPointsRemove
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_ppoint_id_t* ids)
{
  enum pse_res_t res = RES_OK;
  size_t i, j;
  bool found = false;
  (void)found;
  if( !cps || (count && !ids) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that the ids are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseParametricPointsIdsValidate(cps, count, ids));

  /* TODO: do something smarter than this... */
  for(i = 0; i < count; ++i) {
    const pse_ppoint_id_t ppid = ids[i];
    found = false;
    for(j = 0; j < sb_count(cps->ppoints_used); ++j) {
      if( cps->ppoints_used[j] == ppid ) {
        sb_delat(cps->ppoints_used, j);
        found = true;
        break;
      }
    }
    assert(found);
    cps->ppoints[ppid] = PSE_PPOINT_PARAMS_NULL;
    sb_push(cps->ppoints_free, ppid);
  }

  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceParametricPointsClear
  (struct pse_cpspace_t* cps)
{
  size_t i;
  if( !cps )
    return RES_BAD_ARG;

  for(i = 0; i < sb_count(cps->ppoints_used); ++i) {
    const pse_ppoint_id_t ppid = cps->ppoints_used[i];
    cps->ppoints[ppid] = PSE_PPOINT_PARAMS_NULL;
    sb_push(cps->ppoints_free, ppid);
  }
  sb_setn(cps->ppoints_used, 0);

  return RES_OK;
}

PSE_INLINE size_t
pseConstrainedParameterSpaceParametricPointsCountGet
  (struct pse_cpspace_t* cps)
{
  return cps ? sb_count(cps->ppoints_used) : 0;
}

/******************************************************************************
 *
 * PUBLIC API - Constrained Parameter Space Relationship Cost Functors
 *
 ******************************************************************************/

enum pse_res_t
pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
  (struct pse_cpspace_t* cps,
   const size_t count,
   struct pse_relshp_cost_func_params_t* params,
   pse_relshp_cost_func_id_t* ids)
{
  enum pse_res_t res = RES_OK;
  size_t i;
  if( !cps || (count && (!params || !ids)) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that params are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseFunctorsParamsValidate(cps, count, params));

  /* TODO: allocate once cps->functors_used as we know the size we will add */

  /* Try to use freed functors first */
  for(i = 0; i < sb_count(cps->functors_free) && i < count; ++i) {
    const pse_relshp_cost_func_id_t rcfid = cps->functors_free[i];
    cps->functors[rcfid] = params[i];
    ids[i] = rcfid;
    sb_push(cps->functors_used, rcfid);
  }
  /* Remove reused freed functors */
  sb_delnat(cps->functors_free, 0, i);

  /* Check if we have remaining functors to register and create new functors if
   * needed */
  if( i < count ) {
    pse_relshp_cost_func_id_t rcfid = sb_count(cps->functors);
    (void)sb_add(cps->functors, count-i);
    for(; i < count; ++i) {
      cps->functors[rcfid] = params[i];
      ids[i] = rcfid;
      sb_push(cps->functors_used, rcfid);
      ++rcfid;
    }
  }

  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_cost_func_id_t* ids)
{
  enum pse_res_t res = RES_OK;
  size_t i, j;
  if( !cps || (count && !ids) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that the ids are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseFunctorsIdsValidate(cps, count, ids));

  /* TODO: do something smarter than this... */
  for(i = 0; i < count; ++i) {
    const pse_relshp_cost_func_id_t fid = ids[i];
    cps->functors[fid] = PSE_RELSHP_COST_FUNC_PARAMS_NULL;
    sb_push(cps->functors_free, fid);
    for(j = 0; j < sb_count(cps->functors_used); ++j) {
      if( cps->functors_used[j] == fid ) {
        sb_delat(cps->functors_used, j);
        break;
      }
    }
  }

  return res;
}

/******************************************************************************
 *
 * PUBLIC API - Constrained Parameter Space Relationship
 *
 ******************************************************************************/

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsAdd
  (struct pse_cpspace_t* cps,
   const pse_clt_relshps_group_uid_t group_uid,
   const size_t count,
   struct pse_cpspace_relshp_params_t* params,
   pse_relshp_id_t* ids)
{
  enum pse_res_t res = RES_OK;
  size_t i, j, init_size;
  if( !cps || (count && (!params || !ids)) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that the params are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseRelationshipsParamsValidate(cps, count, params));

  /* TODO: allocate once cps->relshps_used as we know the size we will add */

  init_size = sb_count(cps->relshps);

  /* First, try to use freed parametric points */
  for(i = 0; i < sb_count(cps->relshps_free) && i < count; ++i) {
    const pse_relshp_id_t rid = cps->relshps_free[i];
    PSE_CALL_OR_GOTO(res,error, pseRelationshipCopy
      (cps->dev->allocator, &params[i], group_uid, &cps->relshps[rid]));
    ids[i] = rid;
    sb_push(cps->relshps_used, rid);
  }
  /* Remove reused freed parametric points */
  sb_delnat(cps->relshps_free, 0, i);

  /* Check if we have remaining parametric points to register and create new
   * parametric points if needed */
  if( i < count ) {
    pse_relshp_id_t rid = sb_count(cps->relshps);
    (void)sb_add(cps->relshps, count-i);
    for(; i < count; ++i) {
      PSE_CALL_OR_GOTO(res,error, pseRelationshipCopy
        (cps->dev->allocator, &params[i], group_uid, &cps->relshps[rid]));
      ids[i] = rid;
      sb_push(cps->relshps_used, rid);
      ++rid;
    }
  }

  if( group_uid != PSE_CLT_RELSHPS_GROUP_UID_INVALID ) {
    struct pse_relshps_group_t* grp = NULL;
    if( hmgeti(cps->relshps_groups, group_uid) < 0 ) {
      /* The first time we use this group, add it first */
      struct pse_relshps_group_t new_grp = PSE_RELSHPS_GROUP_NULL;
      new_grp.key = group_uid;
      hmputs(cps->relshps_groups, new_grp);
    }
    grp = &hmgets(cps->relshps_groups, group_uid);
    sb_reserve_more(grp->ids, count);
    for(i = 0; i < count; ++i) {
      sb_push(grp->ids, ids[i]);
    }
  }

exit:
  return res;
error:
  /* Clean relationships parameters and add the id to the free list */
  for(j = 0; j < i; ++j) {
    const pse_relshp_id_t rid = ids[j];
    pseRelationshipClean(cps->dev->allocator, &cps->relshps[rid]);
    sb_push(cps->relshps_free, rid);
  }
  /* Restore the size of the other buffers, but keep the allocated memory */
  sb_setn(cps->relshps_used, sb_count(cps->relshps_used) - i);
  sb_setn(cps->relshps, init_size);
  goto exit;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsRemove
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids)
{
  enum pse_res_t res = RES_OK;
  size_t i, j;
  if( !cps || (count && !ids) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that the ids are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseRelationshipsIdsValidate(cps, count, ids));

  /* TODO: do something smarter than this... */
  for(i = 0; i < count; ++i) {
    const pse_relshp_id_t rid = ids[i];
    pseRelationshipClean(cps->dev->allocator, &cps->relshps[rid]);
    sb_push(cps->relshps_free, rid);
    for(j = 0; j < sb_count(cps->relshps_used); ++j) {
      if( cps->relshps_used[j] == rid ) {
        sb_delat(cps->relshps_used, j);
        break;
      }
    }
  }

  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsRemoveByGroup
  (struct pse_cpspace_t* cps,
   const pse_clt_relshps_group_uid_t group_uid)
{
  enum pse_res_t res = RES_OK;
  struct pse_relshps_group_t* grp = NULL;
  size_t i, j, count;
  if( !cps || (group_uid == PSE_CLT_RELSHPS_GROUP_UID_INVALID) )
    return RES_BAD_ARG;

  if( hmgeti(cps->relshps_groups, group_uid) < 0 )
    return RES_NOT_FOUND;
  grp = &hmgets(cps->relshps_groups, group_uid);

  count = sb_count(grp->ids);
  for(i = 0; i < count; ++i) {
    const pse_relshp_id_t rid = grp->ids[i];
    pseRelationshipClean(cps->dev->allocator, &cps->relshps[rid]);
    sb_push(cps->relshps_free, rid);
    for(j = 0; j < sb_count(cps->relshps_used); ++j) {
      if( cps->relshps_used[j] == rid ) {
        sb_delat(cps->relshps_used, j);
        break;
      }
    }
  }

  sb_free(grp->ids);
  (void)hmdel(cps->relshps_groups, group_uid);

  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsClear
  (struct pse_cpspace_t* cps)
{
  size_t i;
  if( !cps )
    return RES_BAD_ARG;

  for(i = 0; i < sb_count(cps->relshps_used); ++i) {
    pseRelationshipClean(cps->dev->allocator, &cps->relshps[i]);
    sb_push(cps->relshps_free, cps->relshps_used[i]);
  }
  sb_setn(cps->relshps_used, 0);

  return RES_OK;
}

size_t
pseConstrainedParameterSpaceRelationshipsCountGet
  (struct pse_cpspace_t* cps)
{
  return cps ? sb_count(cps->relshps_used) : 0;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsParamsGet
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids,
   struct pse_cpspace_relshp_params_t* params)
{
  enum pse_res_t res = RES_OK;
  size_t i;
  if( !cps || (count && (!ids || !params)) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that the ids are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseRelationshipsIdsValidate(cps, count, ids));

  /* Everything is OK, copy the params */
  for(i = 0; i < count; ++i) {
    params[i] = cps->relshps[ids[i]].params;
  }
  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsStateSet
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids,
   const enum pse_cpspace_relshp_state_t* states)
{
  enum pse_res_t res = RES_OK;
  size_t i;
  if( !cps || (count && (!ids || !states)) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that the ids are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseRelationshipsIdsValidate(cps, count, ids));

  for(i = 0; i < count; ++i) {
    cps->relshps[ids[i]].state = states[i];
  }

  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsSameStateSet
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids,
   const enum pse_cpspace_relshp_state_t state)
{
  enum pse_res_t res = RES_OK;
  size_t i;
  if( !cps || (count && !ids) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that the ids are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseRelationshipsIdsValidate(cps, count, ids));

  for(i = 0; i < count; ++i) {
    cps->relshps[ids[i]].state = state;
  }

  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsStateSetByGroup
  (struct pse_cpspace_t* cps,
   const pse_clt_relshps_group_uid_t group_uid,
   const enum pse_cpspace_relshp_state_t state)
{
  enum pse_res_t res = RES_OK;
  struct pse_relshps_group_t* grp = NULL;
  size_t i, count;
  if( !cps || (group_uid == PSE_CLT_RELSHPS_GROUP_UID_INVALID) )
    return RES_BAD_ARG;

  if( hmgeti(cps->relshps_groups, group_uid) < 0 )
    return RES_NOT_FOUND;
  grp = &hmgets(cps->relshps_groups, group_uid);

  count = sb_count(grp->ids);
  for(i = 0; i < count; ++i) {
    cps->relshps[grp->ids[i]].state = state;
  }

  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsStateGet
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids,
   enum pse_cpspace_relshp_state_t* states)
{
  enum pse_res_t res = RES_OK;
  size_t i;
  if( !cps || (count && (!ids || !states)) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  /* First, check that the ids are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseRelationshipsIdsValidate(cps, count, ids));

  for(i = 0; i < count; ++i) {
    states[i] = cps->relshps[ids[i]].state;
  }

  return res;
}

enum pse_res_t
pseConstrainedParameterSpaceRelationshipsGroupStateGet
  (struct pse_cpspace_t* cps,
   const pse_clt_relshps_group_uid_t group_uid,
   enum pse_cpspace_relshps_group_state_t* grp_state)
{
  enum pse_res_t res = RES_OK;
  struct pse_relshps_group_t* grp = NULL;
  size_t i, count;
  PSE_STATIC_ASSERT
    (   (int)PSE_CPSPACE_RELSHPS_GROUP_STATE_DISABLED
     == (int)PSE_CPSPACE_RELSHP_STATE_DISABLED,
     Inconsistent_enum_values);
  PSE_STATIC_ASSERT
    (   (int)PSE_CPSPACE_RELSHPS_GROUP_STATE_ENABLED
     == (int)PSE_CPSPACE_RELSHP_STATE_ENABLED,
     Inconsistent_enum_values);
  if( !cps || (group_uid == PSE_CLT_RELSHPS_GROUP_UID_INVALID) || !grp_state )
    return RES_BAD_ARG;

  if( hmgeti(cps->relshps_groups, group_uid) < 0 )
    return RES_BAD_ARG;
  grp = &hmgets(cps->relshps_groups, group_uid);

  count = sb_count(grp->ids);
  if( !count )
    return RES_OK;

  *grp_state = (int)cps->relshps[grp->ids[0]].state;
  for(i = 1; i < count; ++i) {
    if( (int)*grp_state != (int)cps->relshps[grp->ids[i]].state ) {
      *grp_state = PSE_CPSPACE_RELSHPS_GROUP_STATE_ENABLED_PARTIALLY;
      break; /* Partially enabled state will not change with other values */
    }
  }
  return res;
}
