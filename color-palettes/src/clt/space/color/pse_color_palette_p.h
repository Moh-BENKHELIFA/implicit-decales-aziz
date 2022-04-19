#ifndef PSE_COLOR_PALETTE_P_H
#define PSE_COLOR_PALETTE_P_H

#include "pse_color_palette.h"

#include "pse_color_cost.h"
#include "pse_color_space_Cat02_LMS.h"
#include "pse_color_space_HSV.h"
#include "pse_color_space_Lab.h"
#include "pse_color_space_RGB.h"
#include "pse_color_space_XYZ.h"

PSE_API_BEGIN

struct pse_color_palette_constraint_t;

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

/*! LABr, RGBr */
#define PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT  5

struct pse_color_variation_data_t {
  pse_color_variation_uid_t key;
  struct pse_color_variation_params_t params;
};

struct pse_color_cost_func_data_t {
  pse_clt_cost_func_uid_t key;
  struct pse_color_cost_func_config_t cfg;
  pse_relshp_cost_func_id_t fid;
};

/*! Data associated to a color space.
 * \param key The color format.
 * \warning As the drivers use only real values during their calculations, we
 *    only use the real types and their associated formats: RGBr, LABr, etc.
 */
struct pse_color_space_data_t {
  pse_color_format_t key;
};

/*! Represents a color palette.
 * \param dev The device in which the color palette resides.
 * \param alloc The allocator to be used by the color palette operations.
 * \param logger The logger to be used by the color palette operations.
 * \param cps The Constrained Parameter Space representing the constrained color
 *    palette.
 * \param ppoints_id The list of all parametric points ids representing each
 *    color of the palette.
 * \param pspaces_uid The UIDs of the parameters spaces used internally during
 *    the computations only. Other formats are managed as input/output, but not
 *    as storage for the computations.
 * \param pspaces The data associated to each parameter space stored in
 *    \a pspaces_uid.
 * \param variations_uid The UIDs of the variations enabled on the color
 *    palette.
 * \param variations The data associated to each variation stored in
 *    \a variations_uid.
 * \param cfuncs The information related to cost functors that can be used by
 *    this color palette.
 * \param constraints The information related to constraints that can be applied
 *    on this color palette.
 */
struct pse_color_palette_t {
  struct pse_device_t* dev;
  struct pse_allocator_t* alloc;
  struct pse_logger_t* logger;
  struct pse_cpspace_t* cps;
  pse_ppoint_id_t* ppoints_id; /* stretchy buffer */
  pse_color_format_t* formats; /* stretchy buffer */
  struct pse_color_space_data_t* pspaces; /* std_ds hm */
  pse_color_variation_uid_t* variations_uid; /* stretchy buffer */
  struct pse_color_variation_data_t* variations; /* stb_ds hm */
  struct pse_color_cost_func_data_t* cfuncs; /* stb_ds hm */
  pse_color_palette_constraint_id_t constraint_next_id;
  struct pse_color_palette_constraint_t* constraints; /* stb_ds hm */
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_VARIATION_DATA_NULL_                                         \
  { PSE_CLT_PPOINT_VARIATION_UID_INVALID_, PSE_COLOR_VARIATION_PARAMS_NULL_ }
#define PSE_COLOR_SPACE_DATA_NULL_                                             \
  { PSE_CLT_PSPACE_UID_INVALID_ }
#define PSE_COLOR_COST_FUNC_DATA_NULL_                                         \
  { PSE_CLT_COST_FUNC_UID_INVALID_, PSE_COLOR_COST_FUNC_CONFIG_NULL_,          \
    PSE_RELSHP_COST_FUNC_ID_INVALID_ }
#define PSE_COLOR_PALETTE_NULL_                                                \
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL }

static const pse_color_space_t PSE_COLOR_PALETTE_SPACES_MANAGED[PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT] = {
  PSE_COLOR_SPACE_LAB,
  PSE_COLOR_SPACE_RGB,
  PSE_COLOR_SPACE_Cat02LMS,
  PSE_COLOR_SPACE_HSV,
  PSE_COLOR_SPACE_XYZ
};
static const pse_color_format_t PSE_COLOR_PALETTE_COMPUTATION_FORMATS_MANAGED[PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT] = {
  PSE_COLOR_FORMAT_LABr_,
  PSE_COLOR_FORMAT_RGBr_,
  PSE_COLOR_FORMAT_Cat02LMSr_,
  PSE_COLOR_FORMAT_HSVr_,
  PSE_COLOR_FORMAT_XYZr_
};
static const struct pse_color_variation_data_t PSE_COLOR_VARIATION_DATA_NULL =
  PSE_COLOR_VARIATION_DATA_NULL_;
static const struct pse_color_space_data_t PSE_COLOR_SPACE_DATA_NULL =
  PSE_COLOR_SPACE_DATA_NULL_;
static const struct pse_color_cost_func_data_t PSE_COLOR_COST_FUNC_DATA_NULL =
  PSE_COLOR_COST_FUNC_DATA_NULL_;
static const struct pse_color_palette_t PSE_COLOR_PALETTE_NULL =
  PSE_COLOR_PALETTE_NULL_;

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

static PSE_FINLINE bool
pseIsColorSpaceManaged
  (const pse_color_space_t space)
{
  size_t i;
  for(i = 0; i < PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT; ++i) {
    if( PSE_COLOR_PALETTE_SPACES_MANAGED[i] == space )
      return true;
  }
  return false;
}

static PSE_FINLINE pse_color_format_t
pseColorFormatManagedFromSpace
  (const pse_color_space_t space)
{
  size_t i;
  for(i = 0; i < PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT; ++i) {
    if( PSE_COLOR_PALETTE_SPACES_MANAGED[i] == space )
      return PSE_COLOR_PALETTE_COMPUTATION_FORMATS_MANAGED[i];
  }
  return PSE_COLOR_FORMAT_INVALID;
}

PSE_API_END

#endif /* PSE_COLOR_PALETTE_P_H */
