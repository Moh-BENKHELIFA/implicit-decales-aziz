#ifndef PSE_SLZ_BUFFER_H
#define PSE_SLZ_BUFFER_H

#include "pse_slz_api.h"

PSE_API_BEGIN

struct pse_serialization_context_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

typedef enum pse_res_t
(*pse_serialization_buffer_read_cb)
  (struct pse_serialization_context_t* ctxt,
   void* user_data,
   const size_t bytes_count,
   void* output_buffer);

typedef enum pse_res_t
(*pse_serialization_buffer_write_cb)
  (struct pse_serialization_context_t* ctxt,
   void* user_data,
   const size_t bytes_count,
   const void* input_buffer);

struct pse_serialization_buffer_t {
  pse_serialization_buffer_read_cb read;
  pse_serialization_buffer_write_cb write;
  void* user_data;
};

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_SERIALIZATION_BUFFER_NULL_                                         \
  { NULL, NULL, NULL }

static const struct pse_serialization_buffer_t PSE_SERIALIZATION_BUFFER_NULL =
  PSE_SERIALIZATION_BUFFER_NULL_;

PSE_API_END

#endif /* PSE_SLZ_BUFFER_H */
