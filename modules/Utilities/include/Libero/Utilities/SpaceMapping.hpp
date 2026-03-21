#pragma once
#include "Vec.hpp"

namespace lbr::utl::spacemapping
{
/**
 * \brief Maps an N-dimensional bounded rectangular space into another N-dimensional bounded
 * rectangular space.
 */
template <VecSize N, typename RealT>
constexpr Vec<N, RealT> relativeSpaceMapping(const Vec<N, RealT> &point,
                                             const Rect<N, RealT> &baseSpace,
                                             Rect<N, RealT> &targetSpace) noexcept;

/**
 * \brief Maps a hyperplane embedded in a N dimensional space into a N-1 dimensional space, with
 * canonical basis.
 * \param planeOrientation is assumed to be a unit vector.
 */
template <VecSize N, typename RealT>
constexpr Vec<N - 1, RealT> hyperplaneToSpaceMapping(const Vec<N, RealT> &point,
                                                     const Vec<N, RealT> &planeOrigin,
                                                     const Vec<N, RealT> &planeOrientation);

/**
 * \brief Projects the given point onto a plane (screen) having normal vector camOrientation and
 * position campPosition - camOrientation. In other words, the screen is positioned one unit in
 * front of the camera point.
 * \param camOrientation is assumed to be a unit vector.
 */
template <VecSize N, typename RealT>
constexpr Vec<N - 1, RealT> projectiveMapping(const Vec<N, RealT> &point,
                                              const Vec<N, RealT> &camOrientation,
                                              const Vec<N, RealT> camPosition) noexcept;

} // namespace lbr::utl::spacemapping

#include "detail/SpaceMapping.hxx"
