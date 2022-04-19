#ifndef PSE_TYPES_H
#define PSE_TYPES_H

#include "pse_platform.h"

#include <inttypes.h>

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

typedef uintptr_t pse_clt_type_uid_t;
typedef uintptr_t pse_clt_pspace_uid_t;
typedef uintptr_t pse_clt_ppoint_variation_uid_t;
typedef uintptr_t pse_clt_cost_func_uid_t;
typedef uintptr_t pse_clt_relshps_group_uid_t;
typedef void* pse_clt_cost_func_config_t;
typedef void* pse_clt_cost_func_ctxt_t;
typedef void* pse_clt_cost_func_ctxt_config_t;
typedef void* pse_clt_cost_func_ctxt_init_params_t;

typedef uintptr_t pse_ppoint_id_t;
typedef uintptr_t pse_relshp_id_t;
typedef uintptr_t pse_relshp_cost_func_id_t;

/* This macro can be used to generate code for each type, using a user
 * definition of this macro:
 *    #define PSE_TYPE(name, size_in_bits, ctype)
 */
#define PSE_ALL_TYPES                                                          \
  PSE_TYPE(BOOL, 8, bool)       /*!< boolean stored in 1 byte */               \
  PSE_TYPE(INT,  8, int8_t)     /*!< integer stored in 1 byte */               \
  PSE_TYPE(INT, 16, int16_t)    /*!< integer stored in 2 bytes */              \
  PSE_TYPE(INT, 32, int32_t)    /*!< integer stored in 4 bytes */              \
  PSE_TYPE(INT, 64, int64_t)    /*!< integer stored in 8 bytes */              \
  PSE_TYPE(UINT,  8, uint8_t)   /*!< unsigned integer stored in 1 byte */      \
  PSE_TYPE(UINT, 16, uint16_t)  /*!< unsigned integer stored in 2 bytes */     \
  PSE_TYPE(UINT, 32, uint32_t)  /*!< unsigned integer stored in 4 bytes */     \
  PSE_TYPE(UINT, 64, uint64_t)  /*!< unsigned integer stored in 8 bytes */     \
  PSE_TYPE(IEEE_754_FLOAT, 32, float)  /*!< IEEE 754 float stored in 4 bytes */\
  PSE_TYPE(IEEE_754_FLOAT, 64, double) /*!< IEEE 754 float stored in 8 bytes */

/*! Format: PSE_TYPE_[standard]_(type)_(bits_count)
 * \todo Manage endianess */
enum pse_type_t {
  PSE_TYPE_FIRST__,
  PSE_TYPE_NONE = PSE_TYPE_FIRST__, /*!< no type defined */

# define PSE_TYPE(name,sib,ctype)                                              \
    PSE_TYPE_## name ##_## sib,
  PSE_ALL_TYPES
# undef PSE_TYPE

  PSE_TYPE_COUNT__, /*!< Number of types managed by this enum */

  PSE_TYPE_FLOAT = PSE_TYPE_IEEE_754_FLOAT_32,  /*!< default float use IEEE 754 */
  PSE_TYPE_DOUBLE = PSE_TYPE_IEEE_754_FLOAT_64, /*!< default double use IEEE 754 */
#if defined(PSE_USE_FLOAT_FOR_REAL)
  PSE_TYPE_REAL = PSE_TYPE_FLOAT  /*!< real type is float stored in 4 bytes */
#else
  PSE_TYPE_REAL = PSE_TYPE_DOUBLE /*!< real type is double stored in 8 bytes */
#endif
};

typedef enum pse_res_t
(*pse_clt_type_meminit_cb)
  (void* user_ctxt,
   const pse_clt_type_uid_t type_id,
   const size_t count,
   void** mems_to_init);

typedef enum pse_res_t
(*pse_clt_type_memclean_cb)
  (void* user_ctxt,
   const pse_clt_type_uid_t type_id,
   const size_t count,
   void** mems_to_clean);

struct pse_clt_type_info_t {
  pse_clt_type_uid_t type_id;
  size_t memsize;
  size_t memalign;
  const void* memdefault;
  pse_clt_type_meminit_cb meminit;
  pse_clt_type_memclean_cb memclean;
  void* user_ctxt; /*!< given as a parameter to meminit and memclean */
};

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_CLT_TYPE_UID_INVALID_                                              \
  ((pse_clt_type_uid_t)-1)
#define PSE_CLT_PSPACE_UID_INVALID_                                            \
  ((pse_clt_pspace_uid_t)-1)
#define PSE_CLT_PPOINT_VARIATION_UID_INVALID_                                  \
  ((pse_clt_ppoint_variation_uid_t)-1)
#define PSE_CLT_COST_FUNC_UID_INVALID_                                         \
  ((pse_clt_cost_func_uid_t)-1)
#define PSE_CLT_RELSHPS_GROUP_UID_INVALID_                                     \
  ((pse_clt_relshps_group_uid_t)-1)
#define PSE_CLT_COST_FUNC_CONFIG_NULL_                                         \
  ((pse_clt_cost_func_config_t)NULL)
#define PSE_CLT_COST_FUNC_CTXT_NULL_                                           \
  ((pse_clt_cost_func_ctxt_t)NULL)
#define PSE_CLT_COST_FUNC_CTXT_CONFIG_NULL_                                    \
  ((pse_clt_cost_func_ctxt_config_t)NULL)
#define PSE_CLT_COST_FUNC_CTXT_INIT_PARAMS_NULL_                               \
  ((pse_clt_cost_func_ctxt_init_params_t)NULL)
#define PSE_PPOINT_ID_INVALID_                                                 \
  ((pse_ppoint_id_t)-1)
#define PSE_RELSHP_ID_INVALID_                                                 \
  ((pse_relshp_id_t)-1)
#define PSE_RELSHP_COST_FUNC_ID_INVALID_                                       \
  ((pse_relshp_cost_func_id_t)-1)
#define PSE_CLT_TYPE_INFO_NULL_                                                \
  { PSE_CLT_TYPE_UID_INVALID_, 0, 0, NULL, NULL, NULL, NULL }

static const pse_clt_type_uid_t PSE_CLT_TYPE_UID_INVALID =
  PSE_CLT_TYPE_UID_INVALID_;
static const pse_clt_ppoint_variation_uid_t PSE_CLT_PPOINT_VARIATION_UID_INVALID =
  PSE_CLT_PPOINT_VARIATION_UID_INVALID_;
static const pse_clt_pspace_uid_t PSE_CLT_PSPACE_UID_INVALID =
  PSE_CLT_PSPACE_UID_INVALID_;
static const pse_clt_cost_func_uid_t PSE_CLT_COST_FUNC_UID_INVALID =
  PSE_CLT_COST_FUNC_UID_INVALID_;
static const pse_clt_relshps_group_uid_t PSE_CLT_RELSHPS_GROUP_UID_INVALID =
  PSE_CLT_RELSHPS_GROUP_UID_INVALID_;
static const pse_clt_cost_func_config_t PSE_CLT_COST_FUNC_CONFIG_NULL =
  PSE_CLT_COST_FUNC_CONFIG_NULL_;
static const pse_clt_cost_func_ctxt_t PSE_CLT_COST_FUNC_CTXT_NULL =
  PSE_CLT_COST_FUNC_CTXT_NULL_;
static const pse_clt_cost_func_ctxt_t PSE_CLT_COST_FUNC_CTXT_CONFIG_NULL =
  PSE_CLT_COST_FUNC_CTXT_CONFIG_NULL_;
static const pse_clt_cost_func_ctxt_init_params_t PSE_CLT_COST_FUNC_CTXT_INIT_PARAMS_NULL =
PSE_CLT_COST_FUNC_CTXT_INIT_PARAMS_NULL_;
static const pse_ppoint_id_t PSE_PPOINT_ID_INVALID =
  PSE_PPOINT_ID_INVALID_;
static const pse_relshp_id_t PSE_RELSHP_ID_INVALID =
  PSE_RELSHP_ID_INVALID_;
static const pse_relshp_cost_func_id_t PSE_RELSHP_COST_FUNC_ID_INVALID =
  PSE_RELSHP_COST_FUNC_ID_INVALID_;
static const struct pse_clt_type_info_t PSE_CLT_TYPE_INFO_NULL =
  PSE_CLT_TYPE_INFO_NULL_;

PSE_API_END

#endif /* PSE_TYPES_H */
