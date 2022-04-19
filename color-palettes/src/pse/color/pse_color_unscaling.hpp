#ifndef COLORUNSCALINGHELPER_HPP
#define COLORUNSCALINGHELPER_HPP

namespace Color {

namespace internal {

//template <Space in, UnscaledSpace out>
//struct _unscalingDefaultRevert{
//    /*! \brief Default implementation: convert to the space attached to the unscaled
//     * space, and then revert
//     */
//    template <typename CVector> CVector
//    process(const CVector& unscaledVec){
//        static constexpr Space tmpIn = UnscaledSpaceProperties<out>::scaledSpace();
//        using Scalar = typename CVector::Scalar;
//        Color::ColorBase<Scalar, tmpIn> tmpOutput;
//        tmpOutput.setFromUnscaled(UnscalingHelper<tmpIn, out>( unscaledVec ));
//        return ConvertionHelper<tmpIn, in>::convert(tmpOutput.getNative());
//    }
//};


////////////////////////////////////////////////////////////////////////////////
/// RGB...
////////////////////////////////////////////////////////////////////////////////
template <>
struct UnscalingHelper<Space::RGB, UnscaledSpace::RGB_255>{
    template <typename CVector>
    static inline CVector convert(const CVector& inVec){
        return typename CVector::Scalar(255)*inVec;
    }

    template <typename CVector>
    static inline CVector revert(const CVector& inVec){
        return inVec / typename CVector::Scalar(255);
    }
};

//template <>
//struct UnscalingHelper<Space::LAB, UnscaledSpace::RGB_255>{

//    /*! \brief Default implementation: convert to the space attached to the unscaled
//     * space, and then revert
//     */
//    template <typename CVector> CVector
//    revert(const CVector& unscaledVec){
//        return _unscalingDefaultRevert<Space::LAB, UnscaledSpace::RGB_255>::process(unscaledVec);
//    }
//};


////////////////////////////////////////////////////////////////////////////////
/// HSV2...
////////////////////////////////////////////////////////////////////////////////
template <>
struct UnscalingHelper<Space::HSV, UnscaledSpace::HSV_360>{
    template <typename CVector>
    static inline CVector convert(const CVector& inVec){
        return CVector(typename CVector::Scalar(360) * inVec(0),
                       typename CVector::Scalar(100) * inVec(1),
                       typename CVector::Scalar(100) * inVec(2));
    }

    template <typename CVector>
    static inline CVector revert(const CVector& inVec){
        return CVector(inVec(0) / typename CVector::Scalar(360),
                       inVec(1) / typename CVector::Scalar(100),
                       inVec(2) / typename CVector::Scalar(100));
    }
};


////////////////////////////////////////////////////////////////////////////////
/// LAB2...
////////////////////////////////////////////////////////////////////////////////

template <>
struct UnscalingHelper<Space::LAB, UnscaledSpace::Lab_128>{
    template <typename CVector>
    static inline CVector convert(const CVector& inVec){
        return CVector( UNIT_TO_LAB_LUM(inVec(0)),
                        UNIT_TO_LAB_COL(inVec(1)),
                        UNIT_TO_LAB_COL(inVec(2)));
    }

    template <typename CVector>
    static inline CVector revert(const CVector& inVec){
        return LAB_TO_UNIT_VECTOR(inVec(0), inVec(1), inVec(2));
    }
};


////////////////////////////////////////////////////////////////////////////////
/// LUV2...
////////////////////////////////////////////////////////////////////////////////

template <>
struct UnscalingHelper<Space::LUV, UnscaledSpace::Luv_100>{
    template <typename CVector>
    static inline CVector convert(const CVector& inVec){
        return CVector( UNIT_TO_LUV_LUM(inVec(0)),
                        UNIT_TO_LUV_COL(inVec(1)),
                        UNIT_TO_LUV_COL(inVec(2)));
    }

    template <typename CVector>
    static inline CVector revert(const CVector& inVec){
        return LUV_TO_UNIT_VECTOR(inVec(0), inVec(1), inVec(2));
    }
};


////////////////////////////////////////////////////////////////////////////////
/// LMS2...
////////////////////////////////////////////////////////////////////////////////

template <>
struct UnscalingHelper<Space::RGB, UnscaledSpace::LMS_Cat02>{
private:


public:
    template <typename CVector>
    static inline CVector convert(CVector inVec){
        //using Scalar = typename CVector::Scalar;
        num l, m, s;
        Rgb2Cat02lms(&l, &m, &s,  inVec(0),  inVec(1),  inVec(2));
        //return CVector( Scalar(255) * l, Scalar(255) * m, Scalar(255) * s);
        return CVector( l, m,  s);

//        static constexpr std::array<Scalar, 9> M
//        {
//            17.8824,   43.5161,   4.11935,
//            3.45565,   27.1554,   3.86714,
//            0.0299566,  0.184309, 1.46709
//        };

//        return  Eigen::Map<const Eigen::Matrix<Scalar, 3, 3>> (M.data())
//               * inVec/*.array().pow(Scalar(2.2)).matrix()*/; // must be in [0:1]
    }

    //! reverting to rgb
    template <typename CVector>
    static inline CVector revert(const CVector& inVec){
        //using Scalar = typename CVector::Scalar;
        num r, g, b;
//        Cat02lms2Rgb(&r, &g, &b,
//                     inVec(0)/ Scalar(255), inVec(1)/ Scalar(255), inVec(2)/ Scalar(255));
        Cat02lms2Rgb(&r, &g, &b,
                     inVec(0), inVec(1), inVec(2));

        return CVector(r, g, b);


//        static constexpr std::array<Scalar, 9> M
//        {
//            17.8824,   43.5161,   4.11935,
//            3.45565,   27.1554,   3.86714,
//            0.0299566,  0.184309, 1.46709
//        };

//        // output is in [0:1]
//        return  (Eigen::Map<const Eigen::Matrix<Scalar, 3, 3>> (M.data()).inverse()
//               * inVec)/*.array().pow(Scalar(1)/Scalar(2.2)).matrix()*/;
    }
};

} // namespace internal
} // namespace Color


#endif // COLORUNSCALINGHELPER_HPP
