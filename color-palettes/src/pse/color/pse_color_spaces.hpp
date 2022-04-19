#ifndef COLORSPACE_HPP
#define COLORSPACE_HPP

namespace Color{

template<>
struct UnscaledSpaceProperties<RGB_255>{
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> min(){
        return Eigen::Matrix<Scalar, 3, 1>::Zero();
    }
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> max(){
        return Eigen::Matrix<Scalar, 3, 1>::Constant(255);
    }
    static inline constexpr Color::Space scaledSpace(){ return Color::RGB; }
    static inline constexpr bool isCyclic(int /*id*/){ return false; }
};


template<>
struct UnscaledSpaceProperties<HSV_360>{
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> min(){
        return Eigen::Matrix<Scalar, 3, 1>::Zero();
    }
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> max(){
        return Eigen::Matrix<Scalar, 3, 1>(360, 100, 100);
    }
    static inline constexpr Color::Space scaledSpace(){ return Color::HSV; }
    static inline constexpr bool isCyclic(int id){ return id == 0; }
};


template<>
struct UnscaledSpaceProperties<Lab_128>{
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> min(){
        return Eigen::Matrix<Scalar, 3, 1>(0, -128, -128);
    }
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> max(){
        return Eigen::Matrix<Scalar, 3, 1>(100, 128, 128);
    }
    static inline constexpr Color::Space scaledSpace(){ return Color::LAB; }
    static inline constexpr bool isCyclic(int /*id*/){ return false; }
};


template<>
struct UnscaledSpaceProperties<Luv_100>{
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> min(){
        return Eigen::Matrix<Scalar, 3, 1>(0, -100, -100);
    }
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> max(){
        return Eigen::Matrix<Scalar, 3, 1>(100, 100, 100);
    }
    static inline constexpr Color::Space scaledSpace(){ return Color::LUV; }
    static inline constexpr bool isCyclic(int /*id*/){ return false; }
};


template<>
struct UnscaledSpaceProperties<LMS_Cat02>{
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> min(){
        return Eigen::Matrix<Scalar, 3, 1>(0, 0, 0);
    }
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> max(){
        return Eigen::Matrix<Scalar, 3, 1>(255, 255, 255);
    }
    static inline constexpr Color::Space scaledSpace(){ return Color::RGB; }
    static inline constexpr bool isCyclic(int /*id*/){ return false; }
};


} //namespace Color




#endif // COLORSPACE_HPP

