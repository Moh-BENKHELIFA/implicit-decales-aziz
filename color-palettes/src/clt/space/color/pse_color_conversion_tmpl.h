#ifndef PSE_COLOR_TYPE_CURRENT
  #error PSE_COLOR_TYPE_CURRENT must be defined with the current color type to be instancied.
#endif

#ifndef PSE_COLOR_CONV_FUNC
  #define PSE_COLOR_CONV_FUNC(FromType,ToType)                                 \
    PSE_CONCAT(PSE_CONCAT(PSE_CONCAT(pse,FromType),To),ToType)
#endif
#ifndef PSE_COLOR_CURR_CONV_FUNC
  #define PSE_COLOR_CURR_CONV_FUNC(ToType)                                     \
    PSE_COLOR_CONV_FUNC(PSE_COLOR_TYPE_CURRENT,ToType)
#endif
#ifndef PSE_COLOR_STRUCT
  #define PSE_COLOR_STRUCT(Type)                                               \
    PSE_CONCAT(PSE_CONCAT(pse_color_,Type),_t)
#endif
#ifndef PSE_COLOR_CURR_STRUCT
  #define PSE_COLOR_CURR_STRUCT                                                \
    PSE_COLOR_STRUCT(PSE_COLOR_TYPE_CURRENT)
#endif

static struct pse_color_any_components_t*
PSE_COLOR_CURR_CONV_FUNC(Any)
  (const struct PSE_COLOR_CURR_STRUCT* src,
   const pse_color_format_t dst_fmt,
   struct pse_color_any_components_t* dst)
{
  switch(dst_fmt) {
  #define PSE_COLOR_TYPE(Name)                                                 \
    case PSE_COLOR_FORMAT_##Name##_:                                           \
      return PSE_COLOR_CURR_CONV_FUNC(Name)                                    \
        (src, (struct PSE_COLOR_STRUCT(Name)*)dst) ? dst : NULL;
    PSE_COLOR_TYPES_ALL
  #undef PSE_COLOR_TYPE
    default: assert(false);
  }
  return NULL;
}

#undef PSE_COLOR_CONV_FUNC
#undef PSE_COLOR_CURR_CONV_FUNC
#undef PSE_COLOR_CURR_STRUCT
#undef PSE_COLOR_TYPE_CURRENT
