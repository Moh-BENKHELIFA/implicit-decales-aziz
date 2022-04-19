#include "pse_allocator.h"

#include <stdlib.h>
#include <errno.h>
#ifdef PSE_OS_MACH
# include <malloc/malloc.h>
#else
# include <malloc.h>
#endif

#if defined(PSE_OS_WINDOWS)
/******************************************************************************
 *
 * Allocation header for Windows
 *
 ******************************************************************************/

struct pse_alloc_header_t {
  size_t size;
  size_t alignment;
};

#define PSE_ALLOC_HEADER_MEMSIZE                                               \
  (sizeof(struct pse_alloc_header_t))
#define PSE_ALLOC_HEADER_GET(ptr)                                              \
  ((struct pse_alloc_header_t*)((uintptr_t)(ptr) - PSE_ALLOC_HEADER_MEMSIZE))

#endif

/******************************************************************************
 *
 * Declare the STD allocator, that will used by default
 *
 ******************************************************************************/

/*! \todo: do a specific implementation that keep track of the allocations in
 * order to allow extraction of memleaks and other useful information
 */

static PSE_FINLINE void*
pseStdAllocArray(void* self, size_t elem_memsize, size_t count)
{
  void* ptr = NULL;
  size_t allocated_size = 0;
  (void)self;

  if(  (elem_memsize <= 0)
    || (count <= 0) )
    return NULL;

  allocated_size = elem_memsize*count;
#if defined(PSE_OS_WINDOWS)
  const size_t PSE_ALIGNEMENT_DEFAULT = 16;
  allocated_size += PSE_ALLOC_HEADER_MEMSIZE;
  ptr = _aligned_offset_malloc
    (allocated_size, PSE_ALIGNEMENT_DEFAULT, PSE_ALLOC_HEADER_MEMSIZE);
  if( ptr ) {
    /* TODO: check errno */
    ptr = (void*)((uintptr_t)ptr + PSE_ALLOC_HEADER_MEMSIZE);
    PSE_ALLOC_HEADER_GET(ptr)->size = allocated_size;
    PSE_ALLOC_HEADER_GET(ptr)->alignment = PSE_ALIGNEMENT_DEFAULT;
  }
#else
  ptr = malloc(allocated_size);
#endif
  return ptr;
}

static PSE_FINLINE void*
pseStdAllocArrayAligned(void* self, size_t elem_memsize, size_t elem_memalign, size_t count)
{
  if( elem_memalign == 0 ) {
    return pseStdAllocArray(self, elem_memsize, count);
  }
  {
    void* ptr = NULL;
    int res;
    size_t allocated_size = 0;
    size_t elem_adjusted_memsize;

    (void)self, (void)res;
    if(  (elem_memsize <= 0)
      || (elem_memalign > 256)
      || (count <= 0) )
      return NULL;

    elem_memalign = pseAllocationAlignementAdjust(elem_memalign);
    elem_adjusted_memsize = pseAllocationArrayElemSizeAdjust
      (elem_memsize, elem_memalign);
    allocated_size = elem_adjusted_memsize * count;
#if defined(PSE_OS_WINDOWS)
    allocated_size += PSE_ALLOC_HEADER_MEMSIZE;
    ptr = _aligned_offset_malloc
      (allocated_size, elem_memalign, PSE_ALLOC_HEADER_MEMSIZE);
    if( ptr ) {
      /* TODO: check errno */
      ptr = (void*)((uintptr_t)ptr + PSE_ALLOC_HEADER_MEMSIZE);
      PSE_ALLOC_HEADER_GET(ptr)->size = allocated_size;
      PSE_ALLOC_HEADER_GET(ptr)->alignment = elem_memalign;
    }
#else
    res = posix_memalign(&ptr, elem_memalign, allocated_size);
    assert(res != EINVAL);
    assert(res != ENOMEM || ptr == NULL);
#endif
    return ptr;
  }
}

static PSE_FINLINE void*
pseStdAlloc(void* self, size_t memsize)
{ return pseStdAllocArray(self, memsize, 1); }

static PSE_FINLINE void*
pseStdAllocAligned(void* self, size_t memsize, size_t memalign)
{ return pseStdAllocArrayAligned(self, memsize, memalign, 1); }

static PSE_FINLINE void
pseStdFree(void* self, void* ptr)
{
  (void)self;
#if defined(PSE_OS_WINDOWS)
  if( ptr != NULL ) {
    _aligned_free(PSE_ALLOC_HEADER_GET(ptr));
  }
#else
  free(ptr);
#endif
}

static PSE_FINLINE void*
pseStdRealloc(void* self, void* ptr, size_t newsize)
{
  void* new_ptr = NULL;
  (void)self;

  if( ptr == NULL )
    return pseStdAlloc(self, newsize);
  if( newsize == 0 ) {
    pseStdFree(self, ptr);
    return NULL;
  }

#if defined(PSE_OS_WINDOWS)
  newsize += PSE_ALLOC_HEADER_MEMSIZE;
  ptr = _aligned_offset_realloc
    (ptr, newsize, PSE_ALLOC_HEADER_GET(ptr)->alignment,
     PSE_ALLOC_HEADER_MEMSIZE);
  if( ptr ) {
    /* TODO: check errno */
    ptr = (void*)((uintptr_t)ptr + PSE_ALLOC_HEADER_MEMSIZE);
    PSE_ALLOC_HEADER_GET(ptr)->size = newsize;
  }
#else
  new_ptr = realloc(ptr, newsize);
#endif
  return new_ptr;
}

static PSE_FINLINE size_t
pseStdMemSize(void* self, void* ptr)
{
  (void)self;
  if( !ptr )
    return 0;
#if defined(PSE_OS_UNIX)
  return malloc_usable_size(ptr);
#elif defined(PSE_OS_MACH)
  return malloc_size(ptr);
#elif defined(PSE_OS_WINDOWS)
  return PSE_ALLOC_HEADER_GET(ptr)->size;
#else
#error OS not supported
#endif
}

#define PSE_ALLOCATOR_CSTDLIB_                                                 \
  { NULL,                                                                      \
    pseStdAlloc, pseStdAllocAligned,                                           \
    pseStdAllocArray, pseStdAllocArrayAligned,                                 \
    pseStdRealloc, pseStdFree, pseStdMemSize }

struct pse_allocator_t PSE_ALLOCATOR_CSTDLIB = PSE_ALLOCATOR_CSTDLIB_;

/******************************************************************************
 *
 * Default allocator use STD allocator
 *
 ******************************************************************************/

struct pse_allocator_t PSE_ALLOCATOR_DEFAULT = PSE_ALLOCATOR_CSTDLIB_;
