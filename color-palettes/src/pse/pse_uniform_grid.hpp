#ifndef UNIFORMGRID_HPP
#define UNIFORMGRID_HPP

#include "common.hpp"
#include "Eigen/Dense"
#include <vector>

namespace Indexing{


/*!
  Uniform grid indexed by a position in euclidean space.
  The size used to hash euclidean coordinates is defined at runtime.
  
  Loops over dimensions used to compute index values are unrolled at compile
  time.
 */
template <
        typename _value_t, //! <\brief Type of the objects stored in the grid
        int _dim,          //! <\brief Number of dimension in ambient space
        typename Scalar    //! <\brief Scalar type used for computations
        >
struct UniformGrid{
    enum {
        Dim    = _dim,
    };

    using Point    = Eigen::Matrix<Scalar,      Dim, 1>;
    using nDIndex  = Eigen::Matrix<std::size_t, Dim, 1> ;
    using LinIndex = std::size_t;
    using value_t  = _value_t;

#ifdef __DEBUG__
#define VALIDATE_INDICES true
#pragma message "Index validation is enabled in debug mode"
#else
#define VALIDATE_INDICES false
#endif

  //! \brief State of the index validation, disabled when compiled in release mode
  enum{ INDEX_VALIDATION_ENABLED = VALIDATE_INDICES };

#undef VALIDATE_INDICES

    enum MOVE_DIR { POSITIVE, NEGATIVE };

protected:
    using GridStorage = std::vector<value_t>;
    GridStorage _grid;
    Scalar _epsilon;
    LinIndex _egSize;    //! <\brief Size of the euclidean grid for each dimension

public:
    // Get the index corresponding to position p \warning Bounds are not tested
    inline LinIndex linearIndex   ( const Point& p) const
    { return UnrollIndexLoop<INDEX_VALIDATION_ENABLED>( ndIndex(p),  LinIndex(Dim-1),  _egSize ); }

    // Get the coordinates corresponding to position p \warning Bounds are not tested
    inline nDIndex ndIndex   ( const Point& p) const
    { return (p/_epsilon).template cast<typename nDIndex::Scalar>();  }

    // Get the coordinates corresponding to position p \warning Bounds are not tested
    inline Point indexToPos( const nDIndex& i) const
    { return i.template cast<Scalar>()*_epsilon;  }

    inline nDIndex linearToNdIndex( LinIndex i) const;

    // Get the coordinates corresponding to position p \warning Bounds are not tested
    inline LinIndex ndToLinearIndex   ( const nDIndex& pCoord) const
    { return UnrollIndexLoop<INDEX_VALIDATION_ENABLED>( pCoord,  LinIndex(Dim-1),  _egSize ); }

    inline UniformGrid(const Scalar epsilon)
        : _epsilon(epsilon) {
        // We need to check if epsilon is a power of two and correct it if needed
        const int gridDepth = -std::log2(epsilon);
        _egSize = std::pow(2,gridDepth);
        _epsilon = Scalar(1)/Scalar(_egSize);

        _grid = GridStorage (std::pow(_egSize, int(Dim)));
    }
    inline UniformGrid(int gridDepth) {
        _egSize = std::pow(2,gridDepth);
        _epsilon = 1.f/_egSize;

        _grid = GridStorage (std::pow(_egSize, int(Dim)));
    }

    virtual inline ~UniformGrid() {}

    inline value_t& operator[] (const Point& p)
    { return _grid[linearIndex(p)]; }

    inline const value_t& operator[] (const Point& p) const
    { return _grid[linearIndex(p)]; }

    inline std::size_t size() const { return _grid.size(); }

    //// Visitors
    /// Visitors give you element-wise r/w access to the grid. Functors must
    /// take two arguments: Point (input, read only) and a value_t (output, r/w)

    template <typename FunctorT>
    inline void visitCells(const FunctorT& f){
#pragma omp parallel for
        for (std::size_t i = 0; i < size(); i++){
            nDIndex index  = linearToNdIndex(i);
            f(indexToPos(index), _grid[i]);
        }
    }
};
template <
        typename v, //! <\brief Type of the objects stored in the grid
        int d,      //! <\brief Number of dimension in ambient space
        typename s  //! <\brief Scalar type used for computations
        >
typename UniformGrid<v,d,s>::nDIndex
UniformGrid<v,d,s>::linearToNdIndex(typename UniformGrid<v,d,s>::LinIndex i) const {
    return RollIndexLoop<INDEX_VALIDATION_ENABLED, nDIndex>(i, LinIndex(Dim-1), _egSize);
}


} // namespace Indexing

#endif // UNIFORMGRID_HPP
