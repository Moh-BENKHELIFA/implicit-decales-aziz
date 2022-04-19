#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <pse/color/pse_color_vision_deficiencies.hpp>

#include <unsupported/Eigen/LevenbergMarquardt>

#include <array>
#include <atomic>

template<typename ParametricPoint> class ConstrainedParameterSpace;

template<typename ParametricPoint,
         typename ParameterValidationHelper>
struct SolverFunctor
{
  using CPS = ConstrainedParameterSpace<ParametricPoint>;
  using Scalar = typename ParametricPoint::Scalar;

  static constexpr int ParametricSpaceDim = ParametricPoint::SpaceDim;

  static constexpr int InputsAtCompileTime = Eigen::Dynamic;
  static constexpr int ValuesAtCompileTime = Eigen::Dynamic;

  // TODO: move this color-specific information elsewhere
  static constexpr Scalar outOfGamutExplorationFactor = 1000;

  using FakeBase     = Eigen::DenseFunctor<Scalar>;
  using InputType    = typename FakeBase::InputType;
  using ValueType    = typename FakeBase::ValueType;
  using JacobianType = typename FakeBase::JacobianType;
  using QRSolver     = typename FakeBase::QRSolver;

protected:
    int _inputs_count;
    int _values_count;
    mutable std::atomic_bool _locked_values;
    bool _initialized;

private:
    CPS* _cps;
    std::array<bool, ParametricSpaceDim> _subspace;
    int _subspaceEnabledCount;

    // TODO: mandatory feature but how to manage that more generically??
    bool _optForNormalVision,
         _optForProtanopia,
         _optForDeutaranopia;

public:
    SolverFunctor
      (const int inputs_count = InputsAtCompileTime,
       const int values_count = ValuesAtCompileTime)
      : _inputs_count(inputs_count)
      , _values_count(values_count)
      , _locked_values(false)
      , _cps(nullptr)
      , _subspaceEnabledCount(ParametricSpaceDim)
      , _optForNormalVision(true)
      , _optForProtanopia(false)
      , _optForDeutaranopia(false)
    { _subspace.fill(true); }
    virtual ~SolverFunctor() {}

    inline int inputs() const { return _inputs_count; }
    inline int values() const { return _values_count; }

    inline const std::atomic_bool& valuesLocked() const { return _locked_values; }
    inline void lockValues() const   { while (_locked_values); _locked_values.store(true); }
    inline void unlockValues() const { _locked_values.store(false); }

    inline void setSpace(CPS* space) { _cps = space; _initialized = false; }

    inline const CPS& space() const { assert(Utils::notnull(_cps)) ; return *_cps; }
    inline CPS& space() { assert(Utils::notnull(_cps)) ; return *_cps; }

    inline virtual bool isReady() const { return _initialized && _cps != nullptr; }

    inline void setSubspace(const std::array<bool, ParametricSpaceDim>& subspace)
      {
        _subspace = subspace;
        _subspaceEnabledCount = 0;
        for(bool e: _subspace)
          _subspaceEnabledCount += e ? 1 : 0;
      }
    inline const std::array<bool, ParametricSpaceDim>& subspace() const { return _subspace; }
    inline int subspaceEnabledCount() const { return _subspaceEnabledCount; }

    inline void enableNormalVision(bool enable) { _optForNormalVision   = enable; }
    inline void enablePronopia    (bool enable) { _optForProtanopia     = enable; }
    inline void enableDeutanopia  (bool enable) { _optForDeutaranopia   = enable; }

    inline bool isNormalVisionEnabled () const { return _optForNormalVision; }
    inline bool isPronopiaEnabled     () const { return _optForProtanopia;     }
    inline bool isDeutanopiaEnabled   () const { return _optForDeutaranopia;   }

    inline int nbVisionTypes() const
      { return (_optForNormalVision ? 1 : 0)
             + (_optForProtanopia   ? 1 : 0)
             + (_optForDeutaranopia ? 1 : 0); }

    //! Cases with id >= nbVisionTypes are handled by returning VISION_DEUTERANOPIA
    inline Color::CVD::VISION_TYPE getVisionType(int id) const {
        switch (id){
        case 0:
            return _optForNormalVision
                    ? Color::CVD::VISION_NORMAL
                    : (_optForProtanopia ? Color::CVD::VISION_PROTANOPIA
                                         : Color::CVD::VISION_DEUTERANOPIA);
        case 1:
            return _optForNormalVision
                    ? (_optForProtanopia ? Color::CVD::VISION_PROTANOPIA
                                         : Color::CVD::VISION_DEUTERANOPIA)
                    : Color::CVD::VISION_DEUTERANOPIA;
        default:
            return Color::CVD::VISION_DEUTERANOPIA;
        }
    }
};

#endif // SOLVER_HPP
