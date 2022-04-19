#include "pse_color_space_Cat02_LMS.h"

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

void
pseColorCat02LMSInGamutClampInPlace
  (struct pse_color_Cat02LMS_t* clr)
{
  switch(clr->type) {
    case PSE_COLOR_TYPE_Cat02LMSr: {
      clr->as.Cat02LMSr.L = PSE_MAX(clr->as.Cat02LMSr.L, 0.0);
      clr->as.Cat02LMSr.M = PSE_MAX(clr->as.Cat02LMSr.M, 0.0);
      clr->as.Cat02LMSr.S = PSE_MAX(clr->as.Cat02LMSr.S, 0.0);
    } break;
    default: break;
  }
}

void
pseColorsCat02LMSInGamutClampInPlace
  (struct pse_colors_Cat02LMS_t* clrs)
{
  switch(clrs->type) {
    case PSE_COLOR_TYPE_Cat02LMSr: {
      size_t i;
      for(i = 0; i < clrs->count; ++i) {
        clrs->as.Cat02LMSr[i].L = PSE_MAX(clrs->as.Cat02LMSr[i].L, 0.0);
        clrs->as.Cat02LMSr[i].M = PSE_MAX(clrs->as.Cat02LMSr[i].M, 0.0);
        clrs->as.Cat02LMSr[i].S = PSE_MAX(clrs->as.Cat02LMSr[i].S, 0.0);
      }
    } break;
    default: break;
  }
}
