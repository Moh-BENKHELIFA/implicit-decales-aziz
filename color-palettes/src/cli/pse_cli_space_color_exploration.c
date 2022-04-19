#include <clt/space/color/pse_color_palette.h>
#include <clt/space/color/pse_color_palette_constraints.h>
#include <clt/space/color/pse_color_palette_exploration.h>
#include <clt/space/color/pse_color.h>
#include <clt/space/color/pse_color_vision_deficiencies.h>

#include "../stretchy_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_COLORS_COUNT  8
#define DEFAULT_CNSTR_WEIGHT  1.0f
#define DEFAULT_LOCK_RATIO    0.8f
#define DEFAULT_RAND_SEED     687
#define DEFAULT_DRIVER        PSE_LIB_NAME("pse-drv-eigen-ref")
#define DEFAULT_DF_EPSILON    PSE_REAL_SAFE_EPS
#define MAX_CVD_COUNT         4
#define MAX_LOCKED_COUNT      8

#define PSE_FATAL_IF_NOT(Test,Msg)                                             \
  if(!(Test)) { fprintf(stderr,"%s\n",(Msg)); assert(false); exit(-1); } (void)0

static PSE_INLINE void
HelpPrint()
{
  /* We use multiple calls to avoid warnings on string length in C90 */
  fprintf(stdout,
    "Space color exploration command line instruction. Options are:\n"
  );
  fprintf(stdout,
    "  --count=N                Set to N the number of colors of the palette (default is %d).\n"
    "  --seed=N                 Set to N the random seed used to generate the colors (default is %d).\n"
    "  --driver=FP              Set to FP the driver filepath (default is %s).\n",
    DEFAULT_COLORS_COUNT,
    DEFAULT_RAND_SEED,
    DEFAULT_DRIVER
  );
  fprintf(stdout,
    "  --cvd-d-rasche           Enable the deuteranopia CVD variation using the Rasche2005 algorithm.\n"
    "  --cvd-p-rasche           Enable the protanopia CVD variation using the Rasche2005 algorithm.\n"
    "  --cvd-d-troiano          Enable the deuteranopia CVD variation using the Troiano2008 algorithm.\n"
    "  --cvd-p-troiano          Enable the protanopia CVD variation using the Troiano2008 algorithm.\n"
  );
  fprintf(stdout,
    "  --cnstr-in-gamut[=w]     Enable constraint to force colors to stay in RGB gamut. It has a weight of 'w' (default to %f).\n"
    "  --cnstr-dist-cw=s[,w]    Enable constraint to keep distance component wise, in specific color 's'pace, with 's' one of {LAB,RGB,HSV,XYZ,LMS}. It has a weight of 'w' (default to %f).\n"
    "  --cnstr-energy[=w]       Enable constraint to minimize energy cost of colors on OLED screens. It has a weight of 'w' (default to %f).\n",
    PSE_COLOR_CONSTRAINT_INGAMUT_WEIGHT_DEFAULT,
    DEFAULT_CNSTR_WEIGHT,
    DEFAULT_CNSTR_WEIGHT
  );
  fprintf(stdout,
    "  --lock=i[,r]             Lock 'i'th value and multiply it by 'r' (default to %f) before exploration.\n"
    "                           No more than %d colors can be locked.\n",
    DEFAULT_LOCK_RATIO,
    MAX_LOCKED_COUNT
  );
  fprintf(stdout,
    "  --dfeps=e                Epsilon used during the automatic numerical differential computation (default to %f).\n",
    DEFAULT_DF_EPSILON
  );
  fprintf(stdout,
    "  --log                    Enable stdout logger.\n"
    "  --help                   Display this message.\n"
  );
}

struct cnstr_options_t {
  pse_color_space_t space;
  pse_real_t weight;
};

struct options_t {
  size_t colors_count;
  int rand_seed;
  const char* driver_filepath;
  bool log;
  pse_real_t df_eps;

  size_t locked_count;
  float locked_ratio_default;
  pse_color_idx_t locked[MAX_LOCKED_COUNT];
  float locked_ratio[MAX_LOCKED_COUNT];

  size_t cvds_count;
  enum pse_color_vision_deficiency_variation_t cvds[MAX_CVD_COUNT];

  struct cnstr_options_t cnstr_in_gamut;
  struct cnstr_options_t cnstr_energy;
  struct cnstr_options_t* cnstr_translation_only;
};

#define CNSTR_OPTIONS_INVALID_                                                 \
  { PSE_COLOR_SPACE_INVALID_, 0 }

#define OPTIONS_DEFAULT_                                                       \
  { DEFAULT_COLORS_COUNT, DEFAULT_RAND_SEED, DEFAULT_DRIVER, false,            \
    DEFAULT_DF_EPSILON,                                                        \
    0, DEFAULT_LOCK_RATIO, { 0 }, { DEFAULT_LOCK_RATIO },                      \
    0, { PSE_CVD_DEUTERANOPIA_Troiano2008 },                                   \
    CNSTR_OPTIONS_INVALID_, CNSTR_OPTIONS_INVALID_, NULL }

static pse_color_space_t
ColorSpaceFromCString
  (const char* name)
{
  if( strncmp(name, "LAB", 3) == 0 ) {
    return PSE_COLOR_SPACE_LAB;
  } else if( strncmp(name, "RGB", 3) == 0 ) {
    return PSE_COLOR_SPACE_RGB;
  } else if( strncmp(name, "HSV", 3) == 0 ) {
    return PSE_COLOR_SPACE_HSV;
  } else if( strncmp(name, "XYZ", 3) == 0 ) {
    return PSE_COLOR_SPACE_XYZ;
  } else if( strncmp(name, "LMS", 3) == 0 ) {
    return PSE_COLOR_SPACE_Cat02LMS;
  }
  return PSE_COLOR_SPACE_INVALID;
}

static void
ParametersParse
  (int argc,
   char* argv[],
   struct options_t* opts)
{
  size_t i;
  for(i = 1; i < (size_t)argc; ++i) {
    /******************/
    if( strncmp(argv[i], "--count=", 8) == 0 ) {
      int count;
      PSE_FATAL_IF_NOT
        (strlen(argv[i]) > 8,
         "Bad --count option format.");
      sscanf(&argv[i][8],"%d", &count);
      PSE_FATAL_IF_NOT
        (count > 0,
         "The number of colors must be greater than 0.");
      opts->colors_count = PSE_MAX(1, count);

    /******************/
    } else if( strncmp(argv[i], "--seed=", 7) == 0 ) {
      PSE_FATAL_IF_NOT
        (strlen(argv[i]) > 7,
         "Bad --seed option format.");
      sscanf(&argv[i][7],"%d", &opts->rand_seed);

    /******************/
    } else if( strncmp(argv[i], "--driver=", 9) == 0 ) {
      PSE_FATAL_IF_NOT
        (strlen(argv[i]) > 9,
         "Bad --driver option format.");
      opts->driver_filepath = &argv[i][9];

    /******************/
    } else if( strcmp(argv[i], "--cvd-d-rasche") == 0 ) {
      PSE_FATAL_IF_NOT
        (opts->cvds_count < MAX_CVD_COUNT,
         "Max number of CVD reached.");
      opts->cvds[opts->cvds_count++] = PSE_CVD_DEUTERANOPIA_Rasche2005;

    /******************/
    } else if( strcmp(argv[i], "--cvd-p-rasche") == 0 ) {
      PSE_FATAL_IF_NOT
        (opts->cvds_count < MAX_CVD_COUNT,
         "Max number of CVD reached.");
      opts->cvds[opts->cvds_count++] = PSE_CVD_PROTANOPIA_Rasche2005;

    /******************/
    } else if( strcmp(argv[i], "--cvd-d-troiano") == 0 ) {
      PSE_FATAL_IF_NOT
        (opts->cvds_count < MAX_CVD_COUNT,
         "Max number of CVD reached.");
      opts->cvds[opts->cvds_count++] = PSE_CVD_DEUTERANOPIA_Troiano2008;

    /******************/
    } else if( strcmp(argv[i], "--cvd-p-troiano") == 0 ) {
      PSE_FATAL_IF_NOT
        (opts->cvds_count < MAX_CVD_COUNT,
         "Max number of CVD reached.");
      opts->cvds[opts->cvds_count++] = PSE_CVD_PROTANOPIA_Troiano2008;

    /******************/
    } else if( strncmp(argv[i], "--cnstr-in-gamut", 16) == 0 ) {
      const size_t optlen = strlen(argv[i]);
      if( optlen == 16 ) {
        opts->cnstr_in_gamut.weight = PSE_COLOR_CONSTRAINT_INGAMUT_WEIGHT_DEFAULT;
      } else {
        PSE_FATAL_IF_NOT
          (optlen >= 18,
           "Bad --cnstr-in-gamut option format");
        sscanf(&argv[i][17],"%lf", &opts->cnstr_in_gamut.weight);
      }
      /* Cannot be another color space */
      opts->cnstr_in_gamut.space = PSE_COLOR_SPACE_RGB;

    /******************/
    } else if( strncmp(argv[i], "--cnstr-dist-cw", 15) == 0 ) {
      const size_t optlen = strlen(argv[i]);
      struct cnstr_options_t cnstr_opts = CNSTR_OPTIONS_INVALID_;
      char space[4] = {0};
      PSE_FATAL_IF_NOT
        (optlen >= 19,
         "Bad --cnstr-dist-cw option format");
      if( optlen == 19 ) {
        sscanf(&argv[i][16],"%3s", space);
        cnstr_opts.weight = DEFAULT_CNSTR_WEIGHT;
      } else {
        PSE_FATAL_IF_NOT
          (optlen >= 21,
           "Bad --cnstr-dist-cw option format");
        sscanf(&argv[i][16],"%3s,%lf", space, &cnstr_opts.weight);
      }
      PSE_FATAL_IF_NOT
        (strlen(space) == 3,
         "Bad --cnstr-dist-cw color space");
      cnstr_opts.space = ColorSpaceFromCString(space);
      PSE_FATAL_IF_NOT
        (cnstr_opts.space != PSE_COLOR_SPACE_INVALID,
         "Bad --cnstr-dist-cw color space");
      sb_push(opts->cnstr_translation_only, cnstr_opts);

    /******************/
    } else if( strcmp(argv[i], "--cnstr-energy") == 0 ) {
      const size_t optlen = strlen(argv[i]);
      if( optlen == 14 ) {
        opts->cnstr_energy.weight = DEFAULT_CNSTR_WEIGHT;
      } else {
        PSE_FATAL_IF_NOT
          (optlen >= 16,
           "Bad --cnstr-energy option format");
        sscanf(&argv[i][15],"%lf", &opts->cnstr_energy.weight);
      }
      /* Cannot be another color space */
      opts->cnstr_energy.space = PSE_COLOR_SPACE_RGB;

    /******************/
    } else if( strncmp(argv[i], "--lock=", 7) == 0 ) {
      int idx;
      PSE_FATAL_IF_NOT
        (strlen(argv[i]) > 7,
         "Bad --lock option format");
      PSE_FATAL_IF_NOT
        (opts->locked_count < MAX_LOCKED_COUNT,
         "Max number of locked color reached.");
      opts->locked_ratio[opts->locked_count] = opts->locked_ratio_default;
      sscanf(&argv[i][7],"%d,%f", &idx, &opts->locked_ratio[opts->locked_count]);
      opts->locked[opts->locked_count++] = idx;

    /******************/
    } else if( strncmp(argv[i], "--dfeps=", 8) == 0 ) {
      const size_t optlen = strlen(argv[i]);
      PSE_FATAL_IF_NOT
        (optlen > 8,
         "No value given to --dfeps option");
      sscanf(&argv[i][8],"%lf", &opts->df_eps);

    /******************/
    } else if( strcmp(argv[i], "--log") == 0 ) {
      opts->log = true;

    /******************/
    } else if( strcmp(argv[i], "--help") == 0 ) {
      HelpPrint();
      exit(0);

    /******************/
    } else {
      fprintf(stderr, "Unknwon option: %s\n", argv[i]);
      exit(-1);
    }
  }
}

static PSE_INLINE void
ColorsAsPPMPrint
  (const size_t count,
   const struct pse_colors_t* colors[])
{
  FILE* f = stdout;
  size_t i, j, colors_count;
  struct pse_color_t rgb = PSE_COLOR_INVALID;
  pseColorFormatSet(&rgb, PSE_COLOR_FORMAT_RGBui8);

  colors_count = colors[0]->as.any.count;
  fprintf(f,
    "P3\n"     /* Magick number: P3 -> ASCII portable RGB color image */
    "%d %d\n"  /* Image width and height*/
    "255 \n",  /* Max color value. We use the maximum to have more precision */
    (int)colors_count, (int)count);
  for(i = 0; i < count; ++i) {
    assert(colors[i]->as.any.count == colors_count);
    for(j = 0; j < colors_count; ++j) {
      PSE_CALL(pseColorsExtractAt(colors[i], j, &rgb));
      fprintf(f, "%d %d %d\t",
        rgb.as.RGB.as.RGBui8.R,
        rgb.as.RGB.as.RGBui8.G,
        rgb.as.RGB.as.RGBui8.B);
    }
    fprintf(f,"\n");
  }
}

int main(int argc, char* argv[])
{
  enum pse_res_t res = RES_OK;
  struct pse_allocator_t* alloc = &PSE_ALLOCATOR_DEFAULT;
  struct pse_logger_t* logger = NULL;
  struct pse_device_params_t devp = PSE_DEVICE_PARAMS_NULL;
  struct pse_device_t* dev = NULL;

  struct pse_color_palette_params_t cpp = PSE_COLOR_PALETTE_PARAMS_NULL;
  struct pse_color_palette_t* cp = NULL;

  struct pse_color_palette_exploration_ctxt_params_t excp = PSE_COLOR_PALETTE_EXPLORATION_CTXT_PARAMS_DEFAULT;
  struct pse_cpspace_exploration_ctxt_t* exc = NULL;
  struct pse_cpspace_exploration_extra_results_t results = PSE_CPSPACE_EXPLORATION_EXTRA_RESULTS_NULL;

  struct pse_color_space_constraint_params_t cscp = PSE_COLOR_SPACE_CONSTRAINT_PARAMS_NULL;
  pse_clt_ppoint_variation_uid_t cvds_uid[MAX_CVD_COUNT] = {
    PSE_CLT_PPOINT_VARIATION_UID_INVALID_,
    PSE_CLT_PPOINT_VARIATION_UID_INVALID_,
    PSE_CLT_PPOINT_VARIATION_UID_INVALID_,
    PSE_CLT_PPOINT_VARIATION_UID_INVALID_
  };

  struct options_t opts = OPTIONS_DEFAULT_;

  struct pse_colors_t refs = PSE_COLORS_INVALID;
  struct pse_colors_t colors = PSE_COLORS_INVALID;

  size_t i;

  /* Parse parameters to fill the options */
  ParametersParse(argc, argv, &opts);

  /* Enable logger if asked */
  if( opts.log )
    logger = &PSE_LOGGER_STDOUT;

  /* Allocate colors buffer and initialize it with random colors. */
  srand(opts.rand_seed);
  PSE_CALL(pseColorsAllocate
    (alloc, PSE_COLOR_FORMAT_RGBui8, opts.colors_count, &colors));
  for(i = 0; i < opts.colors_count; ++i) {
    struct pse_color_t clr = PSE_COLOR_INVALID;
    clr.space = PSE_COLOR_SPACE_RGB;
    clr.as.RGB.type = PSE_COLOR_TYPE_RGBui8;
    clr.as.RGB.as.RGBui8.R = (uint8_t)(((float)rand() / (float)RAND_MAX) * 255.0);
    clr.as.RGB.as.RGBui8.G = (uint8_t)(((float)rand() / (float)RAND_MAX) * 255.0);
    clr.as.RGB.as.RGBui8.B = (uint8_t)(((float)rand() / (float)RAND_MAX) * 255.0);
    PSE_CALL(pseColorsSetAt(&colors, i, &clr));
  }
  /* Make a copy of the colors to display them at the end */
  PSE_CALL(pseColorsAllocate
    (alloc, PSE_COLOR_FORMAT_RGBui8, opts.colors_count, &refs));
  PSE_CALL(pseColorsConvert(&colors, &refs));

  /* Create the device */
  devp.allocator = alloc;
  devp.logger = logger;
  devp.backend_drv_filepath = opts.driver_filepath;
  PSE_CALL(pseDeviceCreate(&devp, &dev));

  /* Create a color palette */
  cpp.alloc = alloc;
  cpp.dev = dev;
  PSE_CALL(pseColorPaletteCreate(&cpp, &cp));
  PSE_CALL(pseColorPalettePointCountSet(cp, opts.colors_count));

  /* If asked, add CVD variations */
  if( opts.cvds_count > 0 ) {
    PSE_CALL(pseColorPaletteVariationsAddForCVD
      (cp, opts.cvds_count, opts.cvds, cvds_uid));
  }

  /* If asked, add the constraints */
  if( opts.cnstr_in_gamut.weight > 0 ) {
    pse_color_palette_constraint_id_t id =
      PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID;
    cscp.space = PSE_COLOR_SPACE_RGB;
    cscp.components = PSE_COLOR_COMPONENTS_ALL;
    cscp.colors_ref = PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_ALL;
    cscp.variations_count = opts.cvds_count;
    cscp.variations = cvds_uid;
    cscp.weight = opts.cnstr_in_gamut.weight;
    PSE_CALL(pseColorPaletteConstraintInGamutAdd(cp, &cscp, &id));
  }
  for(i = 0; i < sb_count(opts.cnstr_translation_only); ++i) {
    struct cnstr_options_t* cnstr_opts = &opts.cnstr_translation_only[i];
    if( cnstr_opts->weight > 0 ) {
      pse_color_palette_constraint_id_t id =
        PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID;
      struct pse_color_distance_params_t dist_params =
        PSE_COLOR_DISTANCE_PARAMS_DEFAULT;
      cscp.space = cnstr_opts->space;
      cscp.components = PSE_COLOR_COMPONENTS_ALL;
      cscp.colors_ref = PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_ALL;
      cscp.variations_count = 0;
      cscp.variations = NULL;
      cscp.weight = cnstr_opts->weight;
      PSE_CALL(pseColorPaletteConstraintDistancePerComponentAdd
        (cp, &cscp, &dist_params, &id));
    }
  }
  if( opts.cnstr_energy.weight > 0 ) {
    pse_color_palette_constraint_id_t id =
      PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID;
    cscp.space = PSE_COLOR_SPACE_RGB;
    cscp.components = PSE_COLOR_COMPONENTS_ALL;
    cscp.colors_ref = PSE_COLOR_SPACE_CONSTRAINT_COLORS_REF_ALL;
    cscp.variations_count = opts.cvds_count;
    cscp.variations = cvds_uid;
    cscp.weight = opts.cnstr_energy.weight;
    PSE_CALL(pseColorPaletteConstraintOLEDScreenEnergyConsumptionAdd(cp, &cscp, &id));
  }

  /* Create the exploration context. For some drivers, this call will trigger
   * internal precomputations. */
  excp.options.auto_df_epsilon = opts.df_eps;
  PSE_CALL(pseColorPaletteExplorationContextCreate(cp, &excp, &exc));

  /* Initialize the contexts of the costs functors using the initial colors. */
  PSE_CALL(pseColorPaletteExplorationContextInitFromValues(exc, &refs));

  /* Now change the locked values using the ratios, in order to produce a real
   * exploration. */
  for(i = 0; i < opts.locked_count; ++i) {
    const pse_color_idx_t idx = opts.locked[i];
    struct pse_color_t clr = PSE_COLOR_INVALID;
    pseColorFormatSet(&clr, PSE_COLOR_FORMAT_LABr);
    PSE_CALL(pseColorsExtractAt(&colors, idx, &clr));
    clr.as.Lab.as.LABr.L *= opts.locked_ratio[i];
    PSE_CALL(pseColorsSetAt(&colors, idx, &clr));
  }

  /* Launch the exploration using the modified colors as samples */
  res = pseColorPaletteExplorationSolve
    (exc, &colors, opts.locked_count, opts.locked);
  assert(res == RES_OK || res == RES_NOT_FOUND || res == RES_NOT_CONVERGED);
  (void)res;

  /* Retreive the results of the exploration */
  if( res == RES_OK ) {
    PSE_CALL(pseColorPaletteExplorationResultsRetreive(exc, &colors, &results));
  }

  { /* Print the associated PPM */
    const struct pse_colors_t* clrs[32];
    size_t clrs_count = 0;
    clrs[clrs_count++] = &refs;
    clrs[clrs_count++] = &colors;
    ColorsAsPPMPrint(clrs_count, clrs);
  }

  /* Clean memory before leaving */
  PSE_CALL(pseConstrainedParameterSpaceExplorationContextRefSub(exc));
  PSE_CALL(pseColorPaletteDestroy(cp));
  PSE_CALL(pseDeviceDestroy(dev));
  PSE_CALL(pseColorsFree(alloc, &colors));
  PSE_CALL(pseColorsFree(alloc, &refs));

  sb_free(opts.cnstr_translation_only);

  return 0;
}
