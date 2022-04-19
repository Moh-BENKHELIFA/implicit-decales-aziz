#ifndef PSE_SLZ_BUFFER_FILE_H
#define PSE_SLZ_BUFFER_FILE_H

#include "pse_slz_api.h"

#include <stdio.h>

PSE_API_BEGIN

struct pse_allocator_t;
struct pse_logger_t;
struct pse_serialization_buffer_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

enum pse_file_mode_t {
  PSE_FILE_MODE_READ,
  PSE_FILE_MODE_WRITE,
  PSE_FILE_MODE_WRITE_APPEND
};

enum pse_file_type_t {
  PSE_FILE_TYPE_TEXT,
  PSE_FILE_TYPE_BINARY
};

struct pse_serialization_buffer_file_params_t {
  struct pse_allocator_t* alloc;
  struct pse_logger_t* logger; /*!< May be NULL */
  enum pse_file_mode_t mode;
  enum pse_file_type_t type;
};

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_SERIALIZATION_BUFFER_FILE_PARAMS_NULL_                             \
  { NULL, NULL, PSE_FILE_MODE_READ, PSE_FILE_TYPE_TEXT }

static const struct pse_serialization_buffer_file_params_t PSE_SERIALIZATION_BUFFER_FILE_PARAMS_NULL =
  PSE_SERIALIZATION_BUFFER_FILE_PARAMS_NULL_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_SLZ_API enum pse_res_t
pseSerializationBufferFileCreateFromFilePath
  (const char* filepath,
   struct pse_serialization_buffer_file_params_t* params,
   struct pse_serialization_buffer_t* buffer);

PSE_SLZ_API enum pse_res_t
pseSerializationBufferFileDestroy
  (struct pse_serialization_buffer_t* buffer);

PSE_API_END

#endif /* PSE_SLZ_BUFFER_FILE_H */
