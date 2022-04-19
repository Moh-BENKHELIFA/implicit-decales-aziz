#ifndef PSE_REF_COUNT_H
#define PSE_REF_COUNT_H

#include "pse_api.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

typedef pse_atomic_t pse_ref_t;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_INLINE_API void
pseRefCountInit
  (pse_ref_t* ref)
{
  *ref = 1;
}

PSE_INLINE_API void
pseRefCountInc
  (pse_ref_t* ref)
{
  PSE_ATOMIC_INC(ref);
}

PSE_INLINE_API int
pseRefCountDec
  (pse_ref_t* ref,
   void (*release)(pse_ref_t* ref))
{
  pse_atomic_t cur;
  assert(ref != NULL);
  assert(release != NULL);

  cur = PSE_ATOMIC_DEC(ref);
  assert(cur >= 0);
  if( cur == 0 ) {
    release(ref);
    return 1;
  }
  return 0;
}

PSE_API_END

#endif /* PSE_REF_COUNT_H */
