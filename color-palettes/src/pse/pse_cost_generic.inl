#ifndef IMPLEM_DISCREPANCY_GENERIC_HPP
#define IMPLEM_DISCREPANCY_GENERIC_HPP

namespace Discrepancy {

//! Dummy class used to generate constant and constant discrepancy (0)
template<class _ParametricPoint>
struct ConstantBinaryDiscrepancyFunctor :
  public BinaryDiscrepancyFunctor<_ParametricPoint>
{
    INTERPOLIB_TYPENAME_DECLARE("discr_binaryConstant")
    using Base            = BinaryDiscrepancyFunctor<_ParametricPoint>;
    using ParametricPoint = typename Base::ParametricPoint;
    using Scalar          = typename Base::Scalar;
    using DiscrOutType    = typename Base::DiscrOutType;
    using ParameterSet    = typename Base::ParameterSet;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    static constexpr Scalar value = 0;

    virtual DiscrOutType eval( const ParameterSet& /*l*/, Scalar /*t*/) const final{
        return DiscrOutType(value);
    }
    virtual ~ConstantBinaryDiscrepancyFunctor() {}
};

//! Anchor penalizing variation to a specific reference value using a BinaryDiscrepancyFunctor
template<class _ParametricPoint, class _BinaryFunctor>
struct AnchorUnaryDiscrepancyFunctor :
  public DelegatingUnaryDiscrepancyFunctor<_ParametricPoint,_BinaryFunctor>
{
    INTERPOLIB_TYPENAME_DECLARE(_BinaryFunctor::getClassTypeName() + "_discr_unaryAnchor")
    using Base                = DelegatingUnaryDiscrepancyFunctor<_ParametricPoint,_BinaryFunctor>;
    using ParametricPoint     = typename Base::ParametricPoint;
    using Scalar              = typename Base::Scalar;
    using DiscrOutType        = typename Base::DiscrOutType;
    using ParameterSet        = typename Base::ParameterSet;
    using BinaryFunctor       = typename Base::BinaryFunctor;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;

    static_assert(std::is_same<
                  typename BinaryFunctor::Base,
                  BinaryDiscrepancyFunctor<ParametricPoint>
                  >::value,
                  "BinaryFunctor must be a BinaryDiscrepancyFunctor");
    static_assert(std::is_same<
                  typename BinaryFunctor::DiscrOutType,
                  DiscrOutType
                  >::value,
                  "BinaryFunctor::DiscrOutType must be same type than DiscrOutType");

    inline AnchorUnaryDiscrepancyFunctor(
            const ParametricPoint& ref = ParametricPoint::Zero())
        :_ref(ref) {}

    DiscrOutType eval( const ParameterSet& l, Scalar t /*passed*/) const{
        return Base::_cmpFun.eval({l[0],_ref}, t);
    }
    virtual ~AnchorUnaryDiscrepancyFunctor() {}

    inline void setRef(const ParametricPoint& ref) const { _ref = ref; }
    inline const ParametricPoint& getRef() const { return _ref; }

protected:
    DiscrOutType _evalStart( const ParameterSet& l) const override{
        setRef(l[0]);
        return DiscrOutType(0);
    }

private:
    //! \brief Reference value defining the anchor
    mutable ParametricPoint _ref;
};

//! Penalizes deviation to linear interpolation using a BinaryDiscrepancyFunctor
template<class _ParametricPoint, class _BinaryFunctor>
struct LinearDeviationUnaryDiscrepancyFunctor :
  public DelegatingUnaryDiscrepancyFunctor<_ParametricPoint,_BinaryFunctor>
{
    INTERPOLIB_TYPENAME_DECLARE(_BinaryFunctor::getClassTypeName() + "_discr_linearDeviation")
    using Base                = DelegatingUnaryDiscrepancyFunctor<_ParametricPoint,_BinaryFunctor>;
    using ParametricPoint     = typename Base::ParametricPoint;
    using Scalar              = typename Base::Scalar;
    using DiscrOutType        = typename Base::DiscrOutType;
    using ParameterSet        = typename Base::ParameterSet;
    using BinaryFunctor       = typename Base::BinaryFunctor;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;

    static_assert(std::is_same<
                  typename BinaryFunctor::Base,
                  BinaryDiscrepancyFunctor<ParametricPoint>
                  >::value,
                  "BinaryFunctor must be a BinaryDiscrepancyFunctor");
    static_assert(std::is_same<
                  typename BinaryFunctor::DiscrOutType,
                  DiscrOutType
                  >::value,
                  "BinaryFunctor::DiscrOutType must be same type than DiscrOutType");


    DiscrOutType eval( const ParameterSet& l, Scalar t) const{
        return Base::_cmpFun.eval({ l[0], (Scalar(1)-t)*_start+t*_end}, t);
    }
    DiscrOutType evalEnd  ( const ParameterSet& l) const{
        _end = l[0];
        return DiscrOutType(0);
    }
    virtual ~LinearDeviationUnaryDiscrepancyFunctor() {}

    void setStart(const ParametricPoint& start) { _start = start; }
    void setEnd  (const ParametricPoint& end)   { _end   = end; }

    const ParametricPoint& getStart() const  { return _start; }
    const ParametricPoint& getEnd()   const  { return _end; }

protected:
    DiscrOutType _evalStart( const ParameterSet& l) const override{
        _start = l[0];
        return DiscrOutType(0);
    }

private:
    mutable ParametricPoint _start, _end;
};

/*!
 * \brief Compute the mean of the distances for all the input parametric points
 * to their reference values.
 *
 * Ref: is composed of n ParametricPoints, representing the target values of the
 * n colors
 */
template<class _ParametricPoint, class _BinaryFunctor>
struct PairWiseGlobalAnchorDiscrepancyFunctor :
  public DelegatingDynamicNaryDiscrepancyFunctor<_ParametricPoint,_BinaryFunctor>
{
    INTERPOLIB_TYPENAME_DECLARE(_BinaryFunctor::getClassTypeName() + "_discr_pwglobalAnchor")
    using Base                = DelegatingDynamicNaryDiscrepancyFunctor<_ParametricPoint,_BinaryFunctor>;
    using ParametricPoint     = typename Base::ParametricPoint;
    using Scalar              = typename Base::Scalar;
    using DiscrOutType        = typename Base::DiscrOutType;
    using Iterator            = typename Base::Iterator;
    using ConstIterator       = typename Base::ConstIterator;
    using BinaryFunctor       = typename Base::BinaryFunctor;
    using Container           = std::vector<ParametricPoint>;
    using WContainer          = std::vector<Scalar>;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;

    static_assert(std::is_same<
                  typename BinaryFunctor::Base,
                  BinaryDiscrepancyFunctor<ParametricPoint>
                  >::value,
                  "BinaryFunctor must be a BinaryDiscrepancyFunctor");
    static_assert(std::is_same<
                  typename BinaryFunctor::DiscrOutType,
                  DiscrOutType
                  >::value,
                  "BinaryFunctor::DiscrOutType must be same type than DiscrOutType");


    inline PairWiseGlobalAnchorDiscrepancyFunctor() {}
    inline PairWiseGlobalAnchorDiscrepancyFunctor(Iterator begin_, Iterator end_)
        : _ref(begin_, end_) { }

    DiscrOutType eval( ConstIterator input_, ConstIterator end_, Scalar t /*passed*/) const{
        DiscrOutType out (0);
        bool useW = Base::_weights.size()!= 0 && Base::_weights.size() == _ref.size();
        typename WContainer::const_iterator itW = Base::_weights.cbegin();

        Scalar sumW = 0;
        for (typename Container::const_iterator it = _ref.cbegin();
             (it != _ref.cend()) && ! (input_ == end_);
             ++it, ++input_){
            Scalar w (1);

            if (useW){ w = *itW; ++itW; }

            out += w * Base::_cmpFun.eval({*it, *input_}, t);
            sumW += w;
        }
        return out / sumW;
    }

    DiscrOutType evalStart( ConstIterator input_, ConstIterator end_ ) const{
        _ref.clear();
        std::copy(input_, end_, std::back_inserter(_ref));

        return DiscrOutType(0);
    }
    virtual ~PairWiseGlobalAnchorDiscrepancyFunctor() {}

    inline void setRef(ConstIterator input_, ConstIterator end_) const {
        _ref.clear();
        std::copy(input_, end_, std::back_inserter(_ref));
    }

    inline void setRef(const ParametricPoint& ref, int idx) const { _ref[idx] = ref; }
    inline void setRef(const Container& ref) const { _ref = ref; }
    inline const Container& getRef() const { return _ref; }

private:

    //! \brief Reference value defining the anchor
    mutable Container _ref;
};

} // namespace Discrepancy

#endif // IMPLEM_DISCREPANCY_GENERIC_HPP
