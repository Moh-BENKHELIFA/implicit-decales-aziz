#ifndef PSE_COLOR_VISION_DEFICIENCIES_H
#define PSE_COLOR_VISION_DEFICIENCIES_H

#include "pse_color_api.h"
#include "pse_color_types.h"

PSE_API_BEGIN

struct pse_color_palette_t;
struct pse_colors_ref_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

/*! List of all color variations related to Color Vision Deficiencies (CVD). */
enum pse_color_vision_deficiency_variation_t {
  /*!
   * The Rasche2005 variations implement simulations proposed in
   *   Rasche, K.; Geist, R.; Westall, J.,
   *   Detail preserving reproduction of color images for monochromats and dichromats,
   *   in Computer Graphics and Applications, IEEE , vol.25, no.3, pp.22-30, May-June 2005
   *   doi: 10.1109/MCG.2005.54
   *   http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1438255&isnumber=30967
   */
  PSE_CVD_DEUTERANOPIA_Rasche2005,
  PSE_CVD_PROTANOPIA_Rasche2005,

  /*!
   * The Troiano2008 variations implement simulations proposed in
   *   Luigi Troiano, Cosimo Birtolo, and Maria Miranda. 2008.
   *   Adapting palettes to color vision deficiencies by genetic algorithm.
   *   In Proceedings of the 10th annual conference on Genetic and evolutionary
   *   computation (GECCO '08), Maarten Keijzer (Ed.).
   *   ACM, New York, NY, USA, 1065-1072.
   *   DOI=http://dx.doi.org/10.1145/1389095.1389291
   *
   * \deprecated
   */
  PSE_CVD_DEUTERANOPIA_Troiano2008,
  PSE_CVD_PROTANOPIA_Troiano2008,

  PSE_CVD_VARIATIONS_COUNT_
};

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_COLOR_API enum pse_res_t
pseColorsCVDVariationApply
  (const enum pse_color_vision_deficiency_variation_t cvdv,
   const struct pse_colors_ref_t* src,
   struct pse_colors_ref_t* dst);

PSE_COLOR_API enum pse_res_t
pseColorsCVDVariationApplyInPlace
  (const enum pse_color_vision_deficiency_variation_t cvdv,
   struct pse_colors_ref_t* clrs);

/*! Add color variations related to CVD to the color palette \p cp.
 * \param[in,out] cp Color palette to which will be added the varitions.
 * \param[in] count Number of variations pointed by \p cvdvs.
 * \param[in] cvdvs CVD color variations to add to the color palette \p cp.
 * \param[out] uids Variations UIDs. Must be an array of size at least \p count.
 */
PSE_COLOR_API enum pse_res_t
pseColorPaletteVariationsAddForCVD
  (struct pse_color_palette_t* cp,
   const size_t count,
   const enum pse_color_vision_deficiency_variation_t* cvdvs,
   pse_color_variation_uid_t* uids);

/*! CVD UID can be considered constant during the lifespan of a program. This
 * function allows to get the UID associated to a given CVD. This is the same
 * value than the one returned by the ::pseColorPaletteVariationsAddForCVD
 * function. */
PSE_COLOR_API pse_color_variation_uid_t
pseColorsCVDVariationUID
  (const enum pse_color_vision_deficiency_variation_t cvdv);

PSE_API_END

#endif /* PSE_COLOR_VISION_DEFICIENCIES_H */
