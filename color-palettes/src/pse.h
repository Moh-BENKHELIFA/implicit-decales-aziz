#ifndef PSE_H
#define PSE_H

#include "pse_api.h"
#include "pse_allocator.h"
#include "pse_capacities.h"
#include "pse_logger.h"
#include "pse_types.h"

#include <stddef.h>
#include <inttypes.h>

/*! \file
 * Header file for the Parametric Space Exploration (PSE) library C API.
 * This file provide the full C89 API to use the PSE library. Here is a the
 * specific nomenclature used in this file, allowing shorter names:
 *   - \p pse: parametric space exploration (library acronym);
 *   - \p ppoint: parametric point;
 *   - \p pspace: parametric space;
 *   - \p cpspace: constrained parameter space;
 *   - \p relshp: relationship;
 *   - \p cnstr: constraint;
 *   - \p func: functor;
 *   - \p ctxt: context;
 * \todo Ensure everything is thread-safe and can be called concurrently without
 * any problems. Mainly check the values lock/unlock mechanism to allow fast and
 * possibly lock-free read-write access from user and driver at the same time.
 */

PSE_API_BEGIN

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_device_t;
struct pse_cpspace_t;
struct pse_cpspace_values_t;
struct pse_cpspace_exploration_ctxt_t;

/******************************************************************************
 * 
 * PUBLIC TYPES
 *
 ******************************************************************************/

/*! Says how are managed the parametric points of the relationships:
 *   - INCLUSIVE: all given parametric points are involved in the same
 *      relationship;
 *   - EXCLUSIVE: all given parametric points are **NOT** involved in the
 *      relationship, i.e. all parametric points that are not given are involved
 *      in the same relationship. This relationship is dynamic, meaning that if
 *      you add new parametric points later, they will be included in such
 *      relationship;
 *
 * \note Using EXCLUSIVE with \p ppoints_count at 0 means you create a
 *   relationship between all parametric points of the constrained parameter
 *   space.
 */
enum pse_relshp_kind_t {
  PSE_RELSHP_KIND_INCLUSIVE,
  PSE_RELSHP_KIND_EXCLUSIVE
};

/*! Parameters used for the creation of a device.
 * \param allocator The memory allocator that the device will use. This
 *    allocator will also be given to the driver. May be NULL to let the device
 *    use the default allocator stored in PSE_ALLOCATOR_DEFAULT.
 * \param logger The logger that will be used to provide feedback. May be NULL
 *    to disable logging.
 * \param backend_drv_filepath The path to the dynamic library of the driver to
 *    use. This parameter must be provided. The macro ::PSE_LIB_NAME may be used
 *    to simplify the portability.
 */
struct pse_device_params_t {
  struct pse_allocator_t* allocator; /*! NULL => use internal allocator */
  struct pse_logger_t* logger; /*! NULL => force no log */
  const char* backend_drv_filepath;
};

enum pse_point_attrib_t {
  PSE_POINT_ATTRIB_COORDINATES, /*!< position in the parameter space */
  PSE_POINT_ATTRIB_LOCK_STATUS,  /*!< if point is locked during optimization */

  PSE_POINT_ATTRIB_COUNT_,  /*!< number of attributes managed for a point */

  PSE_POINT_ATTRIB_INVALID
};

struct pse_pspace_point_attrib_component_t {
  enum pse_type_t type;
};

struct pse_pspace_point_attrib_t {
  size_t components_count;
  const struct pse_pspace_point_attrib_component_t* components;
};

struct pse_pspace_point_params_t {
  struct pse_pspace_point_attrib_t attribs[PSE_POINT_ATTRIB_COUNT_];
};

/*! Parameters of parameter space.
 * \param ppoint_params Parameters of a parametric point of the parameter space.
 *    See ::pse_pspace_point_params_t for more information.
 * \param variations_count The number of variations appliable on this parameter
 *    space.
 * \param variations The array of variations appliable on this parameter space.
 * \note When no variation is given, do not forget that the parameter space
 *    exists on its own, and thus will be usable anyway, without any variation.
 */
struct pse_pspace_params_t {
  struct pse_pspace_point_params_t ppoint_params;
  size_t variations_count;
  pse_clt_ppoint_variation_uid_t* variations;
};

typedef enum pse_res_t
(*pse_attrib_value_getter_cb)
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   void* attrib_values);

typedef enum pse_res_t
(*pse_attrib_value_setter_cb)
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   const void* attrib_values);

struct pse_attrib_value_accessors_t {
  pse_attrib_value_getter_cb get;
  pse_attrib_value_setter_cb set;
  void* ctxt;
};

struct pse_cpspace_values_data_accessors_global_t {
  struct pse_attrib_value_accessors_t accessors;
};

struct pse_cpspace_values_data_accessors_per_attrib_t {
  struct pse_attrib_value_accessors_t accessors[PSE_POINT_ATTRIB_COUNT_];
};

enum pse_cpspace_values_storage_t {
  PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_GLOBAL,
  PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_PER_ATTRIB
};

struct pse_cpspace_values_data_t {
  pse_clt_pspace_uid_t pspace;
  enum pse_cpspace_values_storage_t storage;
  union {
    struct pse_cpspace_values_data_accessors_global_t global;
    struct pse_cpspace_values_data_accessors_per_attrib_t per_attrib;
  } as;
};

enum pse_lock_mode_t {
  PSE_LOCK_READ,
  PSE_LOCK_WRITE,
  PSE_LOCK_READ_WRITE
};

struct pse_cpspace_values_lock_params_t {
  enum pse_lock_mode_t mode;
};

struct pse_eval_ctxt_t {
  struct pse_device_t* dev;
  struct pse_cpspace_t* cps;
  struct pse_cpspace_exploration_ctxt_t* exp_ctxt;
};

struct pse_eval_coordinates_t {
  pse_clt_pspace_uid_t pspace_uid;
  size_t scalars_count;
  const pse_real_t* coords;
};

/*! Contains the data related to a relationship during a cost functor
 * evaluation.
 *
 * \param ppoints_count The number of parametric points involved in the
 *   relationship.
 * \param ppoints The array of the parametric points identifiers involved in the
 *   relationship.
 */
struct pse_eval_relshp_data_t {
  size_t ppoints_count;
  pse_ppoint_id_t* ppoints;
};

struct pse_eval_relshps_t {
   size_t count;
   const pse_relshp_id_t* ids;
   const struct pse_eval_relshp_data_t* const* data;
   pse_clt_cost_func_ctxt_t const* ctxts;
   pse_clt_cost_func_ctxt_config_t const* configs;
};

/*! Callback for the initialization of the contexts of cost functors evaluation,
 * associated to relationships.
 *
 * \param[in] user_cfunc_config The specific user cost functor configuration,
 *    global to all contexts that may be initialized.
 * \param[in] user_cfunc_uid The user UID of the cost functor.
 * \param[in] eval_ctxt The evaluation context. See ::pse_eval_ctxt_t for more
 *    information.
 * \param[in,out] eval_relshps The relationships for which this function will
 *    have to initialize the contexts. This function have to initialize the
 *    ::pse_eval_relshps_t::ctxts elements.
 * \param[in] user_init_params The specific user parameters that may be used to
 *    initialize the relationships context.
 */
typedef enum pse_res_t
(*pse_clt_relshps_cost_func_ctxts_init_cb)
  (pse_clt_cost_func_config_t user_cfunc_config,
   const pse_clt_cost_func_uid_t user_cfunc_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps,
   pse_clt_cost_func_ctxt_init_params_t user_init_params);

/*! Callback for the cleaning of the contexts of cost functors evaluation,
 * associated to relationships.
 *
 * \param[in] user_cfunc_config The specific user cost functor configuration,
 *    global to all contexts that may be initialized.
 * \param[in] user_cfunc_uid The user UID of the cost functor.
 * \param[in] eval_ctxt The evaluation context. See ::pse_eval_ctxt_t for more
 *    information.
 * \param[in,out] eval_relshps The relationships for which this function will
 *    have to clean the contexts for later user. This function have to clean the
 *    ::pse_eval_relshps_t::ctxts elements.
 */
typedef enum pse_res_t
(*pse_clt_relshps_cost_func_ctxts_clean_cb)
  (pse_clt_cost_func_config_t func_user_data,
   const pse_clt_cost_func_uid_t func_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps);

/*! Callback for the computation of the costs of given relationships. This
 * function is called in the user space.
 *
 * \param[in] eval_ctxt The evaluation context. See \ref pse_eval_ctxt_t for
 *    more information on the date it contains.
 * \param[in] eval_coords The coordinates of all parametric points that may be
 *    used by the computation.
 * \param[in] eval_relshps The relationships information that may be used by
 *    the computation. See \ref pse_eval_relshps_t for more information.
 * \param[out] costs The costs of each relationship that the function must fill.
 *    This array is of size \p ::pse_eval_relshps_t::count times the number of
 *    costs the function must return per relationship. See
 *    ::pse_relshp_cost_func_params_t::costs_count for more details.
 */
typedef enum pse_res_t
(*pse_clt_relshp_cost_func_compute_cb)
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs);

/*! Allow to say how many costs are needed for a cost functor.
 * \c PSE_COST_ARITY_MODE_PER_RELATIONSHIP We need at least one cost for the
 *    relationship, whatever the number of points involved.
 * \c PSE_COST_ARITY_MODE_PER_POINT We need at least one cost per point
 *    involved in the relationship.
 * \note The real number of costs is given by the member \p cost_count of the
 * ::pse_relshp_cost_func_params_t structure.
 */
enum pse_cost_arity_mode_t {
  PSE_COST_ARITY_MODE_PER_RELATIONSHIP,
  PSE_COST_ARITY_MODE_PER_POINT
};

/*! Parameters of a relationship cost functor. */
struct pse_relshp_cost_func_params_t {
  pse_clt_cost_func_uid_t uid;
  pse_clt_pspace_uid_t expected_pspace;
  pse_clt_relshp_cost_func_compute_cb compute;
  pse_clt_relshp_cost_func_compute_cb compute_df; /* TODO */
  enum pse_cost_arity_mode_t cost_arity_mode;
  size_t costs_count;

  struct pse_clt_type_info_t ctxt_type_info;
  pse_clt_relshps_cost_func_ctxts_init_cb ctxts_init;
  pse_clt_relshps_cost_func_ctxts_clean_cb ctxts_clean;
  pse_clt_cost_func_config_t user_config;
};

/*! Constraints applied to a relationship, taking the form of a list cost
 * functors. The constraint force the minimization the evaluated costs.
 * \param funcs_count The number of cost functor making the constraints of the
 *    relationship.
 * \param funcs The array of cost functors making the constraints of the
 *    relationship.
 * \param ctxts_config Configurations of the contexts used during the evaluation
 *    of each cost functor making the constraints of the relationship. The
 *    pointers will be given through the ::pse_eval_relshps_t structure. This
 *    parameter may be NULL if there is no configuration at all, or a specific
 *    value may be NULL if a specific cost functor do not use a configuration.
 */
struct pse_cpspace_relshp_cnstrs_t {
  size_t funcs_count;
  pse_relshp_cost_func_id_t* funcs;
  pse_clt_cost_func_ctxt_config_t* ctxts_config;
};

struct pse_cpspace_params_t {
  int unused;
};

struct pse_ppoint_params_t {
  int unused;
};

/*! Parameters of a relationship.
 * \param kind The kind of the relationship.
 * \param ppoints_count The number of parametric points taken into account.
 * \param ppoints_id The array of the parametric points that will be taken into
 *    account. See \ref pse_relshp_kind_t to check how the parametric points
 *    will be taken into account depending on the \p kind. Each parametric point
 *    id must appear at most once!
 * \param variations_count The number of additionnal variations in which this
 *    relationship must be taken into account.
 * \param variations The array of the additionnal variations in which this
 *    relationship must be taken into account. For each parameter space used by
 *    this relationship, we will go through all variations and applied them if
 *    possible. The list of parameter spaces used by this relationship is known
 *    by listing all the parameter spaces expected by each cost functors stored
 *    in \p cnstrs. Then for each parameter space of this list, we do the
 *    intersection between its list of appliable variations
 *    (see ::pse_pspace_params_t::variations) and the variations contained in
 *    this member.
 * \param cnstrs The initial constraints associated with this relationship.
 */
struct pse_cpspace_relshp_params_t {
  enum pse_relshp_kind_t kind;
  size_t ppoints_count;
  pse_ppoint_id_t* ppoints_id;
  size_t variations_count;
  pse_clt_ppoint_variation_uid_t* variations;
  struct pse_cpspace_relshp_cnstrs_t cnstrs;
};

enum pse_cpspace_relshp_state_t {
  PSE_CPSPACE_RELSHP_STATE_DISABLED,
  PSE_CPSPACE_RELSHP_STATE_ENABLED
};

enum pse_cpspace_relshps_group_state_t {
  PSE_CPSPACE_RELSHPS_GROUP_STATE_DISABLED,
  PSE_CPSPACE_RELSHPS_GROUP_STATE_ENABLED,
  PSE_CPSPACE_RELSHPS_GROUP_STATE_ENABLED_PARTIALLY
};

/*! Options to do extra computation during the exploration.
 *
 * \param until_convergence For monolithic solve, will iterate until
 *    success/error or \p max_convergence_tries is reached. Not taken into
 *    account for iterative solve as the user can do it him-self.
 * \param max_convergence_tries Number of tries to converge. After that, we stop
 *    with RES_NOT_CONVERGED result.
 * \param auto_df_epsilon When using the automatic differential computation, we
 *    will use this value as the epsilon to have delta coordinates before and
 *    after the current coordinates of each parametric point.
 */
struct pse_cpspace_exploration_options_t {
  bool until_convergence;
  size_t max_convergence_tries;
  pse_real_t auto_df_epsilon;
};

typedef enum pse_res_t
(*pse_pspace_convert_cb)
  (void* user_data,
   const pse_clt_pspace_uid_t from,
   const pse_clt_pspace_uid_t to,
   const size_t ppoints_count,
   const pse_real_t* values_from,
   pse_real_t* values_to);

struct pse_cpspace_exploration_pspace_params_t {
  pse_clt_pspace_uid_t explore_in;
  pse_pspace_convert_cb convert;
  void* convert_user_data;
};

typedef enum pse_res_t
(*pse_ppoint_variation_apply_cb)
  (void* user_data,
   const pse_clt_pspace_uid_t in,
   const pse_clt_ppoint_variation_uid_t to,
   const size_t ppoints_count,
   const pse_real_t* values_from,
   pse_real_t* values_to);

struct pse_cpspace_exploration_variations_params_t {
  size_t count;
  pse_clt_ppoint_variation_uid_t* to_explore;
  pse_ppoint_variation_apply_cb apply;
  void* apply_user_data;
};

/*! Parameters of an exploration context.
 *
 * \param explore_in
 * \param convert
 * \param convert_user_data
 * \param options
 */
struct pse_cpspace_exploration_ctxt_params_t {
  struct pse_cpspace_exploration_pspace_params_t pspace;
  struct pse_cpspace_exploration_variations_params_t variations;
  struct pse_cpspace_exploration_options_t options;
};

struct pse_cpspace_exploration_samples_t {
  struct pse_cpspace_values_t* values;
};

/*! \brief Counter that gives its value for the last call to the solver and the
 *! value since the creation of the context. */
struct pse_counter_t {
  size_t last_call;
  size_t total;
};

struct pse_cpspace_exploration_extra_results_t {
  struct pse_counter_t counter_iterations;
  struct pse_counter_t counter_costs_calls;
};

/******************************************************************************
 * 
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_EVAL_CTXT_NULL_                                                    \
  { NULL, NULL, NULL }
#define PSE_EVAL_COORDINATES_NULL_                                             \
  { PSE_CLT_PSPACE_UID_INVALID_, 0, NULL }
#define PSE_EVAL_RELSHPS_NULL_                                                 \
  { 0, NULL, NULL, NULL, NULL }
#define PSE_EVAL_RELSHP_DATA_NULL_                                             \
  { 0, NULL }
#define PSE_COUNTER_ZERO_                                                      \
  { 0, 0 }
#define PSE_DEVICE_PARAMS_NULL_                                                \
  { NULL, NULL, NULL }
#define PSE_PSPACE_POINT_ATTRIB_COMPONENT_NULL_                                \
  { PSE_TYPE_NONE }
#define PSE_PSPACE_POINT_ATTRIB_NULL_                                          \
  { 0, NULL }
#define PSE_PSPACE_POINT_PARAMS_NULL_                                          \
  { { PSE_PSPACE_POINT_ATTRIB_NULL_, PSE_PSPACE_POINT_ATTRIB_NULL_ } }
#define PSE_PSPACE_PARAMS_NULL_                                                \
  { PSE_PSPACE_POINT_PARAMS_NULL_, 0, NULL }
#define PSE_ATTRIB_VALUE_ACCESSORS_NULL_                                       \
  { NULL, NULL, NULL }
#define PSE_CPSPACE_VALUES_DATA_ACCESSORS_GLOBAL_NULL_                         \
  { PSE_ATTRIB_VALUE_ACCESSORS_NULL_ }
#define PSE_CPSPACE_VALUES_DATA_ACCESSORS_PER_ATTRIB_NULL_                     \
  { { PSE_ATTRIB_VALUE_ACCESSORS_NULL_, PSE_ATTRIB_VALUE_ACCESSORS_NULL_ } }
#define PSE_CPSPACE_VALUES_DATA_NULL_                                          \
  { PSE_CLT_PSPACE_UID_INVALID_, PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_GLOBAL,  \
    { PSE_CPSPACE_VALUES_DATA_ACCESSORS_GLOBAL_NULL_ } }
#define PSE_CPSPACE_LOCK_PARAMS_READ_                                          \
  { PSE_LOCK_READ }
#define PSE_CPSPACE_LOCK_PARAMS_WRITE_                                         \
  { PSE_LOCK_WRITE }
#define PSE_CPSPACE_LOCK_PARAMS_READ_WRITE_                                    \
  { PSE_LOCK_READ_WRITE }
#define PSE_RELSHP_COST_FUNC_PARAMS_NULL_                                      \
  { PSE_CLT_COST_FUNC_UID_INVALID_, PSE_CLT_PSPACE_UID_INVALID_, NULL, NULL,   \
    PSE_COST_ARITY_MODE_PER_RELATIONSHIP, 0,                                   \
    PSE_CLT_TYPE_INFO_NULL_, NULL, NULL, NULL }
#define PSE_PPOINT_PARAMS_NULL_                                                \
  { 0 }
#define PSE_CPSPACE_PARAMS_NULL_                                               \
  { 0 }
#define PSE_CPSPACE_RELSHP_PARAMS_NULL_                                        \
  { PSE_RELSHP_KIND_INCLUSIVE, 0, NULL, 0, NULL,                               \
    PSE_CPSPACE_RELSHP_CNSTRS_NONE_ }
#define PSE_CPSPACE_RELSHP_CNSTRS_NONE_                                        \
  { 0, NULL, NULL }
#define PSE_CPSPACE_EXPLORATION_OPTIONS_DEFAULT_                               \
  { true, 3, PSE_REAL_SAFE_EPS }
#define PSE_CPSPACE_EXPLORATION_PSPACE_PARAMS_NULL_                            \
  { PSE_CLT_PSPACE_UID_INVALID_, NULL, NULL }
#define PSE_CPSPACE_EXPLORATION_VARIATIONS_PARAMS_NULL_                        \
  { 0, NULL, NULL, NULL }
#define PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL_                              \
  { PSE_CPSPACE_EXPLORATION_PSPACE_PARAMS_NULL_,                               \
    PSE_CPSPACE_EXPLORATION_VARIATIONS_PARAMS_NULL_,                           \
    PSE_CPSPACE_EXPLORATION_OPTIONS_DEFAULT_ }
#define PSE_CPSPACE_EXPLORATION_SAMPLES_NULL_                                  \
  { NULL }
#define PSE_CPSPACE_EXPLORATION_EXTRA_RESULTS_NULL_                            \
  { PSE_COUNTER_ZERO_, PSE_COUNTER_ZERO_ }

static const struct pse_eval_ctxt_t PSE_EVAL_CTXT_NULL =
  PSE_EVAL_CTXT_NULL_;
static const struct pse_eval_coordinates_t PSE_EVAL_COORDINATES_NULL =
  PSE_EVAL_COORDINATES_NULL_;
static const struct pse_eval_relshps_t PSE_EVAL_RELSHPS_NULL =
  PSE_EVAL_RELSHPS_NULL_;
static const struct pse_eval_relshp_data_t PSE_EVAL_RELSHP_DATA_NULL =
  PSE_EVAL_RELSHP_DATA_NULL_;
static const struct pse_counter_t PSE_COUNTER_ZERO =
  PSE_COUNTER_ZERO_;
static const struct pse_device_params_t PSE_DEVICE_PARAMS_NULL =
  PSE_DEVICE_PARAMS_NULL_;
static const struct pse_pspace_point_attrib_component_t PSE_PSPACE_POINT_ATTRIB_COMPONENT_NULL =
  PSE_PSPACE_POINT_ATTRIB_COMPONENT_NULL_;
static const struct pse_pspace_point_attrib_t PSE_PSPACE_POINT_ATTRIB_NULL =
  PSE_PSPACE_POINT_ATTRIB_NULL_;
static const struct pse_pspace_point_params_t PSE_PSPACE_POINT_PARAMS_NULL =
  PSE_PSPACE_POINT_PARAMS_NULL_;
static const struct pse_pspace_params_t PSE_PSPACE_PARAMS_NULL =
  PSE_PSPACE_PARAMS_NULL_;
static const struct pse_attrib_value_accessors_t PSE_ATTRIB_VALUE_ACCESSORS_NULL =
  PSE_ATTRIB_VALUE_ACCESSORS_NULL_;
static const struct pse_cpspace_values_data_accessors_global_t PSE_CPSPACE_VALUES_DATA_ACCESSORS_GLOBAL_NULL =
  PSE_CPSPACE_VALUES_DATA_ACCESSORS_GLOBAL_NULL_;
static const struct pse_cpspace_values_data_accessors_per_attrib_t PSE_CPSPACE_VALUES_DATA_ACCESSORS_PER_ATTRIB_NULL =
  PSE_CPSPACE_VALUES_DATA_ACCESSORS_PER_ATTRIB_NULL_;
static const struct pse_cpspace_values_data_t PSE_CPSPACE_VALUES_DATA_NULL =
  PSE_CPSPACE_VALUES_DATA_NULL_;
static const struct pse_cpspace_values_lock_params_t PSE_CPSPACE_VALUES_LOCK_PARAMS_READ =
  PSE_CPSPACE_LOCK_PARAMS_READ_;
static const struct pse_cpspace_values_lock_params_t PSE_CPSPACE_VALUES_LOCK_PARAMS_WRITE =
  PSE_CPSPACE_LOCK_PARAMS_WRITE_;
static const struct pse_cpspace_values_lock_params_t PSE_CPSPACE_VALUES_LOCK_PARAMS_READ_WRITE =
  PSE_CPSPACE_LOCK_PARAMS_READ_WRITE_;
static const struct pse_relshp_cost_func_params_t PSE_RELSHP_COST_FUNC_PARAMS_NULL =
  PSE_RELSHP_COST_FUNC_PARAMS_NULL_;
static const struct pse_ppoint_params_t PSE_PPOINT_PARAMS_NULL =
  PSE_PPOINT_PARAMS_NULL_;
static const struct pse_cpspace_params_t PSE_CPSPACE_PARAMS_NULL =
  PSE_CPSPACE_PARAMS_NULL_;
static const struct pse_cpspace_relshp_params_t PSE_CPSPACE_RELSHP_PARAMS_NULL =
  PSE_CPSPACE_RELSHP_PARAMS_NULL_;
static const struct pse_cpspace_relshp_cnstrs_t PSE_CPSPACE_RELSHP_CNSTRS_NONE =
  PSE_CPSPACE_RELSHP_CNSTRS_NONE_;
static const struct pse_cpspace_exploration_options_t PSE_CPSPACE_EXPLORATION_OPTIONS_DEFAULT =
  PSE_CPSPACE_EXPLORATION_OPTIONS_DEFAULT_;
static const struct pse_cpspace_exploration_pspace_params_t PSE_CPSPACE_EXPLORATION_PSPACE_PARAMS_NULL =
  PSE_CPSPACE_EXPLORATION_PSPACE_PARAMS_NULL_;
static const struct pse_cpspace_exploration_variations_params_t PSE_CPSPACE_EXPLORATION_VARIATIONS_PARAMS_NULL =
  PSE_CPSPACE_EXPLORATION_VARIATIONS_PARAMS_NULL_;
static const struct pse_cpspace_exploration_ctxt_params_t PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL =
  PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL_;
static const struct pse_cpspace_exploration_samples_t PSE_CPSPACE_EXPLORATION_SAMPLES_NULL =
  PSE_CPSPACE_EXPLORATION_SAMPLES_NULL_;
static const struct pse_cpspace_exploration_extra_results_t PSE_CPSPACE_EXPLORATION_EXTRA_RESULTS_NULL =
  PSE_CPSPACE_EXPLORATION_EXTRA_RESULTS_NULL_;

/******************************************************************************
 * 
 * API Device
 *
 ******************************************************************************/

/*!  Create a new device that will use a specific given driver.
 * \param[in] params Parameters for the creation of the driver. See
 *    ::pse_device_params_t for more information on possible parameters.
 * \param[out] dev On success, the newly created device.
 * \return
 *    - ::RES_OK on success
 *    - ::RES_BAD_ARG if no driver is provided
 *    - ::RES_IO_ERR if the driver cannot be loaded
 */
PSE_API enum pse_res_t
pseDeviceCreate
  (const struct pse_device_params_t* params,
   struct pse_device_t** dev);

/*! Destroy the device and release all the remaining references on objects
 * stored in this device. In the end, all the memory allocated through the API
 * in this device is freed. */
PSE_API enum pse_res_t
pseDeviceDestroy
  (struct pse_device_t* dev);

PSE_API enum pse_res_t
pseDeviceCapacityIsManaged
  (struct pse_device_t* dev,
   enum pse_device_capacity_t cap,
   enum pse_type_t type);

/******************************************************************************
 * 
 * API Constrained Parameter Space
 *
 ******************************************************************************/

PSE_API enum pse_res_t
pseConstrainedParameterSpaceCreate
  (struct pse_device_t* dev,
   const struct pse_cpspace_params_t* params,
   struct pse_cpspace_t** cps);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceRefAdd
  (struct pse_cpspace_t* cps);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceRefSub
  (struct pse_cpspace_t* cps);

/******************************************************************************
 *
 * API Constrained Parameter Space Parameter Space
 *
 ******************************************************************************/

PSE_API enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesDeclare
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids,
   const struct pse_pspace_params_t* params);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesForget
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesVariationsAdd
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids,
   const size_t variations_count,
   const pse_clt_ppoint_variation_uid_t* variations);

/*! Remove existing variations. If some variations are not found, return
 * RES_NOT_FOUND. */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesVariationsRemove
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids,
   const size_t variations_count,
   const pse_clt_ppoint_variation_uid_t* variations);

/*! Remove only existing variations, ignore others silently. */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_clt_pspace_uid_t* uids,
   const size_t variations_count,
   const pse_clt_ppoint_variation_uid_t* variations);

/******************************************************************************
 *
 * API Constrained Parameter Space Parametric Point
 *
 ******************************************************************************/

PSE_API enum pse_res_t
pseConstrainedParameterSpaceParametricPointsAdd
  (struct pse_cpspace_t* cps,
   const size_t count,
   const struct pse_ppoint_params_t* params, /* Of size \p count */
   pse_ppoint_id_t* ids); /* Of size \p count */

/*! Remove some parametric points from the given constrained parametric space.
 * Also removes the relationships in which the parametric points are involved.
 *
 * \param[in] cps The constrained parameter space from which we want to remove
 *   the given parametric points.
 * \param[in] count The number of parametric points to remove.
 * \param[in] ids The array of identifiers of the parametric points we want to
 *   remove. Must be an array of at least \p count elements.
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceParametricPointsRemove
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_ppoint_id_t* ids);

/*! Also removes all the relationships*/
PSE_API enum pse_res_t
pseConstrainedParameterSpaceParametricPointsClear
  (struct pse_cpspace_t* cps);

PSE_API size_t
pseConstrainedParameterSpaceParametricPointsCountGet
  (struct pse_cpspace_t* cps);

/******************************************************************************
 *
 * API Constrained Parameter Space Relationship Cost Functors
 *
 ******************************************************************************/

/*! Register cost functors, used by constrained relationships to compute their
 * cost.
 *
 * \param[in] cps The constrained parameter space to which we register the
 *   functors.
 * \param[in] count The number of functors to register.
 * \param[in] params Array of size \p count, which contains the functors
 *   parameters to register.
 * \param[out] ids The functors identifiers to use through other functions of
 *   the API.
 * \see pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
  (struct pse_cpspace_t* cps,
   const size_t count,
   struct pse_relshp_cost_func_params_t* params,
   pse_relshp_cost_func_id_t* ids);

/*! Unregister functors from the constrained parameter space.
 *
 * \param[in] cps The constrained parameter space from which we unregister the
 *   functors.
 * \param[in] count The number of functors to unregister.
 * \param[in] ids Array of size \p count, which contains the functors
 *   identifiers to unregister.
 * \see pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipCostFunctorsUnregister
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_cost_func_id_t* ids);

/******************************************************************************
 *
 * API Constrained Parameter Space Relationship
 *
 ******************************************************************************/

/*! Add new constrained relationships in the given constrained parameter space.
 *
 * \param[in] cps The constrained parameter space on which we want to add the
 *    relationships.
 * \param[in] group_uid The relationships group client UID, if wanted. If not
 *    equals to PSE_CLT_RELSHPS_GROUP_UID_INVALID, all given relationships will
 *    be added to a group which use the given UID. Later, it's possible to use
 *    this UID to apply operations on all associated relationships.
 * \param[in] count The number of relationships to add.
 * \param[in] params An array of parameters of all relationships to add. Must be
 *    at least of size \p count.
 * \param[out] ids The identifiers of the newly added relationships. Must be an
 *    array of at least size \p count.
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsAdd
  (struct pse_cpspace_t* cps,
   const pse_clt_relshps_group_uid_t group_uid,
   const size_t count,
   struct pse_cpspace_relshp_params_t* params,
   pse_relshp_id_t* ids);

/*! Remove existing constrained relationships from the given constrained
 * parameter space.
 *
 * \param[in] cps The constrained parameter space from which we want to remove
 *   constrained relationships.
 * \param[in] count The number of relationships to remove.
 * \param[in] ids The array of the identifiers of the relationships to remove.
 *   Must be an array of at least \p count elements.
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsRemove
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids);

/*! Remove existing constrained and grouped relationships from the given
 * constrained parameter space.
 *
 * \param[in] cps The constrained parameter space from which we want to remove
 *   constrained relationships.
 * \param[in] group_uid The group UID of relationships to remove.
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsRemoveByGroup
  (struct pse_cpspace_t* cps,
   const pse_clt_relshps_group_uid_t group_uid);

/*! Clear all constrained relationships of the given constrained parameter
 * space.
 *
 * \param[in] cps The constrained parameter space from which we want to remove
 *   all the constrained relationships.
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsClear
  (struct pse_cpspace_t* cps);

PSE_API size_t
pseConstrainedParameterSpaceRelationshipsCountGet
  (struct pse_cpspace_t* cps);

/*! Retreive parameters of the given relationships. Using the values of the
 * parameters after having modified the CPS could induce unexpected behavior.
 * If you need to keep this information, make a copy.
 * \param[in] cps The Constrained Parameter Space in which reside the given
 *    relationships
 * \param[in] count The number of parameters of relationships to retreive
 * \param[in] ids The relationships IDs
 * \param[out] params The retreived parameters of the relationships on success
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsParamsGet
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids,
   struct pse_cpspace_relshp_params_t* params);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsStateSet
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids,
   const enum pse_cpspace_relshp_state_t* states);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsSameStateSet
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids,
   const enum pse_cpspace_relshp_state_t state);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsStateSetByGroup
  (struct pse_cpspace_t* cps,
   const pse_clt_relshps_group_uid_t group_uid,
   const enum pse_cpspace_relshp_state_t state);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsStateGet
  (struct pse_cpspace_t* cps,
   const size_t count,
   const pse_relshp_id_t* ids,
   enum pse_cpspace_relshp_state_t* states);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipsGroupStateGet
  (struct pse_cpspace_t* cps,
   const pse_clt_relshps_group_uid_t group_uid,
   enum pse_cpspace_relshps_group_state_t* grp_state);

#if 0
/*! Set the constraints of a given relationship.
 *
 * \param[in] dev The device on which we will work.
 * \param[in] cps The constrained parameter space to which the relationship
 *   belongs.
 * \param[in] id The relationship identifier on which we want to set the
 *   constraints.
 * \param[in] cnstrs The constraints to set.
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipConstraintsSet
  (struct pse_cpspace_t* cps,
   const pse_relshp_id_t id,
   struct pse_cpspace_relshp_cnstrs_t* cnstrs);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipConstraintsAdd
  (struct pse_cpspace_t* cps,
   const pse_relshp_id_t id,
   struct pse_cpspace_relshp_cnstrs_t* cnstrs);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceRelationshipConstraintsRemove
  (struct pse_cpspace_t* cps,
   const pse_relshp_id_t id,
   struct pse_cpspace_relshp_cnstrs_t* cnstrs);
#endif

/******************************************************************************
 *
 * API Constrained Parameter Space Values
 *
 ******************************************************************************/

/*! Declare values for a given constrained parameter space.
 *
 * \param[in] cps The constrained parameter space to which the values will be
 *    associated. This link allow to check validity of the values regarding the
 *    CPS.
 * \param[in] data The values data.
 * \param[out] vals The newly created values object.
 * \return
 *    - ::RES_OK on success
 *    - ::RES_BAD_ARG if parameters are invalid
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceValuesCreate
  (struct pse_cpspace_t* cps,
   const struct pse_cpspace_values_data_t* data,
   struct pse_cpspace_values_t** vals);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceValuesRefAdd
  (struct pse_cpspace_values_t* vals);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceValuesRefSub
  (struct pse_cpspace_values_t* vals);

/*! This function allow the user to lock the values, before reading/writting the
 * values. A call to this function is needed to inform the device that something
 * happen out of its scope.
 *
 * \param vals The values to lock
 * \param params The lock parameters, mainly to say if it's for reading or
 *    writting or both.
 * \param data The data of the values. Filled only on success.
 * \return
 *    - ::RES_OK on success
 *    - ::RES_BAD_ARG if parameters are invalid
 *    - ::RES_BUSY if it's not possible to lock
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceValuesLock
  (struct pse_cpspace_values_t* vals,
   const struct pse_cpspace_values_lock_params_t* params,
   struct pse_cpspace_values_data_t** data);

/*! When the values has been read/written, call this function to inform the
 * device that the operation is finished.
 *
 * \param vals The values to unlock
 * \param data The data of the locked values, as given by
 *    ::pseConstrainedParameterSpaceValuesLock.
 * \return
 *    - ::RES_OK on success
 *    - ::RES_BAD_ARG if parameters are invalid
 *    - ::RES_INVALID if \p vals was not locked
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceValuesUnlock
  (struct pse_cpspace_values_t* vals,
   struct pse_cpspace_values_data_t** data);

/******************************************************************************
 * 
 * API Constrained Parameter Space Exploration
 *
 ******************************************************************************/

PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationContextCreate
  (struct pse_cpspace_t* cps,
   struct pse_cpspace_exploration_ctxt_params_t* params,
   struct pse_cpspace_exploration_ctxt_t** ctxt);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationContextRefAdd
  (struct pse_cpspace_exploration_ctxt_t* ctxt);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationContextRefSub
  (struct pse_cpspace_exploration_ctxt_t* ctxt);

PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationContextParamsGet
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_cpspace_exploration_ctxt_params_t* params);

/*! Initialize all cost functor evaluation contexts of the relationships
 * involved in the provided exploration context. We call the init callbacks
 * associated to the cost functors with the given \p params.
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationRelationshipsAllContextsInit
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   pse_clt_cost_func_ctxt_init_params_t params);

/*! Clean all cost functor evaluation contexts of the relationships involved in
 * the provided exploration context. We call the clean callbacks associated to
 * the cost functors given through the \p params of the
 * pseConstrainedParameterSpaceExplorationRelationshipsAllContextsInit function.
 *
 * \note This function is called automatically when the exploration context is
 * destroyed, but if you want to reinit the contexts, you may want to call this
 * function first to clean the previous state.
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationRelationshipsAllContextsClean
  (struct pse_cpspace_exploration_ctxt_t* ctxt);

/*! Do the full exploration optimization process.
 *
 * \param[in] ctxt The exploration context
 * \param[in] smpls The samples representing the initial state to start from
 * \return
 *    - ::RES_OK on success
 *    - ::RES_BAD_ARG if parameters are invalid
 *    - ::RES_BUSY if another exploration is already started
 *    - ::RES_NOT_FOUND if there was nothing to optimize (e.g. all points were
 *      locked)
 *    - ::RES_NOT_CONVERGED if the convergence was not possible in reasonable
 *      time, by respecting the convergence options attached to the context
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationSolve
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_cpspace_exploration_samples_t* smpls);

/*! Start the step-by-step iterative exploration optimization process.
 *
 * \param[in] ctxt The exploration context
 * \param[in] smpls The samples representing the initial state to start from
 * \return
 *    - ::RES_NOT_CONVERGED on success
 *    - ::RES_BAD_ARG if parameters are invalid
 *    - ::RES_BUSY if another exploration is already started
 *    - ::RES_NOT_FOUND if there was nothing to optimize (e.g. all points were
 *      locked). In this case, the exploration is not started
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationIterativeSolveBegin
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_cpspace_exploration_samples_t* smpls);

/*! Iterate one step on the currently started step-by-step exploration process.
 *
 * \param[in] ctxt The exploration context
 * \return
 *    - ::RES_OK on success
 *    - ::RES_BAD_ARG if parameters are invalid
 *    - ::RES_NOT_READY if there is no exploration started
 *    - ::RES_NOT_CONVERGED if the convergence was not possible in reasonable
 *      time, by respecting the convergence options attached to the context
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationIterativeSolveStep
  (struct pse_cpspace_exploration_ctxt_t* ctxt);

/*! Finalize the already started step-by-step iterative exploration
 * optimization process.
 *
 * \param[in] ctxt The exploration context
 * \return
 *    - ::RES_OK on success
 *    - ::RES_BAD_ARG if parameters are invalid
 *    - ::RES_NOT_READY if there is no exploration process to finalize
 *    - ::RES_BUSY if it's not possible to lock the context to finalize the
 *      process
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationIterativeSolveEnd
  (struct pse_cpspace_exploration_ctxt_t* ctxt);

/*! Retreive the last results of the exploration. Will automatically lock/unlock
 * \p where for writting.
 *
 * \param[in] ctxt The exploration context from where to retreive the last
 *    results.
 * \param[out] where The values where will be set the results of the
 *    exploration. If the data are not of the same parameter space than the one
 *    used during exploration, the user is responsible to do the conversion in
 *    the set callback of the coordinate attributes.
 * \param[out] extra The extra results of the exploration, mainly statistics.
 *    May be NULL if the user is not interested by these extra results.
 * \return
 *    - ::RES_OK on success
 *    - ::RES_BAD_ARG if parameters are invalid
 *    - ::RES_BUSY if it's not possible to lock
 *
 * \see pseConstrainedParameterSpaceValuesLock
 * \see pseConstrainedParameterSpaceValuesUnlock
 */
PSE_API enum pse_res_t
pseConstrainedParameterSpaceExplorationLastResultsRetreive
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_cpspace_values_t* where,
   struct pse_cpspace_exploration_extra_results_t* extra);

PSE_API_END

#endif /* PSE_H */
