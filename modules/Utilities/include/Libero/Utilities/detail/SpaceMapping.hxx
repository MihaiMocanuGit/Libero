#include "../SpaceMapping.hpp"

namespace lbr::utl::spacemapping
{

template <VecSize N, typename RealT>
constexpr Vec<N, RealT> relativeSpaceMapping(const Vec<N, RealT> &point,
                                             const Rect<N, RealT> &baseSpace,
                                             Rect<N, RealT> &targetSpace) noexcept
{
    Vec<N, RealT> res;
    for (VecSize i {0}; i < N; ++i)
    {
        RealT minS {std::min(baseSpace.ul[i], baseSpace.lr[i])};
        RealT maxS {std::max(baseSpace.ul[i], baseSpace.lr[i])};
        RealT minD {std::min(targetSpace.ul[i], targetSpace.lr[i])};
        RealT maxD {std::max(targetSpace.ul[i], targetSpace.lr[i])};

        res[i] = (point[i] - minS) * (maxD - minD) / (maxS - minS) + minD;
    }
    return res;
}

template <VecSize N, typename RealT>
constexpr Vec<N - 1, RealT> hyperplaneToSpaceMapping(const Vec<N, RealT> &point,
                                                     const Vec<N, RealT> &planeOrigin,
                                                     const Vec<N, RealT> &planeOrientation)
{
    using V = Vec<N, RealT>;
    using W = Vec<N - 1, RealT>;
    // Denote with n the unit planeOrientation vector.
    // We need to find the rotation transformation that would rotate n in the following manner:
    // n = (n_0, n_1, ..., n_{N-2}, n_{N-1}) |--> (0, 0, ..., 0, 1) = e_{N-1};
    // For this we can use a trick with the Householder transformation:
    // H_{e_{N-1} - n}(n) would reflect n along a hyperplane that sits right in between e_{N-1} and
    // n (with normal unit vector 1/norm(e_{N-1} - n) * (e_{N-1} - n)). So such a reflection would
    // map n into e_{N-1}

    const V &y = point;
    const V &p = planeOrigin;
    const V &n = planeOrientation;
    // x = y - p;
    V x = V::add(y, V::minus(p));
    // v = e_{N-1} - n
    V v = V::add(V::canonicalBasis()[N - 1], V::minus(n));
    // wikipedia uses V::norm2(v) to normalize, but this doesn't seem right?
    RealT vNorm = V::norm(v);
    if (vNorm == 0.0f) // n is already e_N
    {
        W x_prime {};
        std::copy(x.x, x.x + N - 1, x_prime.x);
        return x_prime;
    }
    // v = v/|v|
    v = V::mul(1.0 / vNorm, v);
    // H(x) = x - 2 * (x^t*v)*v
    V Hx = V::add(x, V::mul(-2 * V::dot(x, v), v));
    // almost done, we just need to map the vector Hx = (Hx_0, Hx_1, ..., Hx_{N-2}, 0) into the
    // vector x' = (Hx_0, Hx_1, ..., Hx_{N-2})
    W x_prime {};
    std::copy(Hx.x, Hx.x + N - 1, x_prime.x);
    return x_prime;
}

template <VecSize N, typename RealT>
constexpr Vec<N - 1, RealT> projectiveMapping(const Vec<N, RealT> &point,
                                              const Vec<N, RealT> &camOrientation,
                                              const Vec<N, RealT> camPosition) noexcept
{
    // clang-format off
    /**
     *                                Cam (x,y,z)
     *                              /  /
     *          CameraOrientation  v  /
     *                       ____________
     *        camera plane  |       /    |
     *          (screen)    |      /     |
     *                      |     x'     | projected point
     *                      |    /       |
     *                      |___/________|
     *                         /
     *                        /
     *                       /
     *                      /
     *                     /
     *          __________/___
     *         /:        /   /| baseSpace
     *        / :       /   / |
     *       /_____________/  |
     *       |  :     /    |  | point
     *       |  :    x     |  |
     *       |  :..........|..|
     *       | /           | /
     *       |/____________|/
     *
     *          Camera
     *           / \
     *          /   \
     *         v     v
     *        ____x'___ camera screen
     *       /         \
     *      /           \
     *     /             \
     *    /               \
     *   /                 \
     *   ---------x---------  game world
     */
    // clang-format on

    // Affine math incoming:
    // 1) Find a point on the screenPlane (p) to represent the relative origin. Denote by c =
    // camPosition and by n = camOrientation
    //      p = c + 1 * n
    // 2) The screen plane is defined as:
    // n^t .* (y-p) = 0
    // (the dot product: between n and (y-p) must be 0)
    // We also know that the line starting at x and going to c, would pass through the screen plane:
    //      x + (c-x) * t = y, for some t < 1
    // Inserting the line equation into the plane equation results in:
    //      n .* (x + (c - x) * t - p) = 0
    // Simplying and extracting t yields:
    //      (n .* (x - p)) / (n .* (x - c)) = t
    // So, x is mapped in y by:
    //      y = x + (c - x) * (n .* (x - p)) / (n .* (x - c));
    // 3) All that remains is to map y in x'. Meaning, transform the 3d point y from the base space
    // into the 2d point x' from the target screen space i.e:
    // return hyperplaneToSpaceMapping(y, p, -n);

    using V = Vec<N, RealT>;
    using W = Vec<N - 1, RealT>;
    const V &c = camPosition;
    const V &n = camOrientation;
    const V &x = point;
    // 1) p = c + n
    const V p = V::add(c, n);
    // 2) t = n^t * (x-p) / (n^t * (x-c))
    const RealT t = V::dot(n, V::add(x, V::minus(p))) / V::dot(n, V::add(x, V::minus(c)));
    // y = x + t * (c-x)
    const V y = V::add(x, V::mul(t, V::add(c, V::minus(x))));
    // 3)
    const W x_prime = hyperplaneToSpaceMapping(y, p, V::minus(n));
    return x_prime;
}

template <VecSize N, typename RealT>
constexpr Vec<N - 1, RealT> projectiveParallelMapping(const Vec<N, RealT> &point,
                                                      const Vec<N, RealT> &camOrientation,
                                                      const Vec<N, RealT> camPosition) noexcept
{
    // clang-format off
    /**
     *                                Cam (x,y,z)
     *                              /  /
     *          CameraOrientation  v  /
     *                       ____________
     *        camera plane  |       /    |
     *          (screen)    |      /     |
     *                      |     x'     | projected point
     *                      |    /       |
     *                      |___/________|
     *                         /
     *                        /
     *                       /
     *                      /
     *                     /
     *          __________/___
     *         /:        /   /| baseSpace
     *        / :       /   / |
     *       /_____________/  |
     *       |  :     /    |  | point
     *       |  :    x     |  |
     *       |  :..........|..|
     *       | /           | /
     *       |/____________|/
     *
     *
     *              Camera
     *                |
     *                v
     *           _____x'____ camera screen
     *           |         |
     *           |         |
     *           |         |
     *           |         |
     *           |         |
     * `-.       |         |
     *    `-.    |         |
     *       `-. |         |
     *          `-.        |
     *             `-x     |
     *                `-.  |
     *                   `-.  game world
     */
    // clang-format on
    // Pretty much the same logic as with projectiveMapping, with the sole difference being that
    // instead of connecting a point x with the camera through a line, it's enough to define a
    // half-line passing through x, with the same direction vector (orientation) as the camera. The
    // camera position is only needed to know in which direction to propagate the line and to define
    // a zero origin of the projected plane (camera screen).
    using V = Vec<N, RealT>;
    using W = Vec<N - 1, RealT>;
    const V &c = camPosition;
    const V &n = camOrientation;
    const V &x = point;
    // 1) p = c + n
    const V p = V::add(c, n);
    // 2) t = n^t * (x - p) / |n|^2
    const RealT t = V::dot(n, V::add(x, V::minus(p))) / V::norm2(n);
    // y = x + t * n
    const V y = V::add(x, V::mul(t, n));
    // 3)
    const W x_prime = hyperplaneToSpaceMapping(y, p, V::minus(n));
    return x_prime;
}

} // namespace lbr::utl::spacemapping
