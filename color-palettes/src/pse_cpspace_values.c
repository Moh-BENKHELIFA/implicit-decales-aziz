#include "pse_cpspace_values_p.h"
#include "pse_cpspace_p.h"
#include "pse_device_p.h"

#include <string.h>

#include "stb_ds.h"

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

/*! Return the minimal size of the attribute, i.e. the accumulated size of all
 * components, without padding. */
static PSE_FINLINE size_t
pseAttribPackedSizeGet
  (const struct pse_pspace_point_attrib_t* attrib)
{
  size_t i, size = 0;
  assert(attrib);
  for(i = 0; i < attrib->components_count; ++i) {
    switch(attrib->components[i].type) {
#   define PSE_TYPE(name,sib,ctype)                                            \
      case PSE_TYPE_## name ##_## sib: { size += sizeof(ctype); } break;
      PSE_ALL_TYPES
#   undef PSE_TYPE
      default: assert(false); break;
    }
  }
  return size;
}

static PSE_INLINE void
pseConstrainedParameterSpaceValuesRefRelease
  (pse_ref_t* ref)
{
  struct pse_cpspace_values_t* vals =
    PSE_CONTAINER_OF(ref, struct pse_cpspace_values_t, ref);
  struct pse_cpspace_t* cps = vals->cps;

  pseConstrainedParameterSpaceValuesDestroy(vals);
  (void)hmdel(cps->values,vals);
  PSE_CALL(pseConstrainedParameterSpaceRefSub(cps));
}

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

void
pseConstrainedParameterSpaceValuesDestroy
  (struct pse_cpspace_values_t* vals)
{
  assert(vals);
  PSE_FREE(vals->cps->dev->allocator, vals);
}

enum pse_res_t
pseConstrainedParameterSpaceValuesDataValidate
  (struct pse_cpspace_t* cps,
   const struct pse_cpspace_values_data_t* data)
{
  size_t i;
  assert(cps && data);
  if( data->pspace == PSE_CLT_PSPACE_UID_INVALID )
    return RES_BAD_ARG;

  switch(data->storage) {
    case PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_GLOBAL: {
      const struct pse_cpspace_values_data_accessors_global_t* accessors =
        &data->as.global;

      /* Check that we can at lest write or read values */
      if( !accessors->accessors.get && !accessors->accessors.set )
        return RES_BAD_ARG;
    } break;
    case PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_PER_ATTRIB: {
      const struct pse_pspace_params_t* pspace =
        pseConstrainedParameterSpaceParameterSpaceGet(cps, data->pspace);
      const struct pse_cpspace_values_data_accessors_per_attrib_t* accessors =
        &data->as.per_attrib;
      if( !pspace )
        return RES_BAD_ARG;

      /* Then verify that we have all the right attributes, well parametered */
      for(i = 0; i < PSE_POINT_ATTRIB_COUNT_; ++i) {
        const struct pse_attrib_value_accessors_t* aacc =
          &accessors->accessors[i];
        const struct pse_pspace_point_attrib_t* pattrib =
          &pspace->ppoint_params.attribs[i];

        /* Check if the attribute is defined and has a get/set. Or we must have
         * nothing at all. It's a XOR: both must be true or false. The NOT
         * operator (!) transform the pointers into booleans, that's how it
         * works like expected. */
        if( (!aacc->get && !aacc->set) != !pattrib->components )
          return RES_BAD_ARG;
        if( !aacc->get && !aacc->set )
          continue; /* This attribute is not used */
      }
    } break;
    default: return RES_BAD_ARG;
  }
  return RES_OK;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseConstrainedParameterSpaceValuesCreate
  (struct pse_cpspace_t* cps,
   const struct pse_cpspace_values_data_t* data,
   struct pse_cpspace_values_t** out_vals)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_values_t* vals = NULL;
  if( !cps || !data || !out_vals )
    return RES_BAD_ARG;

  /* First, validate the parameters */
  PSE_TRY_CALL_OR_RETURN(res, pseConstrainedParameterSpaceValuesDataValidate
    (cps, data));

  vals = PSE_TYPED_ALLOC(cps->dev->allocator, struct pse_cpspace_values_t);
  PSE_VERIFY_OR_ELSE(vals != NULL, res = RES_MEM_ERR; goto error);
  *vals = PSE_CPSPACE_VALUES_NULL;
  pseRefCountInit(&vals->ref);
  vals->cps = cps;
  vals->data = *data;

  /* Keep the values in the cps */
  {
    struct pse_values_entry_t entry;
    entry.key = vals;
    hmputs(cps->values, entry);
    /* Keep a reference on the cps as we need it */
    PSE_CALL(pseConstrainedParameterSpaceRefAdd(cps));
  }

  *out_vals = vals;

exit:
  return res;
error:
  if( vals ) {
    PSE_FREE(cps->dev->allocator, vals);
  }
  goto exit;
}

enum pse_res_t
pseConstrainedParameterSpaceValuesRefAdd
  (struct pse_cpspace_values_t* vals)
{
  if( !vals )
    return RES_BAD_ARG;
  pseRefCountInc(&vals->ref);
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceValuesRefSub
  (struct pse_cpspace_values_t* vals)
{
  if( !vals )
    return RES_BAD_ARG;
  pseRefCountDec(&vals->ref, pseConstrainedParameterSpaceValuesRefRelease);
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceValuesLock
  (struct pse_cpspace_values_t* vals,
   const struct pse_cpspace_values_lock_params_t* params,
   struct pse_cpspace_values_data_t** data)
{
  if( !vals || !params || !data )
    return RES_BAD_ARG;
  if( PSE_ATOMIC_SET(&vals->lock, 1) != 0 )
    return RES_BUSY;
  /* TODO: manage the rights from params */
  *data = &vals->data;
  return RES_OK;
}

enum pse_res_t
pseConstrainedParameterSpaceValuesUnlock
  (struct pse_cpspace_values_t* vals,
   struct pse_cpspace_values_data_t** data)
{
  if( !vals || !data || (&vals->data != *data) )
    return RES_BAD_ARG;
  if( PSE_ATOMIC_SET(&vals->lock, 0) == 0 )
    return RES_INVALID;
  *data = NULL;
  return RES_OK;
}
