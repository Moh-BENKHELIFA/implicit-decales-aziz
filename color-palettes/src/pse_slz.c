#include "pse_slz.h"

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseSerializerLoad
  (const struct pse_serializer_params_t* params,
   struct pse_serializer_t** slzr)
{
  if( !params || !slzr )
    return RES_BAD_ARG;
  /* TODO */
  return RES_NOT_SUPPORTED;
}
