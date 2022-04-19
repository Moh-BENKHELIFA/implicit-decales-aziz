#ifndef PSE_DRV_H
#define PSE_DRV_H

#include "pse.h"
#include "pse_capacities.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

typedef void* pse_lib_handle_t;
typedef void* pse_drv_handle_t;

typedef uintptr_t pse_drv_exploration_id_t;

struct pse_drv_params_t {
  struct pse_allocator_t* allocator;
  struct pse_logger_t* logger;
};

struct pse_cpspace_instance_pspace_data_t {
  pse_clt_pspace_uid_t key;
  size_t ppoint_attribs_comps_count[PSE_POINT_ATTRIB_COUNT_];
};

struct pse_cpspace_instance_variated_cost_func_data_t {
  pse_clt_ppoint_variation_uid_t uid;
  size_t relshps_count;
  pse_relshp_id_t* relshps_ids;
  const struct pse_eval_relshp_data_t** relshps_data;
  pse_clt_cost_func_ctxt_t* relshps_ctxts;
  pse_clt_cost_func_ctxt_config_t* relshps_configs;
};

struct pse_cpspace_instance_cost_func_data_t {
  pse_relshp_cost_func_id_t key;
  struct pse_relshp_cost_func_params_t params;
  size_t variations_count;
  struct pse_cpspace_instance_variated_cost_func_data_t* variations;
};

struct pse_cpspace_instance_relshp_data_t {
  pse_relshp_id_t key;
  struct pse_eval_relshp_data_t eval_data;
};

struct pse_cpspace_instance_t {
  size_t pspaces_count;
  struct pse_cpspace_instance_pspace_data_t* pspaces;

  size_t cfuncs_count;
  struct pse_cpspace_instance_cost_func_data_t* cfuncs;

  size_t ppoints_count;
  pse_ppoint_id_t* ppoints;

  size_t relshps_count;
  struct pse_cpspace_instance_relshp_data_t* relshps;

  struct pse_cpspace_t* cps;  /*!< related CPS */
};

/*! Stores a PSE driver API. This is used to load/unload drivers dynamically. */
struct pse_drv_t {

  /*! Called before unloading the driver */
  enum pse_res_t
  (*clean)
    (pse_drv_handle_t self);

  /*! Return RES_OK if the given \p cap capacity with type \p type is managed by
   * the driver, or else return RES_NOT_SUPPORTED. */
  enum pse_res_t
  (*is_capacity_managed)
    (pse_drv_handle_t self,
     enum pse_device_capacity_t cap,
     enum pse_type_t type); /*!< May be NONE if capacity is not typed */

  enum pse_res_t
  (*cpspace_exploration_prepare)
    (pse_drv_handle_t self,
     struct pse_cpspace_exploration_ctxt_t* exp_ctxt,
     struct pse_cpspace_instance_t* icps,
     const struct pse_cpspace_exploration_ctxt_params_t* params,
     pse_drv_exploration_id_t* exp);

  enum pse_res_t
  (*exploration_clean)
    (pse_drv_handle_t self,
     pse_drv_exploration_id_t exp);

  enum pse_res_t
  (*exploration_solve)
    (pse_drv_handle_t self,
     pse_drv_exploration_id_t exp,
     struct pse_cpspace_values_data_t* smpls);

  enum pse_res_t
  (*exploration_solve_iterative_begin)
    (pse_drv_handle_t self,
     pse_drv_exploration_id_t exp,
     struct pse_cpspace_values_data_t* smpls);

  enum pse_res_t
  (*exploration_solve_iterative_step)
    (pse_drv_handle_t self,
     pse_drv_exploration_id_t exp);

  enum pse_res_t
  (*exploration_solve_iterative_end)
    (pse_drv_handle_t self,
     pse_drv_exploration_id_t exp);

  enum pse_res_t
  (*exploration_last_results_retreive)
    (pse_drv_handle_t self,
     pse_drv_exploration_id_t exp,
     struct pse_cpspace_values_data_t* where,
     struct pse_cpspace_exploration_extra_results_t* extra);

  pse_drv_handle_t self;
  pse_lib_handle_t lib;
};

/*! Prototype of the entrypoint of a PSE driver. This function **MUST** fill the
 * \param drv parameters with the API functions.
 */
typedef enum pse_res_t
(* pse_drv_entrypoint_cb)
  (struct pse_device_t* dev,
   struct pse_drv_params_t* params,
   struct pse_drv_t* drv);

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_DRV_ENTRYPOINT_SYMBOL                                              \
  pseDriverEntryPoint
#define PSE_LIB_HANDLE_INVALID_                                                \
  NULL
#define PSE_DRV_HANDLE_INVALID_                                                \
  NULL
#define PSE_DRV_EXPLORATION_ID_INVALID_                                        \
  ((pse_drv_exploration_id_t)-1)
#define PSE_DRV_PARAMS_NULL_                                                   \
  { NULL, NULL }
#define PSE_CPSPACE_INSTANCE_PSPACE_DATA_NULL_                                 \
  { PSE_CLT_PSPACE_UID_INVALID_, { 0, 0 } }
#define PSE_CPSPACE_INSTANCE_VARIATED_COST_FUNC_DATA_NULL_                     \
  { PSE_CLT_PPOINT_VARIATION_UID_INVALID_, 0, NULL, NULL, NULL, NULL }
#define PSE_CPSPACE_INSTANCE_COST_FUNC_DATA_NULL_                              \
  { PSE_RELSHP_COST_FUNC_ID_INVALID_, PSE_RELSHP_COST_FUNC_PARAMS_NULL_,       \
    0, NULL }
#define PSE_CPSPACE_INSTANCE_RELSHP_DATA_NULL_                                 \
  { PSE_RELSHP_ID_INVALID_, PSE_EVAL_RELSHP_DATA_NULL_ }
#define PSE_CPSPACE_INSTANCE_NULL_                                             \
  { 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL }
#define PSE_DRV_NULL_                                                          \
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,                      \
    PSE_DRV_HANDLE_INVALID_, PSE_LIB_HANDLE_INVALID_ }

static const pse_lib_handle_t PSE_LIB_HANDLE_INVALID =
  PSE_LIB_HANDLE_INVALID_;
static const pse_drv_handle_t PSE_DRV_HANDLE_INVALID =
  PSE_DRV_HANDLE_INVALID_;
static const pse_drv_exploration_id_t PSE_DRV_EXPLORATION_ID_INVALID =
  PSE_DRV_EXPLORATION_ID_INVALID_;
static const struct pse_drv_params_t PSE_DRV_PARAMS_NULL =
  PSE_DRV_PARAMS_NULL_;
static const struct pse_cpspace_instance_pspace_data_t PSE_CPSPACE_INSTANCE_PSPACE_DATA_NULL =
  PSE_CPSPACE_INSTANCE_PSPACE_DATA_NULL_;
static const struct pse_cpspace_instance_variated_cost_func_data_t PSE_CPSPACE_INSTANCE_VARIATED_COST_FUNC_DATA_NULL =
  PSE_CPSPACE_INSTANCE_VARIATED_COST_FUNC_DATA_NULL_;
static const struct pse_cpspace_instance_cost_func_data_t PSE_CPSPACE_INSTANCE_COST_FUNC_DATA_NULL =
  PSE_CPSPACE_INSTANCE_COST_FUNC_DATA_NULL_;
static const struct pse_cpspace_instance_relshp_data_t PSE_CPSPACE_INSTANCE_RELSHP_DATA_NULL =
  PSE_CPSPACE_INSTANCE_RELSHP_DATA_NULL_;
static const struct pse_cpspace_instance_t PSE_CPSPACE_INSTANCE_NULL =
  PSE_CPSPACE_INSTANCE_NULL_;
static const struct pse_drv_t PSE_DRV_NULL =
  PSE_DRV_NULL_;

/******************************************************************************
 *
 * PUBLIC API Driver loading/unloading
 *
 ******************************************************************************/

PSE_API enum pse_res_t
pseDriverLoad
  (struct pse_device_t* dev,
   const char* filepath,
   struct pse_drv_t* drv);

PSE_API enum pse_res_t
pseDriverUnload
  (struct pse_drv_t* drv);

PSE_API_END

#endif /* PSE_DRV_H */
