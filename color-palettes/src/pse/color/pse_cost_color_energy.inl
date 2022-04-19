#ifndef IMPLEM_DISCREPANCY_COLOR_ENERGY_HPP
#define IMPLEM_DISCREPANCY_COLOR_ENERGY_HPP

namespace EnergyDiscrepancy{

template<class _ColorT>
struct RGBMaxDiscrepancyFunctor :
  public Discrepancy::UnaryDiscrepancyFunctor<_ColorT>
{
    INTERPOLIB_TYPENAME_DECLARE("EnergyRGBMax")
    using Base                = Discrepancy::UnaryDiscrepancyFunctor<_ColorT>;
    using ColorT              = typename Base::ParametricPoint;
    using Scalar              = typename Base::Scalar;
    using DiscrOutType        = typename Base::DiscrOutType;
    using ColorSet            = typename Base::ParameterSet;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;

    DiscrOutType eval( const ColorSet& l, Scalar /*t*/) const{
        return l[0].template getUnscaledAs<Color::RGB_255>().maxCoeff();
    }
    virtual ~RGBMaxDiscrepancyFunctor() {}
};

template<class _ParametricPoint>
struct RGBMaxGlobalAnchorDiscrepancyFunctor :
  public Discrepancy::WeightedDynamicNaryDiscrepancyFunctor<_ParametricPoint>
{
    INTERPOLIB_TYPENAME_DECLARE("GlobalRGBMaxAnchor")
    using Base                = Discrepancy::WeightedDynamicNaryDiscrepancyFunctor<_ParametricPoint>;
    using ParametricPoint     = typename Base::ParametricPoint;
    using Scalar              = typename Base::Scalar;
    using DiscrOutType        = typename Base::DiscrOutType;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    using Iterator            = typename Base::Iterator;
    using ConstIterator       = typename Base::ConstIterator;
    using Container           = std::vector<ParametricPoint>;
    using WContainer          = std::vector<Scalar>;
    using DelegatedFunctor    = RGBMaxDiscrepancyFunctor<_ParametricPoint>;


    inline RGBMaxGlobalAnchorDiscrepancyFunctor() {}
    inline RGBMaxGlobalAnchorDiscrepancyFunctor(const Scalar& ref)
    { setRef(ref); }

    inline RGBMaxGlobalAnchorDiscrepancyFunctor(const ParametricPoint& ref)
    { setRef(ref); }

    inline RGBMaxGlobalAnchorDiscrepancyFunctor(ConstIterator begin_, ConstIterator end_)
    { setRef(begin_, end_); }

    DiscrOutType eval( ConstIterator input_, ConstIterator end_, Scalar t /*passed*/) const{
        DiscrOutType out (0);
        bool useW = Base::_weights.size()!= 0;
        typename WContainer::const_iterator itW = Base::_weights.cbegin();

        Scalar sumW = 0;
        for (; ! (input_ == end_);  ++input_){
            Scalar w (1);

            if (useW){ w = *itW; ++itW; }

            out  += w * _fun.eval({*input_},Scalar(t));
            sumW += w;
        }
        return out / sumW - _ref;
    }

    DiscrOutType evalStart( ConstIterator input_, ConstIterator end_ ) const{
        setRef(input_, end_);
        return DiscrOutType(0);
    }
    virtual ~RGBMaxGlobalAnchorDiscrepancyFunctor() {}

    inline void setRef(ConstIterator input_, ConstIterator end_) const {
        _ref = Scalar(0);
        bool useW = Base::_weights.size()!= 0;
        typename WContainer::const_iterator itW = Base::_weights.cbegin();

        Scalar sumW = 0;
        for (; ! (input_ == end_);  ++input_){
            Scalar w (1);

            if (useW){ w = *itW; ++itW; }

            _ref += w * _fun.eval({*input_},Scalar(0.));
            sumW += w;
        }
        _ref /= sumW;
    }

    inline void setRef(const Scalar& ref) const {
        _ref = ref;
    }

    inline void setRef(const ParametricPoint& ref) const {
        _ref = _fun.eval(ref, Scalar(0.));
    }
    inline void setRef(const Container& ref) const {
        setRef(ref.cbegin(), ref.cend());
    }
    inline const Scalar& getRef() const { return _ref; }

    inline const DelegatedFunctor& functor() const { return _fun; }
    inline void setFunctor(const DelegatedFunctor& functor) { _fun = functor; }

private:

    //! \brief Reference value defining the anchor
    mutable Scalar _ref;
    DelegatedFunctor _fun;
};

} // namespace EnergyDiscrepancy

#endif // IMPLEM_DISCREPANCY_COLOR_ENERGY_HPP
