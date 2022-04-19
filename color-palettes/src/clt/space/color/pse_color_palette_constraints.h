#ifndef PSE_COLOR_PALETTE_CONSTRAINTS_H
#define PSE_COLOR_PALETTE_CONSTRAINTS_H

#include "pse_color_api.h"
#include "pse_color_types.h"

#include <pse.h>

PSE_API_BEGIN

struct pse_color_palette_t;
struct pse_color_constraint_distance_config_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

typedef void* pse_color_constraint_custom_data_t;

/*! Describe which colors are involved in a constraint.
 * \param kind The kind of the relationship.
 * \param colors_count The number of colors taken into account in the constraint.
 * \param colors_idx The array of the indices of the colors that will be taken
 *    into account. See ::pse_relshp_kind_t to check how this will happen,
 *    depending on the \p kind. Each color index must appear at most once!
 */
struct pse_color_space_constraint_colors_ref_t {
  enum pse_relshp_kind_t kind;
  size_t colors_count;
  pse_color_idx_t* colors_idx;
};

/*! Generic parameters of a weighted constraint of a color palette that is
 * applicable on a specific color space.
 * \param space The space in which this constraint is valid.
 * \param components The components involved in the constraint. Set it to
 *    \c true to make them involved.
 * \param variations_count The number of variations in which this constraint
 *    must be applied.
 * \param variations The array of variations in which this contraint must be
 *    applied. This array is at least of size \p variations_count.
 * \param weight The weight of this constraint. If the weight is equal to 0, the
 *    constraint will be disabled.
 * \param colors_ref The reference to colors that will be involved in the
 *    constraint. See ::pse_color_space_constraint_colors_ref_t for more
 *    information.
 *
 * \note The weight will not be normalized!
 */
struct pse_color_space_constraint_params_t {
  pse_color_space_t space;
  pse_color_components_flags_t components;
  size_t variations_count;
  pse_color_variation_uid_t* variations;
  pse_real_t weight;
  struct pse_color_space_constraint_colors_ref_t colors_ref;
};

/*! Distance parameters for the constraints which are based on distances. Each
 * constraint will explain how it uses these parameters. */
struct pse_color_distance_params_t {
  pse_real_t threshold;
};

/*! Callback called on the creation of a custom constraint.
 * \param[in,out] custom_data The opaque custom data associated to the
 *    constraint. May be NULL if not used.
 * \param[in,out] cp The color palette in which the constraint is currently
 *    constructed.
 * \param[in] params The parameters of the color space associated with the
 *    constraint.
 * \param[in] id The currently constructred constraint ID.
 * \note The callback must call the ::pseColorPaletteConstraintRelationshipsPush
 * function to add relationships associated with the custom constraint.
 */
typedef enum pse_res_t
(*pse_color_palette_constraint_custom_create_cb)
  (pse_color_constraint_custom_data_t custom_data,
   struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id,
   const struct pse_color_space_constraint_params_t* params);

/*! Callback called during the destruction of a custom constraint.
 * \param[in,out] custom_data The opaque custom data associated to the
 *    constraint. May be NULL if not used.
 * \param[in,out] cp The color palette in which the constraint will be destroyed.
 * \param[in] id The ID of the constraint we want to destroy.
 * \note On remove, the relationships associated with the custom constraint are
 * automatically destroyed. Therefore, this callback must only clean the custom
 * data.
 */
typedef enum pse_res_t
(*pse_color_palette_constraint_custom_destroy_cb)
  (pse_color_constraint_custom_data_t custom_data,
   struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id);

struct pse_color_palette_constraint_params_t {
  struct pse_color_space_constraint_params_t space;
  pse_color_palette_constraint_custom_create_cb create;
  pse_color_palette_constraint_custom_destroy_cb destroy;
  pse_color_constraint_custom_data_t custom_data;
};

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_NONE_                            \
  { PSE_RELSHP_KIND_INCLUSIVE, 0, NULL }
#define PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_ALL_                             \
  { PSE_RELSHP_KIND_EXCLUSIVE, 0, NULL }
#define PSE_COLOR_SPACE_CONSTRAINT_PARAMS_NULL_                                \
  { PSE_COLOR_SPACE_INVALID_, PSE_COLOR_COMPONENTS_NONE, 0, NULL, 1.0,         \
    PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_NONE_ }
#define PSE_COLOR_DISTANCE_PARAMS_DEFAULT_                                     \
  { (pse_real_t)0 }
#define PSE_COLOR_PALETTE_CONSTRAINT_PARAMS_NULL_                              \
  { PSE_COLOR_SPACE_CONSTRAINT_PARAMS_NULL_, NULL, NULL, NULL }
#define PSE_COLOR_CONSTRAINT_INGAMUT_WEIGHT_DEFAULT_                           \
  ((pse_real_t)1000)

static const struct pse_color_space_constraint_colors_ref_t PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_NONE =
  PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_NONE_;
static const struct pse_color_space_constraint_colors_ref_t PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_ALL =
  PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_ALL_;
static const struct pse_color_space_constraint_params_t PSE_COLOR_SPACE_CONSTRAINT_PARAMS_NULL =
  PSE_COLOR_SPACE_CONSTRAINT_PARAMS_NULL_;
static const struct pse_color_distance_params_t PSE_COLOR_DISTANCE_PARAMS_DEFAULT =
  PSE_COLOR_DISTANCE_PARAMS_DEFAULT_;
static const struct pse_color_palette_constraint_params_t PSE_COLOR_PALETTE_CONSTRAINT_PARAMS_NULL =
  PSE_COLOR_PALETTE_CONSTRAINT_PARAMS_NULL_;
static const pse_real_t PSE_COLOR_CONSTRAINT_INGAMUT_WEIGHT_DEFAULT =
  PSE_COLOR_CONSTRAINT_INGAMUT_WEIGHT_DEFAULT_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

/*! Create a new custom constraint the color palette. The callback
 * ::pse_color_palette_constraint_params_t::create will be called by this
 * function in order to create the associated relationships of the constraint.
 * \param[in] cp The color palette in which will reside the new custom
 *    constraint.
 * \param[in] params The parameters of the custom constraint we want to create.
 * \param[out] id The ID of the newly created custom constraint, on success.
 */
PSE_COLOR_API enum pse_res_t
pseColorPaletteConstraintAdd
  (struct pse_color_palette_t* cp,
   struct pse_color_palette_constraint_params_t* params,
   pse_color_palette_constraint_id_t* id);

/*! Remove an existing constraint of the color palette. For custom constraint,
 * this function will call the ::pse_color_palette_constraint_params_t::destroy
 * callback. */
PSE_COLOR_API enum pse_res_t
pseColorPaletteConstraintRemove
  (struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id);

/*! Enable/disable a constraint of the color palette. */
PSE_COLOR_API enum pse_res_t
pseColorPaletteConstraintSwitch
  (struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id,
   bool enable);

/*! Add some CPS relationships associated to the already existing constraint
 * \p id. This function must be used by those who wants to implement their own
 * constraints on color palettes.
 * \param[in] cp The color palette in which the constraint resides.
 * \param[in] id The ID of the constraint to which we want to add the
 *    relationships.
 * \param[in] relshps_count The number of relationships to add to the constraint.
 * \param[in] relshps The array of parameters of the relationships we want to
 *    add to the constraint.
 */
PSE_COLOR_API enum pse_res_t
pseColorPaletteConstraintRelationshipsPush
  (struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id,
   const size_t relshps_count,
   struct pse_cpspace_relshp_params_t* relshps);

/******************************************************************************
 *
 * PUBLIC HELPERS API
 *
 ******************************************************************************/

/*! Resolve the colors references, by including/excluding the colors by their
 * indices in the palette.
 * \param[in] cp The color palette in which the colors reside.
 * \param[in] colors_ref The colors reference to resolve.
 * \param[out] ppoints_count The number of colors resolved and stored in
 *    \p ppoints_id.
 * \param[out] ppoints_id The array that contains the resolved colors, i.e. the
 *    parametric point id in the CPS associated to the color palette.
 */
PSE_COLOR_API enum pse_res_t
pseColorPaletteConstraintColorsRefResolveNow
   (struct pse_color_palette_t* cp,
    const struct pse_color_space_constraint_colors_ref_t* colors_ref,
    size_t* ppoints_count,
    pse_ppoint_id_t** ppoints_id);

/*! Clean the buffer previously returned by a call to
 * ::pseColorPaletteConstraintColorsRefResolveNow. This function succeed if
 * \p ppoints_count is 0 and \p ppoints_id is NULL.
 */
PSE_COLOR_API void
pseColorPaletteConstraintColorsRefBufferClean
   (struct pse_color_palette_t* cp,
    const size_t ppoints_count,
    pse_ppoint_id_t* ppoints_id);

/*! Add a constraint on the color palette to keep the colors in the gamut of the
 * color space provided through parameters. Works only with RGB color space,
 * will fail with all other color spaces.
 */
PSE_COLOR_API enum pse_res_t
pseColorPaletteConstraintInGamutAdd
  (struct pse_color_palette_t* cp,
   const struct pse_color_space_constraint_params_t* params,
   pse_color_palette_constraint_id_t* id);

/*! Add a constraint on the color palette to force the distance between the
 * colors referenced by \p params.colors_ref.
 * \note The \p threshold of distance parameters is used as the maximum
 * distance. If the computed distance is greated than this threshold, the
 * constraint will not cost anymore.
 * \note The distance is computed in the space provided through \p params,
 * taking into account only the components provided.
 */
PSE_COLOR_API enum pse_res_t
pseColorPaletteConstraintDistanceAdd
  (struct pse_color_palette_t* cp,
   const struct pse_color_space_constraint_params_t* params,
   const struct pse_color_distance_params_t* dist_params,
   pse_color_palette_constraint_id_t* id);

/*! Add a constraint on the color palette to force the distance between the
 * colors referenced by \p params.colors_ref, per component. This constraint
 * can be used to force translation only, to ensure that gradients are
 * respected, etc.
 * \note The \p threshold of distance parameters is used as the maximum
 * distance. If the computed distance is greated than this threshold, the
 * constraint will not cost anymore.
 * \note The distance is computed in the space provided through \p params and
 * only for the components provided.
 */
PSE_COLOR_API enum pse_res_t
pseColorPaletteConstraintDistancePerComponentAdd
  (struct pse_color_palette_t* cp,
   const struct pse_color_space_constraint_params_t* params,
   const struct pse_color_distance_params_t* dist_params,
   pse_color_palette_constraint_id_t* id);

/*! Add a constraint on the energy consumption on OLED screens of the colors
 * referenced by \p params.colors_ref. Works only with RGB color space, as its
 * the way OLED screens are built. The energy cost is relative to max(R,G,B).
 */
PSE_COLOR_API enum pse_res_t
pseColorPaletteConstraintOLEDScreenEnergyConsumptionAdd
  (struct pse_color_palette_t* cp,
   const struct pse_color_space_constraint_params_t* params,
   pse_color_palette_constraint_id_t* id);

PSE_API_END

#endif /* PSE_COLOR_PALETTE_CONSTRAINTS_H */
