#ifndef PSE_COLOR_PALETTE_H
#define PSE_COLOR_PALETTE_H

#include "pse_color_api.h"
#include "pse_color_types.h"

#include <clt/pse_clt.h>
#include <pse.h>

/*! \file
 * \brief Main API for the color palette manipulation.
 *
 * This a high-level layer above the PSE API that allows to manipulate
 * constrained color palettes. This file comes with a strong goal which is to
 * provide a simple set of functions that is sufficient to exploit the PSE API
 * without worrying about the details.
 */

PSE_API_BEGIN

struct pse_colors_ref_t;

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_color_palette_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

/*! Parameters of a color palette.
 * \param dev The PSE device in which the color palette will be created.
 * \param alloc The allocator to use by the color palette. May be NULL: the
 *    default allocator will be used in this case.
 * \param logger The logger to use by the color palette. May be NULL: no logs
 *    will be generated.
 */
struct pse_color_palette_params_t {
  struct pse_device_t* dev;
  struct pse_allocator_t* alloc;
  struct pse_logger_t* logger;
};

/*! Prototype of a function used to apply a variation on colors.
 * \param[in] user_data User specific data.
 * \param[in] from The input colors on which we want to apply the variation
 *    \p to_ppv.
 * \param[in] to_fmt The expected output format of the colors \p to.
 * \param[in] to_ppv The expected variation to be applied on the \p to colors.
 * \param[out] to The output colors, using the format \p fmt and on which the
 *    variation \p to_ppv has been applied.
 *
 * \warning Objects \p from and \p to could point on the same colors in
 *    memory.
 *
 * \see ::pse_color_variation_params_t::apply_user_data.
 */
typedef enum pse_res_t
(*pse_color_variation_apply_cb)
  (void* user_data,
   const struct pse_colors_ref_t* from,
   const pse_color_format_t to_fmt,
   const pse_color_variation_uid_t to_ppv,
   struct pse_colors_ref_t* to);

/*! Parameters of a color variation.
 * \param apply The callback to call to apply the variation.
 * \param apply_user_data The specific user data that will be passed to the \p
 *    apply callback when calling it.
 */
struct pse_color_variation_params_t {
  pse_color_variation_apply_cb apply;
  void* apply_user_data;
};

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_VARIATION_USER_START_UID_                                    \
  ((pse_color_variation_uid_t)(UINT32_MAX>>1))
#define PSE_COLOR_PALETTE_PARAMS_NULL_                                         \
  { NULL, NULL, NULL }
#define PSE_COLOR_VARIATION_PARAMS_NULL_                                       \
  { NULL, NULL }
#define PSE_COLOR_SPACE_COMPONENTS_COORDS_3r_                                  \
  { { PSE_TYPE_REAL }, { PSE_TYPE_REAL }, { PSE_TYPE_REAL } }
#define PSE_COLOR_SPACE_COMPONENTS_LOCK_STATUS_                                \
  { { PSE_TYPE_BOOL_8 } }
#define PSE_COLOR_SPACE_PARAMS_3r_                                             \
  {{{                                                                          \
    { 3, PSE_COLOR_SPACE_COMPONENTS_COORDS_3r },                               \
    { 1, PSE_COLOR_SPACE_COMPONENTS_LOCK_STATUS }                              \
  }}, 0, NULL /* No variations at start */ }

static const pse_color_variation_uid_t PSE_COLOR_VARIATION_USER_START_UID =
  PSE_COLOR_VARIATION_USER_START_UID_;
static const struct pse_color_palette_params_t PSE_COLOR_PALETTE_PARAMS_NULL =
  PSE_COLOR_PALETTE_PARAMS_NULL_;
static const struct pse_color_variation_params_t PSE_COLOR_VARIATION_PARAMS_NULL =
  PSE_COLOR_VARIATION_PARAMS_NULL_;
static const struct pse_pspace_point_attrib_component_t PSE_COLOR_SPACE_COMPONENTS_COORDS_3r[3] =
  PSE_COLOR_SPACE_COMPONENTS_COORDS_3r_;
static const struct pse_pspace_point_attrib_component_t PSE_COLOR_SPACE_COMPONENTS_LOCK_STATUS[1] =
  PSE_COLOR_SPACE_COMPONENTS_LOCK_STATUS_;
static const struct pse_pspace_params_t PSE_COLOR_SPACE_PARAMS_Cat02LMSr =
  PSE_COLOR_SPACE_PARAMS_3r_;
static const struct pse_pspace_params_t PSE_COLOR_SPACE_PARAMS_HSVr =
  PSE_COLOR_SPACE_PARAMS_3r_;
static const struct pse_pspace_params_t PSE_COLOR_SPACE_PARAMS_LABr =
  PSE_COLOR_SPACE_PARAMS_3r_;
static const struct pse_pspace_params_t PSE_COLOR_SPACE_PARAMS_RGBr =
  PSE_COLOR_SPACE_PARAMS_3r_;
static const struct pse_pspace_params_t PSE_COLOR_SPACE_PARAMS_XYZr =
  PSE_COLOR_SPACE_PARAMS_3r_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_COLOR_API enum pse_res_t
pseColorPaletteCreate
  (const struct pse_color_palette_params_t* params,
   struct pse_color_palette_t** cp);

PSE_COLOR_API enum pse_res_t
pseColorPaletteDestroy
  (struct pse_color_palette_t* cp);

PSE_COLOR_API struct pse_cpspace_t*
pseColorPaletteConstrainedParameterSpaceGet
  (struct pse_color_palette_t* cp);

PSE_COLOR_API size_t
pseColorPalettePointCountGet
  (struct pse_color_palette_t* cp);

PSE_COLOR_API enum pse_res_t
pseColorPalettePointCountSet
  (struct pse_color_palette_t* cp,
   const size_t count);

PSE_COLOR_API enum pse_res_t
pseColorPaletteColorsIndexToParametricPointsId
  (struct pse_color_palette_t* cp,
   const size_t count,
   const pse_color_idx_t* indices,
   pse_ppoint_id_t* ids);

PSE_COLOR_API enum pse_res_t
pseColorPaletteVariationsDeclare
  (struct pse_color_palette_t* cp,
   const size_t count,
   const pse_color_variation_uid_t* uids,
   const struct pse_color_variation_params_t* params);

PSE_COLOR_API enum pse_res_t
pseColorPaletteVariationsForget
  (struct pse_color_palette_t* cp,
   const size_t count,
   const pse_color_variation_uid_t* uids);

PSE_API_END

#endif /* PSE_COLOR_PALETTE_H */
