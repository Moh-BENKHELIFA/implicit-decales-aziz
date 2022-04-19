#ifndef PSE_ALLOCATOR_H
#define PSE_ALLOCATOR_H

#include "pse_api.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

typedef void* (*pse_alloc_cb)(void* self, size_t memsize);
typedef void* (*pse_alloc_aligned_cb)(void* self, size_t memsize, size_t memalign);
typedef void* (*pse_alloc_array_cb)(void* self, size_t elem_memsize, size_t count);
typedef void* (*pse_alloc_array_aligned_cb)(void* self, size_t elem_memsize, size_t elem_memalign, size_t count);
typedef void* (*pse_realloc_cb)(void* self, void* ptr, size_t newsize);
typedef void  (*pse_free_cb)(void* self, void* ptr);
typedef size_t (*pse_mem_size_cb)(void* self, void* ptr);

struct pse_allocator_t {
  void* self; /*! Will be passed to callbacks */
  pse_alloc_cb alloc;
  pse_alloc_aligned_cb alloc_aligned;
  pse_alloc_array_cb alloc_array;
  pse_alloc_array_aligned_cb alloc_array_aligned;
  pse_realloc_cb realloc;
  pse_free_cb free;
  pse_mem_size_cb mem_size;
};

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_ALLOCATOR_NULL_                                                    \
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }

static const struct pse_allocator_t PSE_ALLOCATOR_NULL =
  PSE_ALLOCATOR_NULL_;

/******************************************************************************
 *
 * MACROS
 *
 ******************************************************************************/

#define PSE_ALLOC(a,memsize)                                                   \
  ((a)->alloc((a)->self, (memsize)))
#define PSE_ALLOC_ALIGNED(a,memsize,memalign)                                  \
  ((a)->alloc_aligned((a)->self, (memsize), (memalign)))
#define PSE_ALLOC_ARRAY(a,memelemsize,elemcount)                               \
  ((a)->alloc_array((a)->self, (memelemsize), (elemcount)))
#define PSE_ALLOC_ARRAY_ALIGNED(a,memelemsize,elemcount,memelemalign)          \
  ((a)->alloc_array_aligned((a)->self, (memelemsize), (elemcount), (memelemalign)))
#define PSE_REALLOC(a,ptr,newsize)                                             \
  ((a)->realloc((a)->self, (ptr), (newsize)))
#define PSE_FREE(a,ptr)                                                        \
  ((a)->free((a)->self, (ptr)))
#define PSE_MEM_SIZE(a,ptr)                                                    \
  ((a)->mem_size((a)->self, (ptr)))

#define PSE_TYPED_ALLOC(a,type)                                                \
  ((type*)PSE_ALLOC((a),sizeof(type)))
#define PSE_TYPED_ALLOC_ALIGNED(a,type,memalign)                               \
  ((type*)PSE_ALLOC_ALIGNED((a),sizeof(type),(memalign)))
#define PSE_TYPED_ALLOC_ARRAY(a,type,count)                                    \
  ((type*)PSE_ALLOC_ARRAY((a),sizeof(type),(count)))
#define PSE_TYPED_ALLOC_ARRAY_ALIGNED(a,type,memalign,count)                   \
  ((type*)PSE_ALLOC_ALIGNED((a),sizeof(type),count,(memalign)))
#define PSE_TYPED_REALLOC(a,ptr,type)                                          \
  ((type*)PSE_REALLOC((a),(ptr),sizeof(type)))

/******************************************************************************
 *
 * PUBLIC GLOBAL VARIABLES
 *
 ******************************************************************************/

PSE_API struct pse_allocator_t PSE_ALLOCATOR_CSTDLIB;
PSE_API struct pse_allocator_t PSE_ALLOCATOR_DEFAULT;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_INLINE_API size_t
pseAllocationAlignementAdjust
  (const size_t memalign)
{
  /* We ensure that the memory alignement is at least the size of a pointer
   * and a power of 2. */
  const size_t adjusted_memalign = PSE_MAX(memalign, sizeof(void*));
  assert(adjusted_memalign % sizeof(void*) == 0);
  assert(PSE_IS_POW2(adjusted_memalign));
  return adjusted_memalign;
}

PSE_INLINE_API size_t
pseAllocationArrayElemSizeAdjust
  (const size_t elem_memsize,
   const size_t elem_memalign)
{
  /* Round elem_memsize to the upper multiple of elem_memalign. We need this to
   * have all elements aligned on elem_memalign for array allocation. */
  /* NOTE: all these operations are integer operations! */
  return
      (1 + ((elem_memsize - 1) / elem_memalign)) /* ceil(elem_memsize/elem_memalign) */
    * elem_memalign;
}

PSE_API_END

#endif /* PSE_ALLOCATOR_H */
