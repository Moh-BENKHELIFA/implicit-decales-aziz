#ifndef PSE_LOGGER_H
#define PSE_LOGGER_H

#include "pse_api.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

enum pse_logger_level_t {
  PSE_LOGGER_LEVEL_QUIET,
  PSE_LOGGER_LEVEL_INFO,
  PSE_LOGGER_LEVEL_WARNING,
  PSE_LOGGER_LEVEL_ERROR,
  PSE_LOGGER_LEVEL_DEBUG
};

typedef void (*pse_log_cb)(void* self, const char* text);

struct pse_logger_t {
  void* self; /*! Will be passed to callbacks */
  pse_log_cb log;
  enum pse_logger_level_t level;
};

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_LOGGER_NULL_                                                       \
  { NULL, NULL, PSE_LOGGER_LEVEL_QUIET }

static const struct pse_logger_t PSE_LOGGER_NULL =
  PSE_LOGGER_NULL_;

/******************************************************************************
 *
 * MACROS
 *
 ******************************************************************************/

#define PSE_LOG(l,type,cstr)                                                   \
  if((l) && (l)->level >= PSE_LOGGER_LEVEL_##type) {                           \
    (l)->log((l)->self, cstr);                                                 \
  } (void)0

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_API struct pse_logger_t PSE_LOGGER_STDOUT;
PSE_API struct pse_logger_t PSE_LOGGER_STDERR;

PSE_API_END

#endif /* PSE_LOGGER_H */
