#include <clt/space/color/pse_color_palette.h>
#include <clt/space/color/pse_color_palette_constraints.h>
#include <clt/space/color/pse_color_palette_exploration.h>
#include <clt/space/color/pse_color.h>
#include <clt/space/color/pse_color_vision_deficiencies.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <ColorSpace/colorspace.h>

#define PSE_COLORS_COUNT_DEFAULT  64

#define RAND_SEED 123456  /* time(NULL) */

#define PRINT_PERFS
/*#define PRINT_INITIAL_PPOINTS*/
/*#define PRINT_FINAL_PPOINTS*/
#define PRINT_PPM

static PSE_INLINE void
setRandomColorsRGBr
  (struct pse_colors_t* colors)
{
  struct pse_color_RGBr_t* values = NULL;
  size_t i;
  PSE_CALL(pseColorsConvertInPlace(colors, PSE_COLOR_FORMAT_RGBr));
  values = colors->as.RGB.as.RGBr;
  for(i = 0; i < colors->as.any.count; ++i) {
    values[i].R = (pse_real_t)(rand() % 100000) / (pse_real_t)100000;
    values[i].G = (pse_real_t)(rand() % 100000) / (pse_real_t)100000;
    values[i].B = (pse_real_t)(rand() % 100000) / (pse_real_t)100000;
  }
}

static PSE_INLINE void
setColorsRGBr
  (struct pse_colors_t* colors,
   const struct pse_color_RGBr_t* color)
{
  struct pse_color_RGBr_t* values = NULL;
  size_t i;
  PSE_CALL(pseColorsConvertInPlace(colors, PSE_COLOR_FORMAT_RGBr));
  values = colors->as.RGB.as.RGBr;
  for(i = 0; i < colors->as.any.count; ++i) {
    values[i] = *color;
  }
}

static PSE_INLINE void
setColorsLABr
  (struct pse_colors_t* colors,
   const struct pse_color_LABr_t* color)
{
  struct pse_color_LABr_t* values = NULL;
  size_t i;
  PSE_CALL(pseColorsConvertInPlace(colors, PSE_COLOR_FORMAT_LABr));
  values = colors->as.Lab.as.LABr;
  for(i = 0; i < colors->as.any.count; ++i) {
    values[i] = *color;
  }
}

static PSE_INLINE void
printColors
  (const struct pse_colors_t* colors)
{
  size_t i;
  struct pse_colors_t temp = PSE_COLORS_INVALID;
  PSE_CALL(pseColorsAllocate
    (&PSE_ALLOCATOR_DEFAULT, PSE_COLOR_FORMAT_RGBr,
     colors->as.any.count, &temp));
  PSE_CALL(pseColorsConvert(colors, &temp));
  for(i = 0; i < colors->as.any.count; ++i) {
    fprintf(stdout, "Color %d: {%f, %f, %f}\n",
      (int)i,
      temp.as.RGB.as.RGBr[i].R,
      temp.as.RGB.as.RGBr[i].G,
      temp.as.RGB.as.RGBr[i].B);
  }
  PSE_CALL(pseColorsFree(&PSE_ALLOCATOR_DEFAULT, &temp));
}

static PSE_INLINE void
printPPM
  (const size_t count,
   const struct pse_colors_t* colors[])
{
#define _STR(a) #a
#define STR(a)  _STR(a)
#define MAXVAL  255
  FILE* f = stdout;
  size_t i, j, colors_count;
  struct pse_colors_t temp = PSE_COLORS_INVALID;
  colors_count = colors[0]->as.any.count;
  fprintf(f,
    "P3\n"     /* Magick number: P3 -> ASCII portable RGB color image */
    "%d "      /* Image width */
    "%d\n"     /* Image height */
    "%d \n",   /* Max color value. We use the maximum to have more precision */
    (int)colors_count,
    (int)count,
    (int)MAXVAL);
  PSE_CALL(pseColorsAllocate
    (&PSE_ALLOCATOR_DEFAULT, PSE_COLOR_FORMAT_RGBr, colors_count, &temp));
  for(i = 0; i < count; ++i) {
    PSE_CALL(pseColorsConvert(colors[i], &temp));
    pseColorsRGBInGamutClampInPlace(&temp.as.RGB);
    for(j = 0; j < colors_count; ++j) {
      fprintf(f, "%d %d %d\t",
        (int)(MAXVAL * temp.as.RGB.as.RGBr[j].R),
        (int)(MAXVAL * temp.as.RGB.as.RGBr[j].G),
        (int)(MAXVAL * temp.as.RGB.as.RGBr[j].B)
      );
    }
    fprintf(f,"\n");
  }
  PSE_CALL(pseColorsFree(&PSE_ALLOCATOR_DEFAULT, &temp));
#undef MAXVAL
#undef STR
#undef _STR
}

static PSE_FINLINE double
clock_elps_ms
  (struct timespec* tstart,
   struct timespec* tend)
{
  const uint64_t ielps =
        (tend->tv_sec * 1000000000 +   tend->tv_nsec)
    - (tstart->tv_sec * 1000000000 + tstart->tv_nsec);
  return (double)ielps / 1000000.0;
}

int main(int argc, char* argv[])
{
  struct timespec ts, te;
  struct pse_allocator_t* alloc = &PSE_ALLOCATOR_DEFAULT;
  struct pse_color_palette_params_t cpp = PSE_COLOR_PALETTE_PARAMS_NULL;
  struct pse_color_palette_exploration_ctxt_params_t excp = PSE_COLOR_PALETTE_EXPLORATION_CTXT_PARAMS_DEFAULT;
  struct pse_color_space_constraint_params_t cscp = PSE_COLOR_SPACE_CONSTRAINT_PARAMS_NULL;

  struct pse_device_params_t devp = PSE_DEVICE_PARAMS_NULL;
  struct pse_cpspace_exploration_extra_results_t results = PSE_CPSPACE_EXPLORATION_EXTRA_RESULTS_NULL;

  struct pse_colors_t rand_colors = PSE_COLORS_INVALID;
  struct pse_colors_t refs_colors = PSE_COLORS_INVALID;
  struct pse_colors_t smpls_colors = PSE_COLORS_INVALID;
  struct pse_colors_t opts_colors = PSE_COLORS_INVALID;
  pse_color_idx_t locked[1] = { 0 };

  struct pse_device_t* dev = NULL;
  struct pse_cpspace_exploration_ctxt_t* exc = NULL;
  struct pse_color_palette_t* cp = NULL;

  size_t cvds_count = 0;
  pse_clt_ppoint_variation_uid_t cvds_uid[1] = {
    PSE_CLT_PPOINT_VARIATION_UID_INVALID_
  };
  enum pse_color_vision_deficiency_variation_t cvds[1] = {
    PSE_CVD_DEUTERANOPIA_Rasche2005
  };

  pse_color_palette_constraint_id_t ingmt =
    PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID;
  pse_color_palette_constraint_id_t trsonly =
    PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID;

  size_t colors_count = PSE_COLORS_COUNT_DEFAULT;
  const char* driver_filepath = PSE_LIB_NAME("pse-drv-eigen");

  if(argc >= 2)
    driver_filepath = argv[1];
  if(argc >= 3)
    colors_count = (size_t)atoi(argv[2]);

  PSE_CALL(pseColorsAllocate(alloc, PSE_COLOR_FORMAT_RGBr, colors_count, &rand_colors));
  PSE_CALL(pseColorsAllocate(alloc, PSE_COLOR_FORMAT_LABr, colors_count, &refs_colors));
  PSE_CALL(pseColorsAllocate(alloc, PSE_COLOR_FORMAT_RGBr, colors_count, &smpls_colors));
  PSE_CALL(pseColorsAllocate(alloc, PSE_COLOR_FORMAT_RGBui8, colors_count, &opts_colors));

  devp.allocator = alloc;
  devp.logger = &PSE_LOGGER_STDOUT;
  devp.backend_drv_filepath = driver_filepath;

#ifdef RAND_SEED
  srand(RAND_SEED);  /* To have same values everytime */
#endif
  setRandomColorsRGBr(&rand_colors);
  PSE_CALL(pseColorsConvert(&rand_colors, &refs_colors));
  PSE_CALL(pseColorsConvert(&refs_colors, &smpls_colors));

#ifdef PRINT_INITIAL_PPOINTS
  fprintf(stdout, "Initial colors ===================\n");
  printColors(&smpls_colors);
#endif

  /* Create a palex device */
  PSE_CALL(pseDeviceCreate(&devp, &dev));

  /* Create a color palette */
  cpp.alloc = alloc;
  cpp.dev = dev;
  PSE_CALL(pseColorPaletteCreate(&cpp, &cp));
  PSE_CALL(pseColorPalettePointCountSet(cp, colors_count));
  if( cvds_count > 0 ) {
    PSE_CALL(pseColorPaletteVariationsAddForCVD
      (cp, cvds_count, cvds, cvds_uid));
  }

  cscp.space = PSE_COLOR_SPACE_RGB;
  cscp.components = PSE_COLOR_COMPONENTS_ALL;
  cscp.colors_ref = PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_ALL;
  cscp.variations_count = cvds_count;
  cscp.variations = cvds_uid;
  cscp.weight = PSE_COLOR_CONSTRAINT_INGAMUT_WEIGHT_DEFAULT;
  PSE_CALL(pseColorPaletteConstraintInGamutAdd(cp, &cscp, &ingmt));

  cscp.space = PSE_COLOR_SPACE_LAB;
  cscp.variations_count = 0;
  cscp.variations = NULL;
  cscp.weight = 1.0;
  PSE_CALL(pseColorPaletteConstraintDistancePerComponentAdd
    (cp, &cscp, &PSE_COLOR_DISTANCE_PARAMS_DEFAULT, &trsonly));

  /* TODO: expose a way to add any constraint between any parametric point
   * easily. */

  clock_gettime(CLOCK_REALTIME, &ts);
  PSE_CALL(pseColorPaletteExplorationContextCreate
    (cp, &excp, &exc));
  clock_gettime(CLOCK_REALTIME, &te);
#ifdef PRINT_PERFS
  fprintf(stdout, "Context creation time: %.3f ms\n", clock_elps_ms(&ts, &te));
#endif

  clock_gettime(CLOCK_REALTIME, &ts);
  PSE_CALL(pseColorPaletteExplorationContextInitFromValues
    (exc, &refs_colors));
  clock_gettime(CLOCK_REALTIME, &te);
#ifdef PRINT_PERFS
  fprintf(stdout, "Contexts initialization time: %.3f ms\n", clock_elps_ms(&ts, &te));
#endif

  smpls_colors.as.RGB.as.RGBr[0].R *= 0.8;
  smpls_colors.as.RGB.as.RGBr[0].G *= 0.8;
  smpls_colors.as.RGB.as.RGBr[0].B *= 0.8;

  clock_gettime(CLOCK_REALTIME, &ts);
  PSE_CALL(pseColorPaletteExplorationSolve(exc, &smpls_colors, 1, locked));
  clock_gettime(CLOCK_REALTIME, &te);
#ifdef PRINT_PERFS
  fprintf(stdout, "Elapsed time: %.3f ms\n", clock_elps_ms(&ts, &te));
#endif

  PSE_CALL(pseColorsConvert(&smpls_colors, &opts_colors));
  PSE_CALL(pseColorPaletteExplorationResultsRetreive
    (exc, &opts_colors, &results));

#ifdef PRINT_FINAL_PPOINTS
  fprintf(stdout, "Final colors =====================\n");
  printColors(&opts_colors);
#endif

#ifdef PRINT_PPM
  {
    struct pse_colors_t cvd_smpls_colors = PSE_COLORS_INVALID;
    struct pse_colors_t cvd_opts_colors = PSE_COLORS_INVALID;
    struct pse_colors_ref_t cvd_colors_ref = PSE_COLORS_REF_INVALID;
    struct pse_colors_ref_t smpls_colors_ref = PSE_COLORS_REF_INVALID;
    struct pse_colors_ref_t opts_colors_ref = PSE_COLORS_REF_INVALID;
    struct pse_colors_ref_t cvd_opts_colors_ref = PSE_COLORS_REF_INVALID;
    const struct pse_colors_t* clrs[5] = { NULL, NULL, NULL, NULL, NULL };
    clrs[0] = &refs_colors;
    clrs[1] = &cvd_smpls_colors;
    clrs[2] = &smpls_colors;
    clrs[3] = &opts_colors;
    clrs[4] = &cvd_opts_colors;
    PSE_CALL(pseColorsAllocate(alloc, PSE_COLOR_FORMAT_LABr, colors_count, &cvd_smpls_colors));
    PSE_CALL(pseColorsAllocate(alloc, PSE_COLOR_FORMAT_LABr, colors_count, &cvd_opts_colors));
    PSE_CALL(pseColorsRefMapColors(&smpls_colors_ref, &smpls_colors));
    PSE_CALL(pseColorsRefMapColors(&opts_colors_ref, &opts_colors));
    PSE_CALL(pseColorsRefMapColors(&cvd_colors_ref, &cvd_smpls_colors));
    PSE_CALL(pseColorsRefMapColors(&cvd_opts_colors_ref, &cvd_opts_colors));
    PSE_CALL(pseColorsCVDVariationApply(cvds[0], &smpls_colors_ref, &cvd_colors_ref));
    PSE_CALL(pseColorsCVDVariationApply(cvds[0], &opts_colors_ref, &cvd_opts_colors_ref));
    printPPM(5, clrs);
    PSE_CALL(pseColorsFree(alloc, &cvd_smpls_colors));
    PSE_CALL(pseColorsFree(alloc, &cvd_opts_colors));
  }
#endif

  PSE_CALL(pseConstrainedParameterSpaceExplorationContextRefSub(exc));
  PSE_CALL(pseColorPaletteDestroy(cp));
  PSE_CALL(pseDeviceDestroy(dev));

  PSE_CALL(pseColorsFree(alloc, &rand_colors));
  PSE_CALL(pseColorsFree(alloc, &refs_colors));
  PSE_CALL(pseColorsFree(alloc, &smpls_colors));
  PSE_CALL(pseColorsFree(alloc, &opts_colors));

  return 0;
}
