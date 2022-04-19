#ifndef DISCREPANCY_HPP
#define DISCREPANCY_HPP

#include "pse_common.hpp"

#include <Eigen/Core>

#include <iterator>
#include <vector>

namespace Discrepancy {

#if 0
struct DiscrepancyEvaluationContext;

typedef uintptr_t DiscrepancyFunctorDataId;
constexpr DiscrepancyFunctorDataId DiscrepancyFunctorDataId_INVALID =
  DiscrepancyFunctorDataId(-1);

template<typename ParametricPoint>
struct DiscrepancyFunctor {
  using Scalar = typename ParametricPoint::Scalar;

  typedef ERet (*create)
    (DiscrepancyEvaluationContext* dectxt,
     const size_t dfd_count,
     DiscrepancyFunctorDataId* dfd_ids)

  typedef ERet (*destroy)
    (DiscrepancyEvaluationContext* dectxt,
     const size_t dfd_count,
     const DiscrepancyFunctorDataId* dfd_ids)

  //! \brief Initialize the DFD using the same parametric points for each.
  typedef ERet (*init)
    (DiscrepancyEvaluationContext* dectxt,
     const size_t dfd_count,
     const DiscrepancyFunctorDataId* dfd_ids,
     const size_t pps,
     const ParametricPoint* pps);

  typedef Scalar (*eval)
    (DiscrepancyEvaluationContext* dectxt,
     const size_t count,
     const ParametricPoint* pps,
     const Scalar t);

  // TODO: how to manage the difference between interpolation and exploration?
  // - For exploration, we only evaluate with t == 0.
  // - For interpolation, we evaluate along t in [0,1].
  // - For exploration, we then need only the initialized value related to the
  //   "start point", that in fact, has no meaning for exploration: it's the
  //   only point we have to manage.
  // - For interpolation, we need the "start point" and the "end point" values.
  //
  // IMPORTANT: the t parameter is **ONLY** used by the discrepancy functor
  // Discrepancy::LinearDeviationUnaryDiscrepancyFunctor that do an linear
  // interpolation between its _start and _end values. This functor compute
  // the deviation from the linear interpolation given by a BinaryDiscrepancyFunctor.
  // For example, if _start = "black", _end = "white", you compute at "grey 50%"
  // (a value computed by interpolation), using the LAB_LuminanceDiscrepancyFunctor,
  // you will have the costs :
  // - t==0: -0.5
  // - t==0.25: -0.25
  // - t==0.5: 0
  // - t==0.75: 0.25
  // - t==1: 0.5
  // In fact, this is the cost computation of the deviation of the interpolation
  // made through polynomials and the linear interpolation. At end points for
  // the exploration solver, this discrepancy gives a cost of 0, so it has no
  // impact on the final result.
  //
  // To manage this case properly, and to allow to use other interpolation than
  // the linear interpolation of costs in the solvers, we should introduce a
  // concept that is the famous InterpolableCPS (or something like that), in
  // which we will be able to say how to interpolate between 2 CPS. The specific
  // case of Discrepancy::LinearDeviationUnaryDiscrepancyFunctor describes the
  // cost of the UnaryFunctors of each ParametricPoint interpolated throught
  // the pairings.
  // Here is the idea for the interpolation solver:
  // - For each pairing, compute the interpolated parametric points along the
  //   polynomial, for all 'u' (their numbers is actually given by
  //   NbParametricSteps).
  // - Compute specific costs **of the parametric point pairings**, at each
  //   parametric step, using these interpolated parametric points. For now, we
  //   have the gamut and the laplacian. **We must externalize these computation
  //   to have them elsewhere than in the solver it-self!**. Here, we should
  //   have the deviation from the linear interpolation (the specific case
  //   described above)!
  // - Unary costs of parametric points are computed independently for both CPS,
  //   at each interpolated parametric point, like if it was the only value.
  //   Unary costs are interpolated using the function given by the ICPS for
  //   each parametric point pairing. Right now it's a linear interpolation, but
  //   that may be another interpolation function.
  // - For constraints between parametric points, we compute them independently
  //   and we interpolate their results using the function given by the ICPS.
  //   Right now, we make the computation of the deviation from the reference
  //   value in the solver, but that should be embedded in the functor.
  // - For global constraints, we compute the full interpolated palettes and
  //   then we compute the global constraints independently and we interpolate
  //   them using the interpolation function given by the ICPS.
  // - We have to think about the virtual CPS associated with the CPS that will
  //   allow to compute the costs in many parallel virtual CPS, and take them
  //   into account during the optimization. Right now it's only for binary
  //   constraints, but we could also allow them in other kind of functors:
  //   unary and global.
};
#endif

////////////////////////////////////////////////////////////////////////////////
///
///

/*!
  \brief N-ary discrepancy operators, size is known at compile time
 */
template<class _ParametricPoint, int _N>
struct NaryDiscrepancyFunctor {
protected:
    using Base = NaryDiscrepancyFunctor<_ParametricPoint, _N>;

public:
    INTERPOLIB_TYPENAME_DECLARE("discr_" + std::to_string(_N))
    using ParametricPoint = _ParametricPoint;
    using Scalar          = typename ParametricPoint::Scalar;
    using DiscrOutType    = Scalar;
    static constexpr size_t DiscrOutSize = 1;
    static constexpr int N = _N;
    using ParameterSet = std::array<ParametricPoint, N>;

public:
    virtual DiscrOutType eval(const ParameterSet& l, Scalar t) const = 0;

    inline DiscrOutType evalStart (const ParameterSet& l) const {
        return (_startRefValue = _evalStart(l));
    }

    // end is going to disappear
    virtual DiscrOutType evalEnd   (const ParameterSet& l) const {
        return eval(l, Scalar(1));
    }

    inline const DiscrOutType& getRefValue() const { return _startRefValue; }

    inline void setRefValue(const DiscrOutType& v) const { _startRefValue = v; }

    virtual ~NaryDiscrepancyFunctor() {}

protected:
    virtual DiscrOutType _evalStart (const ParameterSet& l) const  {
        return eval(l, Scalar(0));
    }

    mutable DiscrOutType _startRefValue;
};

template<class ParametricPoint, int N>
struct WeightedNaryDiscrepancyFunctor :
  public Utils::Weighted<NaryDiscrepancyFunctor<ParametricPoint,N>>
{
  INTERPOLIB_TYPENAME_DECLARE("discrW_" + std::to_string(N))
  virtual ~WeightedNaryDiscrepancyFunctor() {}
};

template<class ParametricPoint>
using UnaryDiscrepancyFunctor =
  WeightedNaryDiscrepancyFunctor<ParametricPoint,1>;

template<class ParametricPoint>
using BinaryDiscrepancyFunctor =
  WeightedNaryDiscrepancyFunctor<ParametricPoint,2>;

////////////////////////////////////////////////////////////////////////////////
/// Nary combination operators and associated helpers
///

//! \brief Multiply 2 NAry functors
//! \FIXME Serialization is not supported yet
template<typename _FirstFunctor, typename _SecondFunctor>
struct NaryDiscrepancyFunctorMultiplyOp :
  public NaryDiscrepancyFunctor<typename _FirstFunctor::ParametricPoint, _FirstFunctor::N>
{
    INTERPOLIB_TYPENAME_DECLARE("discr_NaryMult")
    using Base            = NaryDiscrepancyFunctor<typename _FirstFunctor::ParametricPoint, _FirstFunctor::N>;
    using ParametricPoint = typename Base::ParametricPoint;
    using Scalar          = typename Base::Scalar;
    using DiscrOutType    = typename Base::DiscrOutType;
    using ParameterSet    = typename Base::ParameterSet;
    using FirstFunctor    = _FirstFunctor;
    using SecondFunctor   = _SecondFunctor;

    static_assert(std::is_same< typename FirstFunctor::Base::Base, Base>::value,
                  "FirstFunctor must be a NaryDiscrepancyFunctor");
    static_assert(std::is_same< typename FirstFunctor::Base::Base, Base>::value,
                  "SecondFunctor must be a NaryDiscrepancyFunctor");
    static_assert(FirstFunctor::N == SecondFunctor::N,
                  "The two functors must have the same arity");

public:
    FirstFunctor  f1;
    SecondFunctor f2;

    inline NaryDiscrepancyFunctorMultiplyOp(){}
    inline NaryDiscrepancyFunctorMultiplyOp(const FirstFunctor& f1_,
                                              const FirstFunctor& f2_)
        :f1(f1_), f2(f2_){}

    virtual DiscrOutType eval(const ParameterSet& l, Scalar t /*passed*/) const
    { return f1.eval(l, t) * f2.eval(l, t); }

    virtual DiscrOutType evalEnd  (const ParameterSet& l) const
    { return f1.evalEnd(l) * f2.evalEnd(l); }

    virtual ~NaryDiscrepancyFunctorMultiplyOp() {}

protected:

    DiscrOutType _evalStart(const ParameterSet& l) const override
    { return f1._evalStart(l) * f2._evalStart(l); }
};

//! \brief Add 2 binary functors
template<typename _FirstFunctor, typename _SecondFunctor>
struct NaryDiscrepancyFunctorAddOp :
  public NaryDiscrepancyFunctor<typename _FirstFunctor::ParametricPoint, _FirstFunctor::N>
{
public:
    INTERPOLIB_TYPENAME_DECLARE("discr_NaryAdd")
    using Base            = NaryDiscrepancyFunctor<typename _FirstFunctor::ParametricPoint, _FirstFunctor::N>;
    using ParametricPoint = typename Base::ParametricPoint;
    using Scalar          = typename Base::Scalar;
    using DiscrOutType    = typename Base::DiscrOutType;
    using ParameterSet    = typename Base::ParameterSet;
    using FirstFunctor    = _FirstFunctor;
    using SecondFunctor   = _SecondFunctor;

private:
    static_assert(std::is_same< typename FirstFunctor::Base::Base, Base>::value,
                  "FirstFunctor must be a NnaryDiscrepancyFunctor");
    static_assert(std::is_same< typename FirstFunctor::Base::Base, Base>::value,
                  "SecondFunctor must be a NnaryDiscrepancyFunctor");
    static_assert(FirstFunctor::N == SecondFunctor::N,
                  "The two functors must have the same arity");

    FirstFunctor  *_f1;
    SecondFunctor *_f2;

public:
    inline NaryDiscrepancyFunctorAddOp()
        :_f1(nullptr), _f2(nullptr){}
    inline NaryDiscrepancyFunctorAddOp(FirstFunctor* f1_,
                                         FirstFunctor* f2_)
        :_f1(f1_), _f2(f2_){}

    virtual DiscrOutType eval( const ParameterSet& l, Scalar t /*passed*/) const
    { return _f1->eval(l, t) + _f2->eval(l, t); }

    virtual DiscrOutType evalEnd  ( const ParameterSet& l) const
    { return _f1->evalEnd(l) + _f2->evalEnd(l); }

    virtual ~NaryDiscrepancyFunctorAddOp() { delete _f1; delete _f2; }

    FirstFunctor&  f1() { return _f1; }
    SecondFunctor& f2() { return _f2; }
    const FirstFunctor&  f1() const { return _f1; }
    const SecondFunctor& f2() const { return _f2; }

protected:
    DiscrOutType _evalStart( const ParameterSet& l) const override
    { return _f1->evalStart(l) + _f2->evalStart(l); }
};


/*!
 * \brief Sort-of factory combining existing operators.
 *
 * For instance, to add the contribution of two existing functors, call:
 * \code
 * NaryOpDiscrepancyFunctor::combine<NaryOpDiscrepancyFunctor>(fun1, fun2);
 * \endcode
 *
//! \TODO Rename from Binary to NAry
 */
struct NaryOpDiscrepancyFunctor {
    INTERPOLIB_TYPENAME_DECLARE("discr_NaryCombineOp")

    template <template<typename,typename>class CombOp,
              typename Func1, typename Func2>
    static inline
    CombOp<Func1, Func2>
    combine(const Func1& func1, const Func2& func2){
        return CombOp<Func1, Func2>(func1, func2);
    }

    template <template<typename,typename>class CombOp,
              typename Func1, typename Func2>
    static inline
    CombOp<Func1, Func2>*
    combine_ptr(Func1* func1, Func2* func2){
        assert(Utils::notnull(func1));
        assert(Utils::notnull(func2));
        return new CombOp<Func1, Func2>(func1, func2);
    }
};

//! \brief
template<class _ParametricPoint, class _BinaryFunctor>
struct DelegatingUnaryDiscrepancyFunctor :
  public ::Discrepancy::UnaryDiscrepancyFunctor<_ParametricPoint>
{
    INTERPOLIB_TYPENAME_DECLARE("discrD_1_" + _BinaryFunctor::getClassTypeName())
    using Base            = UnaryDiscrepancyFunctor<_ParametricPoint>;
    using ParametricPoint = typename Base::ParametricPoint;
    using Scalar          = typename Base::Scalar;
    using DiscrOutType    = typename Base::DiscrOutType;
    using ParameterSet    = typename Base::ParameterSet;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    using BinaryFunctor   = _BinaryFunctor;

    virtual ~DelegatingUnaryDiscrepancyFunctor() {}

    inline const BinaryFunctor& functor() const { return _cmpFun; }
    inline void setFunctor(const BinaryFunctor& functor) { _cmpFun = functor; }

protected:
    BinaryFunctor _cmpFun;
};

////////////////////////////////////////////////////////////////////////////////
/// Dynamic NAry operators: base classes
///
/*!
  \brief N-ary discrepancy operators, size is unknown at compile time

    The different ParametricPoint can be weighted by calling #setWeights
 */
template<class _ParametricPoint>
struct DynamicNaryDiscrepancyFunctor {
protected:
    using Base = DynamicNaryDiscrepancyFunctor<_ParametricPoint>;

public:
    INTERPOLIB_TYPENAME_DECLARE("discr_dN")
    using ParametricPoint = _ParametricPoint;
    using Scalar          = typename ParametricPoint::Scalar;
    using DiscrOutType    = Scalar;
    static constexpr size_t DiscrOutSize = 1;
    using Iterator        = typename std::vector<ParametricPoint>::iterator;
    using ConstIterator   = typename std::vector<ParametricPoint>::const_iterator;
    using WContainer      = std::vector<Scalar>;

public:
    /*!
     * \brief eval
     * \param begin_
     * \param end_
     * \param t
     * \return
     *
     * \note Values are weighed only if #setWeights has been called previously
     * \see #setWeights
     */
    virtual DiscrOutType eval(ConstIterator begin_, ConstIterator end_, Scalar t) const = 0;

    virtual DiscrOutType evalStart (ConstIterator begin_, ConstIterator end_) const {
        return eval(begin_, end_, Scalar(0));
    }

    virtual DiscrOutType evalEnd   (ConstIterator begin_, ConstIterator end_) const {
        return eval(begin_, end_, Scalar(1));
    }

    inline const WContainer& weights() const { return _weights; }
    /*!
     * \brief setWeights Set weighting values used during #eval
     * \param input_
     * \param end_
     *
     * \note std::distance(input_,end_) must be the same for this function than #eval
     * \see #eval
     */
    inline void setWeights(typename WContainer::const_iterator input_,
                           typename WContainer::const_iterator end_) {
        _weights.clear();
        std::copy(input_, end_, std::back_inserter(_weights));
    }

    virtual ~DynamicNaryDiscrepancyFunctor() {}

protected:

protected:
    WContainer _weights;
};

template<class ParametricPoint>
struct WeightedDynamicNaryDiscrepancyFunctor :
  public Utils::Weighted<DynamicNaryDiscrepancyFunctor<ParametricPoint>>
{
  INTERPOLIB_TYPENAME_DECLARE("discrW_dN")
  virtual ~WeightedDynamicNaryDiscrepancyFunctor() {}
};

// FIXME: this version takes the Weighted form but the Unary one takes the
// non-weighted one... I'm quite sure that should be consistent
template<class _ParametricPoint, class _BinaryFunctor>
struct DelegatingDynamicNaryDiscrepancyFunctor :
  public WeightedDynamicNaryDiscrepancyFunctor<_ParametricPoint>
{
    INTERPOLIB_TYPENAME_DECLARE("discrDW_dN_" + _BinaryFunctor::getClassTypeName())
    using Base            = WeightedDynamicNaryDiscrepancyFunctor<_ParametricPoint>;
    using ParametricPoint = typename Base::ParametricPoint;
    using Scalar          = typename Base::Scalar;
    using DiscrOutType    = typename Base::DiscrOutType;
    using Iterator        = typename Base::Iterator;
    using ConstIterator   = typename Base::ConstIterator;
    using WContainer      = typename Base::WContainer;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    using BinaryFunctor   = _BinaryFunctor;

    virtual ~DelegatingDynamicNaryDiscrepancyFunctor() {}

    inline const BinaryFunctor& functor() const { return _cmpFun; }
    inline void setFunctor(const BinaryFunctor& functor) { _cmpFun = functor; }

protected:
    BinaryFunctor _cmpFun;
};

} // namespace Discrepancy

#endif // DISCREPANCY_HPP
