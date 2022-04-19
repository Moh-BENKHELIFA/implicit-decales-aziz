#ifndef IMPLEM_DISCREPANCY_COLOR_HPP
#define IMPLEM_DISCREPANCY_COLOR_HPP

#include <KdTree/kdtree.h>

namespace ColorDiscrepancy {

/*!
 * \brief Preserve the relative luminance
 *
 * \warning The colors defining the luminance order are set by calling #evalStart
 */
template<class _Color>
struct RelativeLuminanceDiscrepancyFunctor :
  public Discrepancy::BinaryDiscrepancyFunctor<_Color>
{
    INTERPOLIB_TYPENAME_DECLARE("colordiscr_binaryRelativeLum")
    using Base          = Discrepancy::BinaryDiscrepancyFunctor<_Color>;
    using ColorT        = typename Base::ParametricPoint;
    using Scalar        = typename Base::Scalar;
    using DiscrOutType  = typename Base::DiscrOutType;
    using ColorSet      = typename Base::ParameterSet;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    static constexpr Scalar W = 500;

private:
    using ExpectedColor = Color::ColorBase<Scalar, Base::ParametricPoint::space>;
    static_assert(std::is_same< ColorT, ExpectedColor>::value,
                  "RelativeLuminanceDiscrepancyFunctor requires colors");

public:
    virtual DiscrOutType eval( const ColorSet& l, Scalar /*t*/) const{
        Scalar l1 = l[0].template getUnscaledAs<Color::Lab_128>()(0);
        Scalar l2 = l[1].template getUnscaledAs<Color::Lab_128>()(0);
        if (_increasing)
            return l1 <= l2 ? Scalar(0) : W + std::pow(l2-l1,2);
        else
            return l1 >= l2 ? Scalar(0) : W + std::pow(l2-l1,2);
    }
    virtual ~RelativeLuminanceDiscrepancyFunctor() {}

protected:
    DiscrOutType _evalStart( const ColorSet& l) const override{

        // we can use normalized coordinates to check the luminance order
        Scalar Lstart = l[0].template getAs<Color::LAB>()(0);
        Scalar Lend   = l[1].template getAs<Color::LAB>()(0);
        _increasing   = Lend > Lstart;
        return DiscrOutType(Lstart);
    }

private:
    //mutable Scalar _Lstart;
    mutable bool _increasing;
};

namespace internal{

//! Component wise discrepancy functor
template<class _Color, Color::UnscaledSpace _space, int id>
struct CWXXXDiscrepancyFunctor :
  public Discrepancy::BinaryDiscrepancyFunctor<_Color>
{
    INTERPOLIB_TYPENAME_DECLARE
      ( "_colordiscr_cw_signed_"
      + (std::string)Color::UnscaledSpaceNames[int(_space)]
      + "_"
      + std::to_string(id))
    using Base          = Discrepancy::BinaryDiscrepancyFunctor<_Color>;
    using ColorT        = typename Base::ParametricPoint;
    using Scalar        = typename Base::Scalar;
    using DiscrOutType  = typename Base::DiscrOutType;
    using ColorSet      = typename Base::ParameterSet;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    static constexpr Color::UnscaledSpace space =  _space;

private:
    using ExpectedColor = Color::ColorBase<Scalar, Base::ParametricPoint::space>;
    static_assert(std::is_same< ColorT, ExpectedColor>::value,
                  "CWXXXDiscrepancyFunctor requires colors");

    //! \brief Values below this threshold will return 0 error
    DiscrOutType _safeThreshold;
    DiscrOutType _limitedContrast;
    mutable Scalar _contrastDistance;

public:
    inline CWXXXDiscrepancyFunctor(DiscrOutType safeTh      = DiscrOutType(0),
                                   DiscrOutType limContrast = DiscrOutType(0)):
    _safeThreshold(safeTh), _limitedContrast(limContrast), _contrastDistance(0){}

    virtual DiscrOutType eval( const ColorSet& l, Scalar /*t*/) const{
        return __eval (l, false);
    }

    inline DiscrOutType __eval( const ColorSet& l, bool calledFromStart) const{
        static constexpr bool isCyclic = Color::UnscaledSpaceProperties<space>::isCyclic(id);

        Scalar norm = l[1].template getUnscaledAs<space>()(id) -
                      l[0].template getUnscaledAs<space>()(id);
        if (isCyclic){
            static const Scalar intervalMax   = Color::UnscaledSpaceProperties<space>::template max<Scalar>()(id);
            static const Scalar intervalMin   = Color::UnscaledSpaceProperties<space>::template min<Scalar>()(id);
            static const Scalar intervalRange = intervalMax - intervalMin;
            static const Scalar halfRange     = intervalRange/Scalar(2);

            if (std::abs(norm) >= halfRange) norm = norm > 0
                    ? ( norm - intervalMax )
                    : ( intervalMax + norm);
        }

        if (_limitedContrast != DiscrOutType(0))
        {
            // where need to compute the reference distance
            if ( calledFromStart )
            {
                _contrastDistance = _limitedContrast * norm;
                return _contrastDistance;
            }

            return norm < Scalar(0)
                    ? std::max(norm, _contrastDistance)
                    : std::min(norm, _contrastDistance);
        }
        else
        {
            if (_safeThreshold == DiscrOutType(0))
                return DiscrOutType(norm);
            else {
                return (norm < Scalar(0)
                        ? std::min ( DiscrOutType(norm) + _safeThreshold,  DiscrOutType(0))
                        : std::max ( DiscrOutType(norm) - _safeThreshold,  DiscrOutType(0)));

            }
        }

    }

    inline void setSafeThreshold(const DiscrOutType& th) { _safeThreshold = th; }
    DiscrOutType safeThreshold() const { return _safeThreshold; }

    inline void setLimitedContrast(const DiscrOutType& lc) {
        _limitedContrast = lc; _contrastDistance = Scalar(0); }
    DiscrOutType limitedContrast() const { return _limitedContrast; }
    virtual ~CWXXXDiscrepancyFunctor() {}
protected:

    DiscrOutType _evalStart( const ColorSet& l) const override{
        return __eval (l, true);
    }
};

//! Unsigned component wise discrepancy functor
template<class _Color, Color::UnscaledSpace _space, int id>
struct CWUnsignedXXXDiscrepancyFunctor :
  public Discrepancy::BinaryDiscrepancyFunctor<_Color>
{
    INTERPOLIB_TYPENAME_DECLARE
      ( "colordiscr_binaryL2_"
      + (std::string)Color::UnscaledSpaceNames[int(_space)]
      + "_"
      + std::to_string(id))
    using Base          = Discrepancy::BinaryDiscrepancyFunctor<_Color>;
    using ColorT        = typename Base::ParametricPoint;
    using Scalar        = typename Base::Scalar;
    using DiscrOutType  = typename Base::DiscrOutType;
    using ColorSet      = typename Base::ParameterSet;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    static constexpr Color::UnscaledSpace space =  _space;

private:
    using ExpectedColor = Color::ColorBase<Scalar, Base::ParametricPoint::space>;
    static_assert(std::is_same< ColorT, ExpectedColor>::value,
                  "L2BinaryXXXDiscrepancyFunctor requires colors");

public:
    DiscrOutType eval( const ColorSet& l, Scalar /*t*/) const{
        static constexpr bool isCyclic = Color::UnscaledSpaceProperties<space>::isCyclic(id);

        Scalar norm = std::abs(l[1].template getUnscaledAs<space>()(id) -
                               l[0].template getUnscaledAs<space>()(id));
        if (isCyclic){
            static const Scalar intervalMax   = Color::UnscaledSpaceProperties<space>::template max<Scalar>()(id);
            static const Scalar intervalMin   = Color::UnscaledSpaceProperties<space>::template min<Scalar>()(id);
            static const Scalar intervalRange = intervalMax - intervalMin;
            static const Scalar halfRange     = intervalRange/Scalar(2);

            if (norm >= halfRange) norm = intervalMax - norm;
        }

        return DiscrOutType(norm);
    }
    virtual ~CWUnsignedXXXDiscrepancyFunctor() {}
};
}

//! Compute per-dimension L2 distance in color space
template<class _Color>
struct DistanceDiscrepancyFunctor :
  public Discrepancy::BinaryDiscrepancyFunctor<_Color>
{
    INTERPOLIB_TYPENAME_DECLARE("colordiscr_binaryL2_")
    using Base          = Discrepancy::BinaryDiscrepancyFunctor<_Color>;
    using ColorT        = typename Base::ParametricPoint;
    using Scalar        = typename Base::Scalar;
    using DiscrOutType  = typename Base::DiscrOutType;
    using ColorSet      = typename Base::ParameterSet;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    static constexpr Color::UnscaledSpace space =  Color::INVALID_UNSCALED;

private:
    using ExpectedColor = Color::ColorBase<Scalar, Base::ParametricPoint::space>;
    static_assert(std::is_same< ColorT, ExpectedColor>::value,
                  "DistanceDiscrepancyFunctor requires colors");

    //! \brief size of the region around the point
    DiscrOutType _safeThreshold;
    DiscrOutType _limitedContrast;
    mutable Scalar _contrastDistance;

public:
    inline DistanceDiscrepancyFunctor(DiscrOutType safeTh      = DiscrOutType(0),
                                      DiscrOutType limContrast = DiscrOutType(0)):
    _safeThreshold(safeTh), _limitedContrast(limContrast), _contrastDistance(0){}

    virtual DiscrOutType eval( const ColorSet& l, Scalar t /*passed*/ ) const{
        return __eval (l, t, false);
    }

    inline DiscrOutType __eval( const ColorSet& l, Scalar t /*passed*/, bool calledFromStart) const{
        static constexpr int Dim = ColorT::SpaceDim;
        static constexpr Color::UnscaledSpace space = ColorT::getUnscaledSpace();

        typename ColorT::CVector diffVec =
                  l[1].getUnscaledNative()
                - l[0].getUnscaledNative();

        for (int id = 0; id < Dim; ++id){
            static bool isCyclic = Color::UnscaledSpaceProperties<space>::isCyclic(id);

            if (isCyclic){
                //! \FIXME I'm too lazy... should use loop unrolling or a factory to avoid runtime branching
                //! In any case, this is not super optimal: getUnscaledNative will be called
                //! for each CW eval function
                //! Right now it is not critical: we optimize in Lab
                switch(id){
                case 0:
                {
                    internal::CWXXXDiscrepancyFunctor<ColorT, space, 0> f;
                    diffVec(0) = f.eval(l, t);
                }
                case 1:
                {
                    internal::CWXXXDiscrepancyFunctor<ColorT, space, 1> f;
                    diffVec(1) = f.eval(l, t);
                }
                case 2:
                {
                    internal::CWXXXDiscrepancyFunctor<ColorT, space, 2> f;
                    diffVec(2) = f.eval(l, t);
                }
                };
            }
        }

        DiscrOutType norm = diffVec.norm();


        if (_limitedContrast != DiscrOutType(0))
        {
            // where need to compute the reference distance
            if ( calledFromStart )
            {
                _contrastDistance = _limitedContrast * norm;
                return _contrastDistance;
            }

            return std::min(norm, _contrastDistance);

        }
        else
        {
            if (_safeThreshold == DiscrOutType(0))
                return DiscrOutType(norm);
            else {
                return (std::max ( DiscrOutType(norm) - _safeThreshold,  DiscrOutType(0)));
            }
        }

    }

    inline void setSafeThreshold(const DiscrOutType& th) { _safeThreshold = th; }
    DiscrOutType safeThreshold() const { return _safeThreshold; }

    inline void setLimitedContrast(const DiscrOutType& lc) {
        _limitedContrast = lc; _contrastDistance = Scalar(0); }
    DiscrOutType limitedContrast() const { return _limitedContrast; }
    virtual ~DistanceDiscrepancyFunctor() {}

protected:
    DiscrOutType _evalStart( const ColorSet& l) const override{
        return __eval (l, Scalar(0), true);
    }
};

/*!
 * \brief Measure the distance of Correlated Color Temperature
 */
template<class _Color>
struct CCTDiscrepancyFunctor :
  public Discrepancy::BinaryDiscrepancyFunctor<_Color>
{
    INTERPOLIB_TYPENAME_DECLARE("colordiscr_binaryCCT")
    using Base          = Discrepancy::BinaryDiscrepancyFunctor<_Color>;
    using ColorT        = typename Base::ParametricPoint;
    using Scalar        = typename Base::Scalar;
    using DiscrOutType  = typename Base::DiscrOutType;
    using ColorSet      = typename Base::ParameterSet;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;

private:
    using ExpectedColor = Color::ColorBase<Scalar, Base::ParametricPoint::space>;
    static_assert(std::is_same< ColorT, ExpectedColor>::value,
                  "CCTDiscrepancyFunctor requires colors");

public:

    DiscrOutType eval( const ColorSet& l, Scalar /*t*/) const{
        return DiscrOutType(std::abs(l[0].computeCCT() - l[1].computeCCT()));
    }
    virtual ~CCTDiscrepancyFunctor() {}

};

/*
 * The kdtree is built in the _Color ambiant space
 */
template<class _Color, class _BinaryFunctor>
struct PointSetDeviationUnaryDiscrepancyFunctor :
  public Discrepancy::DelegatingUnaryDiscrepancyFunctor<_Color,_BinaryFunctor>
{
    INTERPOLIB_TYPENAME_DECLARE(_BinaryFunctor::getClassTypeName() + "_pointSetDeviation")
    using Base                = ::Discrepancy::DelegatingUnaryDiscrepancyFunctor<_Color,_BinaryFunctor>;
    using ColorT              = typename Base::ParametricPoint;
    using Scalar              = typename Base::Scalar;
    using DiscrOutType        = typename Base::DiscrOutType;
    using ColorSet            = typename Base::ParameterSet;
    using BinaryFunctor       = typename Base::BinaryFunctor;
    using KdTree              = Super4PCS::KdTree<Scalar>;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;

    static_assert(std::is_same<
                  typename BinaryFunctor::Base,
                  ::Discrepancy::BinaryDiscrepancyFunctor<ColorT>
                  >::value,
                  "BinaryFunctor must be a BinaryDiscrepancyFunctor");
    static_assert(std::is_same<
                  typename BinaryFunctor::DiscrOutType,
                  DiscrOutType
                  >::value,
                  "BinaryFunctor::DiscrOutType must be same type than DiscrOutType");

    DiscrOutType eval( const ColorSet& l, Scalar t /*passed*/) const{
        typename KdTree::Index id = _tree.doQueryRestrictedClosestIndex(l[0].getNative());
        if (id != _tree.invalidIndex()){
            const typename KdTree::VectorType& v = _tree.getPoint(id);
            return Base::_cmpFun.eval({ l[0], ColorT(v)}, t);
        } else {
            std::cerr<<"Error occured during kdtree computation" << std::endl;
            assert(0);
            return DiscrOutType(0);
        }
    }
    virtual ~PointSetDeviationUnaryDiscrepancyFunctor() {}

    //! \deprecated
    inline const std::string& path() const { return _path; }
    //! \deprecated
    inline void setFromKdTree(const KdTree& tree, const std::string& path) {
      _path = path;
      _tree = tree;
    }

    inline size_t pointsCount() const { return (size_t)_tree.size(); }
    inline void clearAndReservePointsCount(const size_t count) {
      _tree.resize(count);
    }
    inline const ColorT getPoint(const size_t idx) {
      return ColorT(_tree.getPoint(idx));
    }
    inline void addPoint(const ColorT& color) {
      _tree.add(color.getNative());
    }
    inline void finalizePoints() {
      _tree.finalize();
    }
private:
    std::string _path; //path to image file
    KdTree _tree;

};

#if 0
// TODO: move this function in Qt helpers
template<typename PointSetDeviationUnaryDiscrepancyFunctor>
inline bool
helperPointSetDeviationDiscrepancyFunctorFillFromImage
  (const std::string& image_filepath,
   PointSetDeviationUnaryDiscrepancyFunctor& functor)
{
  using ColorT = typename PointSetDeviationUnaryDiscrepancyFunctor::ColorT;
  using Scalar = typename PointSetDeviationUnaryDiscrepancyFunctor::Scalar;
  using Converter = Color::ColorQtCompat<ColorT::space>;

  QImage im(QString::fromStdString(path));
  if( !im.isNull() ) {
    const int w = im.width();
    const int h = im.height();
    functor.clearAndReservePointsCount(w * h);

    int u,v;
    for (v = 0; v != h; ++v){
        for (u = 0;u != w; ++u){
            functor.addPoint(Converter::template fromQColor<Scalar>(QColor(im.pixel(u,v))).getNative());
        }
    }
    functor.finalizePoints();
    return true;
  }
  return false;
}
#endif

/*!
 * \brief Compute the distance between the mean and the reference value
 */
template<class _ParametricPoint, class _BinaryFunctor>
struct GlobalDeviationDiscrepancyFunctor :
  public Discrepancy::DelegatingDynamicNaryDiscrepancyFunctor<_ParametricPoint,_BinaryFunctor>
{
    INTERPOLIB_TYPENAME_DECLARE(_BinaryFunctor::getClassTypeName() + "_discr_globalAnchor")
    using Base                = ::Discrepancy::DelegatingDynamicNaryDiscrepancyFunctor<_ParametricPoint,_BinaryFunctor>;
    using ParametricPoint     = typename Base::ParametricPoint;
    using Scalar              = typename Base::Scalar;
    using DiscrOutType        = typename Base::DiscrOutType;
    using Iterator            = typename Base::Iterator;
    using ConstIterator       = typename Base::ConstIterator;
    using BinaryFunctor       = typename Base::BinaryFunctor;
    using Container           = std::vector<ParametricPoint>;
    using WContainer          = std::vector<Scalar>;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    static constexpr ::Color::UnscaledSpace sumSpace = BinaryFunctor::space;
    using RefPointType        = ::Color::ColorBase<Scalar,
                                                 ::Color::UnscaledSpaceProperties<sumSpace>::scaledSpace()>;

    static_assert(std::is_same<
                  typename BinaryFunctor::Base,
                  ::Discrepancy::BinaryDiscrepancyFunctor<ParametricPoint>
                  >::value,
                  "BinaryFunctor must be a BinaryDiscrepancyFunctor");
    static_assert(std::is_same<
                  typename BinaryFunctor::DiscrOutType,
                  DiscrOutType
                  >::value,
                  "BinaryFunctor::DiscrOutType must be same type than DiscrOutType");


    inline GlobalDeviationDiscrepancyFunctor() {}
    inline GlobalDeviationDiscrepancyFunctor(const ParametricPoint& ref)
        : _ref(ref) { }
    inline GlobalDeviationDiscrepancyFunctor(ConstIterator begin_, ConstIterator end_)
        { setRef(begin_, end_); }

    DiscrOutType eval( ConstIterator input_, ConstIterator end_, Scalar t /*passed*/) const{
        //DiscrOutType out (0);
        bool useW = !Base::_weights.empty();
        typename WContainer::const_iterator itW = Base::_weights.cbegin();

        typename RefPointType::CVector vec;
        Scalar sumW = 0;
        vec.setZero();
        for(ConstIterator it = input_; it != end_; ++it) {
            Scalar w (1);

            if (useW){ w = *itW; ++itW; }

            if (sumSpace == Color::INVALID_UNSCALED)
                vec += w * (*it).getUnscaledNative();
            else
                vec += w * (*it).template getUnscaledAs<sumSpace>();
            sumW += w;
        }
        RefPointType mean;
        //if (sumSpace == Color::INVALID_UNSCALED)
            mean.setUnscaledNative((vec / sumW).eval());
        //else
        //    mean.template setFromUnscaled<sumSpace>((vec / sumW).eval());

        return Base::_cmpFun.eval({_ref, mean}, t);

        /*for (; ! (input_ == end_);  ++input_){
            Scalar w (1);

            if (useW){ w = *itW; ++itW; }

            out += w * Base::_cmpFun.eval({_ref, *input_}, t);
            sumW += w;
        }
        return out / sumW;*/
    }

    DiscrOutType evalStart( ConstIterator input_, ConstIterator end_ ) const{
        setRef(input_, end_);
        return DiscrOutType(0);
    }
    virtual ~GlobalDeviationDiscrepancyFunctor() {}

    inline void setRef(ConstIterator input_, ConstIterator end_) const {
        bool useW = Base::_weights.size()!= 0;
        typename WContainer::const_iterator itW = Base::_weights.cbegin();

        typename ParametricPoint::CVector vec;
        Scalar sumW = 0;
        vec.setZero();
        for(ConstIterator it = input_; it != end_; ++it) {
            Scalar w (1);

            if (useW){ w = *itW; ++itW; }

            if (sumSpace == Color::INVALID_UNSCALED)
                vec += w * (*it).getUnscaledNative();
            else
                vec += w * (*it).template getUnscaledAs<sumSpace>();
            sumW += w;
        }
        //if (sumSpace == Color::INVALID_UNSCALED)
            _ref.setUnscaledNative((vec / sumW).eval());
        //else
        //    _ref.template setFromUnscaled<sumSpace>((vec / sumW).eval());
    }

    inline void setRef(const ParametricPoint& ref) const { _ref = ref; }
    inline void setRef(const Container& ref) const {
        setRef(ref.cbegin(), ref.cend());
    }
    inline const RefPointType& getRef() const { return _ref; }

private:

    //! \brief Reference value defining the anchor
    mutable RefPointType _ref;
};

/*!
 * \brief
 */
template<class _ParametricPoint>
struct CCTGlobalAnchorDiscrepancyFunctor :
  public Discrepancy::WeightedDynamicNaryDiscrepancyFunctor<_ParametricPoint>
{
    INTERPOLIB_TYPENAME_DECLARE("GlobalCCTAnchor")
    using Base                = Discrepancy::WeightedDynamicNaryDiscrepancyFunctor<_ParametricPoint>;
    using ParametricPoint     = typename Base::ParametricPoint;
    using Scalar              = typename Base::Scalar;
    using DiscrOutType        = typename Base::DiscrOutType;
    static constexpr size_t DiscrOutSize = Base::DiscrOutSize;
    using Iterator            = typename Base::Iterator;
    using ConstIterator       = typename Base::ConstIterator;
    using Container           = std::vector<ParametricPoint>;
    using WContainer          = std::vector<Scalar>;


    inline CCTGlobalAnchorDiscrepancyFunctor() {}
    inline CCTGlobalAnchorDiscrepancyFunctor(const Scalar& ref)
        : _ref(ref) {}

    inline CCTGlobalAnchorDiscrepancyFunctor(const ParametricPoint& ref)
    { setRef(ref); }

    inline CCTGlobalAnchorDiscrepancyFunctor(ConstIterator begin_, ConstIterator end_)
    { setRef(begin_, end_); }

    DiscrOutType eval( ConstIterator input_, ConstIterator end_, Scalar /*t*/) const{
        DiscrOutType out (0);
        bool useW = Base::_weights.size()!= 0;
        typename WContainer::const_iterator itW = Base::_weights.cbegin();

        Scalar sumW = 0;
        for (; ! (input_ == end_);  ++input_){
            Scalar w (1);

            if (useW){ w = *itW; ++itW; }

            out  += w * (*input_).computeTemperature();
            sumW += w;
        }
        return out / sumW - _ref;
    }

    DiscrOutType evalStart( ConstIterator input_, ConstIterator end_ ) const{
        setRef(input_, end_);
        return DiscrOutType(0);
    }
    virtual ~CCTGlobalAnchorDiscrepancyFunctor() {}

    inline void setRef(ConstIterator input_, ConstIterator end_) const {
        _ref = Scalar(0);
        bool useW = Base::_weights.size()!= 0;
        typename WContainer::const_iterator itW = Base::_weights.cbegin();

        Scalar sumW = 0;
        for (; ! (input_ == end_);  ++input_){
            Scalar w (1);

            if (useW){ w = *itW; ++itW; }

            _ref += w * (*input_).computeTemperature();
            sumW += w;
        }
        _ref /= sumW;
    }

    inline void setRef(const Scalar& ref) const {
        _ref = ref;
    }

    inline void setRef(const ParametricPoint& ref) const {
        _ref = ref.computeTemperature();
    }
    inline void setRef(const Container& ref) const {
        setRef(ref.cbegin(), ref.cend());
    }
    inline const Scalar& getRef() const { return _ref; }

private:

    //! \brief Reference value defining the anchor
    mutable Scalar _ref;
};

} // namespace ColorDiscrepancy


#endif // IMPLEM_DISCREPANCY_COLOR_HPP
