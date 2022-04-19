#include "pse_logger.h"

#include <stdio.h>

static PSE_INLINE void
pseLog_fprintf_stdout
  (void* self,
   const char* text)
{
  (void)self;
  fprintf(stdout, "%s", text);
}

static PSE_INLINE void
pseLog_fprintf_stderr
  (void* self,
   const char* text)
{
  (void)self;
  fprintf(stderr, "%s", text);
}

struct pse_logger_t PSE_LOGGER_STDOUT =
  { NULL, pseLog_fprintf_stdout, PSE_LOGGER_LEVEL_DEBUG };
struct pse_logger_t PSE_LOGGER_STDERR =
  { NULL, pseLog_fprintf_stderr, PSE_LOGGER_LEVEL_DEBUG };
