#pragma once

#include <cstddef>
#include <cstdlib>

namespace lbr::utl
{

template <typename T, size_t N>
struct Vec
{
    static constexpr size_t Dim {N};
    using Type = T;
    using Vector = Vec<T, N>;
    T x[N];

    T norm2() const noexcept;
    T norm() const noexcept;

    T &operator[](size_t i) noexcept;
    T operator[](size_t i) const noexcept;

    static Vector norm2(const Vector &vec);
    static Vector norm(const Vector &vec);
    static Vector add(const Vector &vec1, const Vector &vec2);
    static Vector mul(T scalar, const Vector &vec);
    static T dot(const Vector &vec1, const Vector &vec2);
};
template <typename T>
using Vec1 = Vec<T, 1>;
using Vec1f = Vec<float, 1>;

template <typename T>
using Vec2 = Vec<T, 2>;
using Vec2f = Vec<float, 2>;

template <typename T>
using Vec3 = Vec<T, 3>;
using Vec3f = Vec<float, 3>;

struct Color
{
    std::byte r, g, b, a;
};

template <typename T, size_t N>
struct Rect
{
    Vec<T, N> ur, ll;

    bool isInside(const Vec<T, N> &y) const noexcept;

    Vec<T, N> center() const noexcept;
};
template <typename T>
using Rect3 = Rect<float, 3>;
using Rect3f = Rect<float, 3>;

} // namespace lbr::utl

#include "detail/Vec.hxx"
