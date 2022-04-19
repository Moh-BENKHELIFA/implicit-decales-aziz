#include "pse_slz_buffer_file.h"
#include "pse_slz_buffer.h"

#include <pse_allocator.h>

#include <stdio.h>

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_file_buffer_data_t {
  struct pse_serialization_buffer_file_params_t params;
  FILE* handle;
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_FILE_BUFFER_DATA_NULL_                                             \
  { PSE_SERIALIZATION_BUFFER_FILE_PARAMS_NULL_, NULL }

static const struct pse_file_buffer_data_t PSE_FILE_BUFFER_DATA_NULL =
  PSE_FILE_BUFFER_DATA_NULL_;

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static PSE_FINLINE const char*
pseFileModeAndTypeToFOpenModes
  (const enum pse_file_mode_t mode,
   const enum pse_file_type_t type)
{
  switch(mode) {
    case PSE_FILE_MODE_READ:
      return type == PSE_FILE_TYPE_BINARY ? "rb" : "r";
    case PSE_FILE_MODE_WRITE:
      return type == PSE_FILE_TYPE_BINARY ? "wb" : "w";
    case PSE_FILE_MODE_WRITE_APPEND:
      return type == PSE_FILE_TYPE_BINARY ? "ab" : "a";
    default: assert(false);
  }
  return "";
}

static enum pse_res_t
pseSerializationBufferFileRead
  (struct pse_serialization_context_t* ctxt,
   void* user_data,
   const size_t bytes_count,
   void* output_buffer)
{
  struct pse_file_buffer_data_t* data = (struct pse_file_buffer_data_t*)user_data;
  size_t read_count = 0;
  assert(data);
  (void)ctxt;
  read_count = fread(output_buffer, 1, bytes_count, data->handle);
  return (read_count == bytes_count)
    ? RES_OK
    : RES_IO_ERR;
}

static enum pse_res_t
pseSerializationBufferFileWrite
  (struct pse_serialization_context_t* ctxt,
   void* user_data,
   const size_t bytes_count,
   const void* input_buffer)
{
  struct pse_file_buffer_data_t* data = (struct pse_file_buffer_data_t*)user_data;
  size_t written_count = 0;
  assert(data);
  (void)ctxt;
  written_count = fwrite(input_buffer, 1, bytes_count, data->handle);
  return (written_count == bytes_count)
    ? RES_OK
    : RES_IO_ERR;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseSerializationBufferFileCreateFromFilePath
  (const char* filepath,
   struct pse_serialization_buffer_file_params_t* params,
   struct pse_serialization_buffer_t* buffer)
{
  enum pse_res_t res = RES_OK;
  struct pse_file_buffer_data_t* data = NULL;
  const char* modes = NULL;
  FILE* file = NULL;
  if( !filepath || !params || !buffer )
    return RES_BAD_ARG;

  modes = pseFileModeAndTypeToFOpenModes(params->mode, params->type);
  file = fopen(filepath, modes);
  PSE_VERIFY_OR_ELSE(file != NULL, res = RES_IO_ERR; goto error);

  data = PSE_TYPED_ALLOC(params->alloc, struct pse_file_buffer_data_t);
  PSE_VERIFY_OR_ELSE(data != NULL, res = RES_MEM_ERR; goto error);
  *data = PSE_FILE_BUFFER_DATA_NULL;
  data->params = *params;
  data->handle = file;

  buffer->read = (params->mode == PSE_FILE_MODE_READ)
    ? pseSerializationBufferFileRead
    : NULL;
  buffer->write = (params->mode == PSE_FILE_MODE_READ)
    ? NULL
    : pseSerializationBufferFileWrite;
  buffer->user_data = data;

exit:
  return res;
error:
  if( file )
    fclose(file);
  goto exit;
}

enum pse_res_t
pseSerializationBufferFileDestroy
  (struct pse_serialization_buffer_t* buffer)
{
  struct pse_file_buffer_data_t* data = NULL;
  if( !buffer )
    return RES_BAD_ARG;
  if(  buffer->read != pseSerializationBufferFileRead
    && buffer->write != pseSerializationBufferFileWrite )
    return RES_BAD_ARG;

  data = (struct pse_file_buffer_data_t*)buffer->user_data;
  if( data->handle )
    fclose(data->handle);
  PSE_FREE(data->params.alloc, data);

  return RES_OK;
}
