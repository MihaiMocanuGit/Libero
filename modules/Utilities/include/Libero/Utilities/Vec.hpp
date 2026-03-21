#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace lbr::utl
{

using VecSize = uint16_t;

template <VecSize N, typename T>
struct Vec
{
    static constexpr VecSize Dim {N};
    using Type = T;
    using Vector = Vec<N, T>;
    using BaseType = std::array<Vector, Dim>;
    T x[N] = {};

    // constexpr Vec(T coeff[N]) noexcept { std::copy(coeff, coeff + N, x); }

    constexpr T norm2() const noexcept;
    constexpr T norm() const noexcept;

    constexpr T &operator[](VecSize i) noexcept;
    constexpr T operator[](VecSize i) const noexcept;
    constexpr bool operator==(const Vector &rhs) const noexcept;

    static constexpr bool equal(const Vector &lhs, const Vector &rhs, T eps = 0.0001) noexcept;
    static constexpr Vector add(const Vector &vec1, const Vector &vec2) noexcept;
    static constexpr Vector minus(const Vector &vec) noexcept;
    static constexpr Vector mul(T scalar, const Vector &vec) noexcept;
    static constexpr T dot(const Vector &vec1, const Vector &vec2) noexcept;
    static constexpr T norm2(const Vector &vec) noexcept;
    static constexpr T norm(const Vector &vec) noexcept;
    static constexpr BaseType canonicalBasis() noexcept;
};

template <typename T>
using Vec1 = Vec<1, T>;
using Vec1f = Vec<1, float>;
using Vec1i = Vec<1, int>;
using Vec1u = Vec<1, unsigned>;

template <typename T>
using Vec2 = Vec<2, T>;
using Vec2f = Vec<2, float>;
using Vec2i = Vec<2, int>;
using Vec2u = Vec<2, unsigned>;

template <typename T>
using Vec3 = Vec<3, T>;
using Vec3f = Vec<3, float>;
using Vec3i = Vec<3, int>;
using Vec3u = Vec<3, unsigned>;

struct Color
{
    uint8_t r, g, b, a;
};

template <VecSize N, typename T>
struct Rect
{
    static constexpr VecSize Dim {N};
    using Type = T;
    using Rectangle = Rect<N, T>;
    using Vector = Vec<N, T>;

    Vector ul, lr;

    constexpr bool isInside(const Vector &point) const noexcept;
    constexpr bool isInside(const Rectangle &rect) const noexcept;
    constexpr bool isFullyInside(const Rectangle &rect) const noexcept;
    constexpr Vector center() const noexcept;
};

template <typename T>
using Rect2 = Rect<2, float>;
using Rect2f = Rect<2, float>;
using Rect2i = Rect<2, int>;
using Rect2u = Rect<2, unsigned>;

template <typename T>
using Rect3 = Rect<3, float>;
using Rect3f = Rect<3, float>;
using Rect3i = Rect<3, int>;
using Rect3u = Rect<3, unsigned>;

} // namespace lbr::utl

#include "detail/Vec.hxx"
