#include "pse_color_space_RGB.h"

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

void
pseColorRGBInGamutClampInPlace
  (struct pse_color_RGB_t* clr)
{
  switch(clr->type) {
    case PSE_COLOR_TYPE_RGBr: {
      clr->as.RGBr.R = PSE_CLAMP(clr->as.RGBr.R, 0.0, 1.0);
      clr->as.RGBr.G = PSE_CLAMP(clr->as.RGBr.G, 0.0, 1.0);
      clr->as.RGBr.B = PSE_CLAMP(clr->as.RGBr.B, 0.0, 1.0);
    } break;
    default: break;
  }
}

void
pseColorsRGBInGamutClampInPlace
  (struct pse_colors_RGB_t* clrs)
{
  switch(clrs->type) {
    case PSE_COLOR_TYPE_RGBr: {
      size_t i;
      for(i = 0; i < clrs->count; ++i) {
        clrs->as.RGBr[i].R = PSE_CLAMP(clrs->as.RGBr[i].R, 0.0, 1.0);
        clrs->as.RGBr[i].G = PSE_CLAMP(clrs->as.RGBr[i].G, 0.0, 1.0);
        clrs->as.RGBr[i].B = PSE_CLAMP(clrs->as.RGBr[i].B, 0.0, 1.0);
      }
    } break;
    default: break;
  }
}
