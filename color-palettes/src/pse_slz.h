#ifndef PSE_SLZ_H
#define PSE_SLZ_H

#include "pse_slz_api.h"

PSE_API_BEGIN

struct pse_allocator_t;
struct pse_logger_t;
struct pse_serialization_buffer_t;

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_serialization_context_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

enum pse_serialization_mode_t {
  PSE_SERIALIZATION_MODE_READ,
  PSE_SERIALIZATION_MODE_WRITE
};

struct pse_serializer_params_t {
  struct pse_allocator_t* alloc;
  struct pse_logger_t* logger;
  const char* driver_filepath;
};

struct pse_serialization_context_params_t {
  enum pse_serialization_mode_t mode;
  struct pse_serialization_buffer_t* buffer;
};

struct pse_serializer_t {
  enum pse_res_t
  (*context_create)
    (struct pse_serializer_t* self,
     const struct pse_serialization_context_params_t* params,
     struct pse_serialization_context_t** ctxt);

  enum pse_res_t
  (*context_destroy)
    (struct pse_serializer_t* self,
     struct pse_serialization_context_t* ctxt);

  enum pse_res_t
  (*serialize_builtin)
    (struct pse_serialization_context_t* ctxt);
};

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_SLZ_API enum pse_res_t
pseSerializerCreate
  (const struct pse_serializer_params_t* params,
   struct pse_serializer_t** slzr);

PSE_API_END

#endif /* PSE_SLZ_H */
