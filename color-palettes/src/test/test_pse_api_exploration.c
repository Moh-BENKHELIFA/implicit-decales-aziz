#include "test_utils.h"

#include <pse.h>

#include <stdio.h>
#include <stdlib.h>

enum ColorSpace {
  ColorSpace_RGB,
  ColorSpace_HSV,
  ColorSpace_Lab
};

enum ParameterSpace {
  ParameterSpace_Color = 0x1000,
  ParameterSpace_Color_RGB = ParameterSpace_Color + ColorSpace_RGB + 1,
  ParameterSpace_Color_HSV = ParameterSpace_Color + ColorSpace_HSV + 1,
  ParameterSpace_Color_Lab = ParameterSpace_Color + ColorSpace_Lab + 1
};

enum cost_function_uid {
  TEST_CF_DIST,
  TEST_CF_CLAMPED_DIST,
  TEST_CF_REF_DIST
};

typedef pse_real_t Real;

struct ColorRGB {
  Real R, G, B;
};
struct ColorHSV {
  Real H, S, V;
};
struct ColorLab {
  Real L, a, b;
};

struct Color {
  enum ColorSpace space;
  union {
    struct ColorRGB rgb;
    struct ColorHSV hsv;
    struct ColorLab lab;
  } as;
};

#define COLOR_BLACK_ { ColorSpace_RGB, { { 0, 0, 0 } } }

struct ClampContext {
  pse_real_t max_cost;
};

struct ReferenceContext {
  struct Color ref;
};

#define CLAMP_CONTEXT_DEFAULT_  {0}
static const struct ClampContext CLAMP_CONTEXT_DEFAULT =
  CLAMP_CONTEXT_DEFAULT_;

static inline enum pse_res_t
initReferenceContexts
  (pse_clt_cost_func_config_t user_cfunc_config,
   const pse_clt_cost_func_uid_t user_cfunc_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps,
   pse_clt_cost_func_ctxt_init_params_t user_init_params)
{
  size_t i;
  assert(user_cfunc_uid == TEST_CF_REF_DIST);
  (void)user_cfunc_config, (void)user_cfunc_uid;
  (void)eval_ctxt;
  (void)user_init_params;

  for(i = 0; i < eval_relshps->count; ++i) {
    struct ReferenceContext* rc =
      (struct ReferenceContext*)eval_relshps->ctxts[i];
    rc->ref.space = ColorSpace_RGB;
    rc->ref.as.rgb.R = (Real)(rand() % 256) / 255.0;
    rc->ref.as.rgb.G = (Real)(rand() % 256) / 255.0;
    rc->ref.as.rgb.B = (Real)(rand() % 256) / 255.0;
  }
  return RES_OK;
}

static inline struct Color
convert
  (const struct Color* a,
   enum ColorSpace to_space)
{
  (void)to_space;
  return *a;
}

enum pse_res_t
convertColor
  (void* user_data,
   const pse_clt_pspace_uid_t from,
   const pse_clt_pspace_uid_t to,
   const size_t values_count,
   const pse_real_t* values_from,
   pse_real_t* values_to)
{
  enum pse_res_t res = RES_OK;
  size_t i;
  (void)user_data;
  if( from == to ) {
    for(i = 0; i < values_count; ++i) {
      values_to[i*3+0] = values_from[i*3+0];
      values_to[i*3+1] = values_from[i*3+1];
      values_to[i*3+2] = values_from[i*3+2];
    }
  }
  switch(from) {
    case ColorSpace_RGB: {
      assert(to == ColorSpace_Lab);
      for(i = 0; i < values_count; ++i) {
        values_to[i*3+0] = values_from[i*3+0] * 666.0;
        values_to[i*3+1] = values_from[i*3+1] * 666.0;
        values_to[i*3+2] = values_from[i*3+2] * 666.0;
      }
    } break;
    case ColorSpace_Lab: {
      assert(to == ColorSpace_RGB);
      for(i = 0; i < values_count; ++i) {
        values_to[i*3+0] = values_from[i*3+0] / 666.0;
        values_to[i*3+1] = values_from[i*3+1] / 666.0;
        values_to[i*3+2] = values_from[i*3+2] / 666.0;
      }
    } break;
    default: assert(false); res = RES_NOT_SUPPORTED;
  }
  return res;
}

static PSE_FINLINE enum pse_res_t
computeColorDistance
  (const struct Color* a,
   const struct Color* b)
{
  /* Euclidean distance */
  pse_real_t R = 0;
  pse_real_t G = 0;
  pse_real_t B = 0;
  if( a->space == b->space ) {
    if( a->space == ColorSpace_RGB ) {
      R = b->as.rgb.R - a->as.rgb.R;
      G = b->as.rgb.G - a->as.rgb.G;
      B = b->as.rgb.B - a->as.rgb.B;
    } else {
      struct Color ca = convert(a, ColorSpace_RGB);
      struct Color cb = convert(b, ColorSpace_RGB);
      R = cb.as.rgb.R - ca.as.rgb.R;
      G = cb.as.rgb.G - ca.as.rgb.G;
      B = cb.as.rgb.B - ca.as.rgb.B;
    }
  } else if( a->space == ColorSpace_RGB ) {
    struct Color cb = convert(b, ColorSpace_RGB);
    R = cb.as.rgb.R - a->as.rgb.R;
    G = cb.as.rgb.G - a->as.rgb.G;
    B = cb.as.rgb.B - a->as.rgb.B;
  } else if( b->space == ColorSpace_RGB ) {
    struct Color ca = convert(a, ColorSpace_RGB);
    R = b->as.rgb.R - ca.as.rgb.R;
    G = b->as.rgb.G - ca.as.rgb.G;
    B = b->as.rgb.B - ca.as.rgb.B;
  } else {
    struct Color ca = convert(a, ColorSpace_RGB);
    struct Color cb = convert(b, ColorSpace_RGB);
    R = cb.as.rgb.R - ca.as.rgb.R;
    G = cb.as.rgb.G - ca.as.rgb.G;
    B = cb.as.rgb.B - ca.as.rgb.B;
  }
  return PSE_REAL_SQRT(R*R + G*G + B*B) * 255;
}

static inline enum pse_res_t
computeColorDistanceCb
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  size_t i;
  struct Color c1, c2;
  pse_ppoint_id_t ppid1, ppid2;
  (void)eval_ctxt;

  for(i = 0; i < eval_relshps->count; ++i) {
    assert(eval_relshps->data[i]->ppoints_count == 2);
    ppid1 = eval_relshps->data[i]->ppoints[0];
    ppid2 = eval_relshps->data[i]->ppoints[1];
    c1.space = ColorSpace_Lab;
    c1.as.lab.L = eval_coords->coords[ppid1*3+0];
    c1.as.lab.a = eval_coords->coords[ppid1*3+1];
    c1.as.lab.b = eval_coords->coords[ppid1*3+2];
    c2.space = ColorSpace_Lab;
    c2.as.lab.L = eval_coords->coords[ppid2*3+0];
    c2.as.lab.a = eval_coords->coords[ppid2*3+1];
    c2.as.lab.b = eval_coords->coords[ppid2*3+2];
    costs[i] = computeColorDistance(&c1, &c2);
  }

  return RES_OK;
}

static inline enum pse_res_t
computeColorClampedDistanceCb
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  size_t i;
  (void)eval_ctxt, (void)eval_coords;
  for(i = 0; i < eval_relshps->count; ++i) {
    struct ClampContext* ctxt = (struct ClampContext*)eval_relshps->ctxts[i];
    costs[i] = PSE_MIN(255.0/*costs[i]*/, ctxt->max_cost);
  }
  return RES_OK;
}

static inline enum pse_res_t
computeColorRefDistanceCb
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  size_t i;
  struct Color c;
  pse_ppoint_id_t ppid;
  (void)eval_ctxt;

  for(i = 0; i < eval_relshps->count; ++i) {
    struct ReferenceContext* ctxt =
      (struct ReferenceContext*)eval_relshps->ctxts[i];
    assert(eval_relshps->data[i]->ppoints_count == 1);
    ppid = eval_relshps->data[i]->ppoints[0];
    c.space = ColorSpace_Lab;
    c.as.lab.L = eval_coords->coords[ppid*3+0];
    c.as.lab.a = eval_coords->coords[ppid*3+1];
    c.as.lab.b = eval_coords->coords[ppid*3+2];
    costs[i] = computeColorDistance(&c, &ctxt->ref);
  }

  return RES_OK;
}

enum pse_res_t
getAttribs
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   void* attrib_values)
{
  const struct Color* colors = (const struct Color*)ctxt;
  size_t i;
  (void)as_type;

  switch(attrib) {
    case PSE_POINT_ATTRIB_COORDINATES: {
      pse_real_t* out = (pse_real_t*)attrib_values;
      assert(as_type == PSE_TYPE_REAL);
      for(i = 0; i < count; ++i) {
        const pse_ppoint_id_t ppid = values_idx[i];
        struct Color curr = colors[ppid];
        if( curr.space != ColorSpace_Lab )
          curr = convert(&curr, ColorSpace_Lab);
        out[ppid*3+0] = (pse_real_t)curr.as.lab.L;
        out[ppid*3+1] = (pse_real_t)curr.as.lab.a;
        out[ppid*3+2] = (pse_real_t)curr.as.lab.b;
      }
    } break;
    case PSE_POINT_ATTRIB_LOCK_STATUS: {
      uint8_t* out = (uint8_t*)attrib_values;
      assert(as_type == PSE_TYPE_BOOL_8);
      for(i = 0; i < count; ++i) {
        *out = (i == 0) ? 1 : 0;
      }
    } break;
    default: assert(false);
  }
  return RES_OK;
}

enum pse_res_t
setAttribs
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   const void* attrib_values)
{
  struct Color* colors = (struct Color*)ctxt;
  size_t i;
  (void)as_type;

  switch(attrib) {
    case PSE_POINT_ATTRIB_COORDINATES: {
      const pse_real_t* in = (const pse_real_t*)attrib_values;
      assert(as_type == PSE_TYPE_REAL);
      for(i = 0; i < count; ++i) {
        const pse_ppoint_id_t ppid = values_idx[i];
        colors[ppid].space = ColorSpace_Lab;
        colors[ppid].as.lab.L = (Real)in[i*3+0];
        colors[ppid].as.lab.a = (Real)in[i*3+1];
        colors[ppid].as.lab.b = (Real)in[i*3+2];
      }
    } break;
    default: assert(false);
  }
  return RES_OK;
}

void
printColors
  (const size_t count,
   const struct Color* colors)
{
  static const char* space_name[] = { "RGB", "HSV", "Lab" };
  size_t i;
  for(i = 0; i < count; ++i) {
    fprintf(stdout, "Color %d: %s{%f,%f,%f}\n",
      (int)i, space_name[colors->space],
      /* Don't care about the type as RGB, HSV and Lab are all made of 3 floats
       * in memory */
      colors[i].as.rgb.R,
      colors[i].as.rgb.G,
      colors[i].as.rgb.B);
  }
}

int main()
{
  struct pse_device_params_t devp = PSE_DEVICE_PARAMS_NULL;
  pse_clt_pspace_uid_t psps_uid[2] = { PSE_CLT_PSPACE_UID_INVALID_, PSE_CLT_PSPACE_UID_INVALID_ };
  struct pse_pspace_params_t psps[2] = { PSE_PSPACE_PARAMS_NULL_, PSE_PSPACE_PARAMS_NULL_ };
  struct pse_cpspace_params_t cpsp = PSE_CPSPACE_PARAMS_NULL;
  struct pse_cpspace_values_data_t data = PSE_CPSPACE_VALUES_DATA_NULL;
  struct pse_cpspace_exploration_ctxt_params_t ctxtp = PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL;
  struct pse_cpspace_exploration_extra_results_t results = PSE_CPSPACE_EXPLORATION_EXTRA_RESULTS_NULL;
  struct pse_cpspace_exploration_samples_t smpls = PSE_CPSPACE_EXPLORATION_SAMPLES_NULL;
  struct pse_relshp_cost_func_params_t ccfps[] = {
    { TEST_CF_CLAMPED_DIST, ColorSpace_RGB, computeColorClampedDistanceCb, NULL,
      PSE_COST_ARITY_MODE_PER_RELATIONSHIP, 1,
      { TEST_CF_CLAMPED_DIST, sizeof(struct ClampContext), 0, &CLAMP_CONTEXT_DEFAULT,
        NULL, NULL, NULL },
      NULL, NULL, NULL
    },
    { TEST_CF_DIST, ColorSpace_RGB, computeColorDistanceCb, NULL,
      PSE_COST_ARITY_MODE_PER_RELATIONSHIP, 1,
      { TEST_CF_DIST, 0, 0, NULL, NULL, NULL, NULL }, /* No context */
      NULL, NULL, NULL
    },
    { TEST_CF_REF_DIST, ColorSpace_RGB, computeColorRefDistanceCb, NULL,
      PSE_COST_ARITY_MODE_PER_RELATIONSHIP, 1,
      { TEST_CF_REF_DIST, sizeof(struct ReferenceContext), 0, NULL, NULL, NULL, NULL },
      initReferenceContexts, NULL, NULL,
    }
  };
  pse_relshp_cost_func_id_t ccfids[] = {
    PSE_RELSHP_COST_FUNC_ID_INVALID_,
    PSE_RELSHP_COST_FUNC_ID_INVALID_,
    PSE_RELSHP_COST_FUNC_ID_INVALID_
  };
  struct pse_ppoint_params_t ppps[] = {
    PSE_PPOINT_PARAMS_NULL_,
    PSE_PPOINT_PARAMS_NULL_,
    PSE_PPOINT_PARAMS_NULL_
  };
  pse_ppoint_id_t ppids[] = {
    PSE_PPOINT_ID_INVALID_,
    PSE_PPOINT_ID_INVALID_,
    PSE_PPOINT_ID_INVALID_
  };
  pse_ppoint_id_t pppairs[][2] = {
    { PSE_PPOINT_ID_INVALID_, PSE_PPOINT_ID_INVALID_ },
    { PSE_PPOINT_ID_INVALID_, PSE_PPOINT_ID_INVALID_ },
    { PSE_PPOINT_ID_INVALID_, PSE_PPOINT_ID_INVALID_ }
  };
  struct pse_cpspace_relshp_params_t rps[] = {
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_,
    PSE_CPSPACE_RELSHP_PARAMS_NULL_
  };
  pse_relshp_id_t rids[] = {
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_,
    PSE_RELSHP_ID_INVALID_
  };
  struct pse_pspace_point_attrib_component_t coords_components[] = {
    {PSE_TYPE_FLOAT},
    {PSE_TYPE_FLOAT},
    {PSE_TYPE_FLOAT}
  };
  struct pse_pspace_point_attrib_component_t lock_components[] = {
    {PSE_TYPE_BOOL_8}
  };
  struct Color smpls_colors[] = {
    { ColorSpace_RGB, { { 1.0f, 0.0f, 0.0f } } },
    { ColorSpace_RGB, { { 0.0f, 1.0f, 0.0f } } },
    { ColorSpace_RGB, { { 0.0f, 0.0f, 1.0f } } }
  };
  struct Color opts_colors[] = {
    COLOR_BLACK_,
    COLOR_BLACK_,
    COLOR_BLACK_
  };

  struct pse_device_t* dev = NULL;
  struct pse_cpspace_t* cps = NULL;
  struct pse_cpspace_values_t* valssmpls = NULL;
  struct pse_cpspace_values_t* valsopts = NULL;
  struct pse_cpspace_exploration_ctxt_t* ctxt = NULL;
  struct pse_cpspace_values_data_t* optdata = NULL;

  size_t i;

  /* Create a palex device */
  devp.backend_drv_filepath = PSE_LIB_NAME("pse-drv-eigen-ref");
  CHECK(pseDeviceCreate(&devp, &dev), RES_OK);

  /* Fill parameters of the parameter space we want */
  psps_uid[0] = ColorSpace_RGB;
  psps[0].ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components_count = 3;
  psps[0].ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components = coords_components;
  psps[0].ppoint_params.attribs[PSE_POINT_ATTRIB_LOCK_STATUS].components_count = 1;
  psps[0].ppoint_params.attribs[PSE_POINT_ATTRIB_LOCK_STATUS].components = lock_components;
  psps_uid[1] = ColorSpace_Lab;
  psps[1].ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components_count = 3;
  psps[1].ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components = coords_components;
  psps[1].ppoint_params.attribs[PSE_POINT_ATTRIB_LOCK_STATUS].components_count = 1;
  psps[1].ppoint_params.attribs[PSE_POINT_ATTRIB_LOCK_STATUS].components = lock_components;

  /* Create a constrained parameter space */
  CHECK(pseConstrainedParameterSpaceCreate(dev, &cpsp, &cps), RES_OK);

  /* Declare the parameter spaces */
  CHECK(pseConstrainedParameterSpaceParameterSpacesDeclare
    (cps, 2, psps_uid, psps), RES_OK);

  /* Create funcs to make them available later for the constrained parameter
   * spaces. */
  CHECK(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 3, ccfps, ccfids), RES_OK);

  /* Fill parameters of the parametric point slots we want */
  CHECK(pseConstrainedParameterSpaceParametricPointsAdd
    (cps, 3, ppps, ppids), RES_OK);

  /* Create all relationships of the CPS */
  /* Global constraint */
  rps[0].kind = PSE_RELSHP_KIND_EXCLUSIVE;
  rps[0].cnstrs.funcs_count = 1;
  rps[0].cnstrs.funcs = &ccfids[0];
  /* Unary constraints */
  for(i = 0; i < 3; ++i) {
    struct pse_cpspace_relshp_params_t* rp = &rps[1+i];
    rp->kind = PSE_RELSHP_KIND_INCLUSIVE;
    rp->ppoints_count = 1;
    rp->ppoints_id = &ppids[i];
    rp->cnstrs.funcs_count = 1;
    rp->cnstrs.funcs = &ccfids[2];
  }
  /* Binary constraints */
  pppairs[0][0] = ppids[0];
  pppairs[0][1] = ppids[1];
  pppairs[1][0] = ppids[0];
  pppairs[1][1] = ppids[2];
  pppairs[2][0] = ppids[1];
  pppairs[2][1] = ppids[2];
  for(i = 0; i < 3; ++i) {
    struct pse_cpspace_relshp_params_t* rp = &rps[4+i];
    rp->kind = PSE_RELSHP_KIND_INCLUSIVE;
    rp->ppoints_count = 2;
    rp->ppoints_id = pppairs[i];
    rp->cnstrs.funcs_count = 1;
    rp->cnstrs.funcs = &ccfids[1];
  }
  CHECK(pseConstrainedParameterSpaceRelationshipsAdd
    (cps, PSE_CLT_RELSHPS_GROUP_UID_INVALID, 7, rps, rids), RES_OK);

  data.pspace = psps_uid[1];
  data.storage = PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_GLOBAL;
  data.as.global.accessors.ctxt = smpls_colors;
  data.as.global.accessors.get = getAttribs;
  data.as.global.accessors.set = setAttribs;
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, &data, &valssmpls), RES_OK);

  data.as.global.accessors.ctxt = opts_colors;
  CHECK(pseConstrainedParameterSpaceValuesCreate(cps, &data, &valsopts), RES_OK);

  ctxtp.pspace.convert = convertColor;
  ctxtp.pspace.explore_in = psps_uid[1];
  CHECK(pseConstrainedParameterSpaceExplorationContextCreate
    (cps, &ctxtp, &ctxt), RES_OK);

  CHECK(pseConstrainedParameterSpaceExplorationRelationshipsAllContextsInit
    (ctxt, NULL), RES_OK);

  smpls.values = valssmpls;
  CHECK(pseConstrainedParameterSpaceExplorationSolve(ctxt, &smpls), RES_OK);

  CHECK(pseConstrainedParameterSpaceExplorationLastResultsRetreive
    (ctxt, valsopts, &results), RES_OK);
  CHECK(pseConstrainedParameterSpaceValuesLock
    (valsopts, &PSE_CPSPACE_VALUES_LOCK_PARAMS_READ, &optdata), RES_OK);
  printColors(3, opts_colors);
  CHECK(pseConstrainedParameterSpaceValuesUnlock
    (valsopts, &optdata), RES_OK);

  CHECK(pseConstrainedParameterSpaceExplorationContextRefSub(ctxt), RES_OK);
  CHECK(pseConstrainedParameterSpaceValuesRefSub(valsopts), RES_OK);
  CHECK(pseConstrainedParameterSpaceValuesRefSub(valssmpls), RES_OK);
  CHECK(pseConstrainedParameterSpaceRefSub(cps), RES_OK);
  CHECK(pseDeviceDestroy(dev), RES_OK);
  return 0;
}
