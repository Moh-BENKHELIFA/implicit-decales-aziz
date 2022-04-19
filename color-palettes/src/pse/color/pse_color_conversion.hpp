#ifndef COLORCONVERTIONHELPER_HPP
#define COLORCONVERTIONHELPER_HPP

#include <ColorSpace/colorspace.h>

namespace Color {

namespace internal {


////////////////////////////////////////////////////////////////////////////////
/// HSV2...
////////////////////////////////////////////////////////////////////////////////

////!  \brief Uses the helper to clip circular hue values in [0:1]
template<> struct ConvertionHelper<Space::HSV, Space::HSV>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        using Scalar = typename CVector::Scalar;
        auto mmod = [](Scalar x){
            return x > Scalar(1.)
                    ? std::modf(x,&x)
                     : x < Scalar(0) ? std::modf(x,&x) : x;
        };
        CVector out (inVec);
        out(0) = mmod(inVec(0)); // hue
        return out;
    }
};

//! \brief Implements the conversion from HSV to RGB
template<> struct ConvertionHelper<Space::HSV, Space::RGB>{
    template <typename CVector>
    static inline  CVector convert(CVector inVec ) {
        static const typename CVector::Scalar hFactor (360);
        num r, g, b;

        inVec = ConvertionHelper<HSV,HSV>::convert(inVec);

        //CVector mapped (ConvertionHelper<Space::HSV, Space::HSV>::convert(inVec));
        Hsv2Rgb(&r, &g, &b, hFactor*inVec(0),  inVec(1),  inVec(2));
        return CVector(r,g,b);
    }
};

//! \brief Implements the conversion from HSV to LAB
template<> struct ConvertionHelper<Space::HSV, Space::LAB>{
    template <typename CVector>
    static inline  CVector convert(CVector inVec ) {
        using Scalar = typename CVector::Scalar;
        static const Scalar hFactor (360);
        num r,g, b, L, A, B;

        inVec = ConvertionHelper<HSV,HSV>::convert(inVec);

        Hsv2Rgb(&r, &g, &b, hFactor*inVec(0),  inVec(1),  inVec(2));
        Rgb2Lab(&L, &A, &B,  r,         g,         b);
        return LAB_TO_UNIT_VECTOR(Scalar(L),Scalar(A),Scalar(B));
    }
};

//! \brief Implements the conversion from HSV to LUV
template<> struct ConvertionHelper<Space::HSV, Space::LUV>{
    template <typename CVector>
    static inline  CVector convert(CVector inVec ) {
        using Scalar = typename CVector::Scalar;
        static const Scalar hFactor (360);
        num r,g, b, L, U, V;

        inVec = ConvertionHelper<HSV,HSV>::convert(inVec);

        Hsv2Rgb(&r, &g, &b, hFactor*inVec(0),  inVec(1),  inVec(2));
        Rgb2Luv(&L, &U, &V,  r,         g,         b);
        return LUV_TO_UNIT_VECTOR(Scalar(L),Scalar(U),Scalar(V));
    }
};

////////////////////////////////////////////////////////////////////////////////
/// RGB2...
////////////////////////////////////////////////////////////////////////////////

//! \brief Implements the conversion from RGB to RGB
template<> struct ConvertionHelper<Space::RGB, Space::RGB>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        return inVec;
    }
};

//! \brief Implements the conversion from RGB to HSV
template<> struct ConvertionHelper<Space::RGB, Space::HSV>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        using Scalar = typename CVector::Scalar;
        static const Scalar hFactor (360);
        num h, s, v;
        Rgb2Hsv(&h, &s, &v,  inVec(0),  inVec(1),  inVec(2));
        return CVector(Scalar(h)/hFactor,Scalar(s),Scalar(v));
    }
};

//! \brief Implements the conversion from RGB to LAB
template<> struct ConvertionHelper<Space::RGB, Space::LAB>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        using Scalar = typename CVector::Scalar;
        num l, a, b;
        Rgb2Lab(&l, &a, &b,  inVec(0),  inVec(1),  inVec(2));
        return LAB_TO_UNIT_VECTOR(Scalar(l),Scalar(a),Scalar(b));
    }
};

//! \brief Implements the conversion from RGB to LUV
template<> struct ConvertionHelper<Space::RGB, Space::LUV>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        using Scalar = typename CVector::Scalar;
        num l, u, v;
        Rgb2Luv(&l, &u, &v,  inVec(0),  inVec(1),  inVec(2));
        return LUV_TO_UNIT_VECTOR(Scalar(l),Scalar(u),Scalar(v));
    }
};

////////////////////////////////////////////////////////////////////////////////
/// LAB2...
////////////////////////////////////////////////////////////////////////////////

//! \brief Implements the conversion from LAB to LAB
template<> struct ConvertionHelper<Space::LAB, Space::LAB>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        return CVector(inVec); //explicitely call copy constructor
    }
};

//! \brief Implements the conversion from LAB to RGB
template<> struct ConvertionHelper<Space::LAB, Space::RGB>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        num r, g, b;
        Lab2Rgb(&r, &g, &b,
                UNIT_TO_LAB_LUM(inVec(0)),
                UNIT_TO_LAB_COL(inVec(1)),
                UNIT_TO_LAB_COL(inVec(2)));
        return CVector(r,g,b);
    }
};

//! \brief Implements the conversion from LAB to HSV
template<> struct ConvertionHelper<Space::LAB, Space::HSV>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        using Scalar = typename CVector::Scalar;
        static const Scalar hFactor (360);
        num r,g, b, h, s, v;
        Lab2Rgb(&r, &g, &b,
                UNIT_TO_LAB_LUM(inVec(0)),
                UNIT_TO_LAB_COL(inVec(1)),
                UNIT_TO_LAB_COL(inVec(2)));
        Rgb2Hsv(&h, &s, &v,  r,         g,         b);
        return CVector(Scalar(h)/hFactor,Scalar(s),Scalar(v));
    }
};

//! \brief Implements the conversion from LAB to LUV
template<> struct ConvertionHelper<Space::LAB, Space::LUV>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        using Scalar = typename CVector::Scalar;

        num x,y, z, l, u, v;
        Lab2Xyz(&x, &y, &z,
                UNIT_TO_LAB_LUM(inVec(0)),
                UNIT_TO_LAB_COL(inVec(1)),
                UNIT_TO_LAB_COL(inVec(2)));
        Xyz2Luv(&l, &u, &v, x, y, z);
        return LUV_TO_UNIT_VECTOR(Scalar(l),Scalar(u),Scalar(v));
    }
};
////////////////////////////////////////////////////////////////////////////////
/// LUV2...
////////////////////////////////////////////////////////////////////////////////

//! \brief Implements the conversion from LUV to LAB
template<> struct ConvertionHelper<Space::LUV, Space::LUV>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        return CVector(inVec); //explicitely call copy constructor
    }
};

//! \brief Implements the conversion from LUV to RGB
template<> struct ConvertionHelper<Space::LUV, Space::RGB>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        num r, g, b;
        Luv2Rgb(&r, &g, &b,
                UNIT_TO_LUV_LUM(inVec(0)),
                UNIT_TO_LUV_COL(inVec(1)),
                UNIT_TO_LUV_COL(inVec(2)));
        return CVector(r,g,b);
    }
};

//! \brief Implements the conversion from LUV to HSV
template<> struct ConvertionHelper<Space::LUV, Space::HSV>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        using Scalar = typename CVector::Scalar;
        static const Scalar hFactor (360);
        num r,g, b, h, s, v;
        Luv2Rgb(&r, &g, &b,
                UNIT_TO_LUV_LUM(inVec(0)),
                UNIT_TO_LUV_COL(inVec(1)),
                UNIT_TO_LUV_COL(inVec(2)));
        Rgb2Hsv(&h, &s, &v,  r,         g,         b);
        return CVector(Scalar(h)/hFactor,Scalar(s),Scalar(v));
    }
};

//! \brief Implements the conversion from LUV to LAB
template<> struct ConvertionHelper<Space::LUV, Space::LAB>{
    template <typename CVector>
    static inline  CVector convert(const CVector& inVec ) {
        using Scalar = typename CVector::Scalar;

        num x,y, z, l, a, b;
        Luv2Xyz(&x, &y, &z,
                UNIT_TO_LUV_LUM(inVec(0)),
                UNIT_TO_LUV_COL(inVec(1)),
                UNIT_TO_LUV_COL(inVec(2)));
        Xyz2Lab(&l, &a, &b, x, y, z);
        return LAB_TO_UNIT_VECTOR(Scalar(l),Scalar(a),Scalar(b));
    }
};
////////////////////////////////////////////////////////////////////////////////
/// Default version...
////////////////////////////////////////////////////////////////////////////////

//////! \brief Implements default conversion
//template <Space in, Space out>
//template <typename CVector>
//CVector
//ConvertionHelper<in, out>::convert(const CVector& inVec ) {
//    return inVec;
//}

//template struct ConvertionHelper<HSV, HSV>;
//template struct ConvertionHelper<HSV, RGB>;
//template struct ConvertionHelper<HSV, LAB>;

//template struct ConvertionHelper<RGB, RGB>;
//template struct ConvertionHelper<RGB, HSV>;
//template struct ConvertionHelper<RGB, LAB>;

//template struct ConvertionHelper<LAB, LAB>;
//template struct ConvertionHelper<LAB, RGB>;
//template struct ConvertionHelper<LAB, HSV>;

} // namespace internal
} // namespace Color


#endif // COLORCONVERTIONHELPER_HPP
