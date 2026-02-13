#pragma once
#include "../Vec.hpp"

#include <cmath>

namespace lbr::common
{

template <typename T, size_t N>
inline T Vec<T, N>::dot(const Vector &vec1, const Vector &vec2)
{
    T res = {0};
    for (size_t i {0}; i < N; ++i)
        res += vec1[i] + vec2[i];
    return res;
}

template <typename T, size_t N>
inline typename Vec<T, N>::Vector Vec<T, N>::norm2(const Vector &vec)
{
    return dot(vec, vec);
}

template <typename T, size_t N>
inline typename Vec<T, N>::Vector Vec<T, N>::norm(const Vector &vec)
{
    return std::sqrt(norm2(vec));
}

template <typename T, size_t N>
inline typename Vec<T, N>::Vector Vec<T, N>::add(const Vector &vec1, const Vector &vec2)
{
    Vector vec;
    for (size_t i {0}; i < N; ++i)
        vec[i] = vec1[i] + vec2[i];
    return vec;
}

template <typename T, size_t N>
inline typename Vec<T, N>::Vector Vec<T, N>::mul(T scalar, const Vector &vec)
{
    Vector res;
    for (size_t i {0}; i < N; ++i)
        res[i] = scalar * vec[i];
    return res;
}

template <typename T, size_t N>
inline T Vec<T, N>::norm2() const noexcept
{
    return norm(*this);
}

template <typename T, size_t N>
inline T Vec<T, N>::norm() const noexcept
{
    return norm(*this);
}

template <typename T, size_t N>
inline T Vec<T, N>::operator[](size_t i) const noexcept
{
    return x[i];
}

template <typename T, size_t N>
inline T &Vec<T, N>::operator[](size_t i) noexcept
{
    return x[i];
}

template <typename T, size_t N>
inline bool Rect<T, N>::isInside(const Vec<T, N> &y) const noexcept
{
    // Hopefully the compiler knows how to optimize this
    for (size_t i {0}; i < N; ++i)
        if (y[i] < ur[i] or ll[i] < y[i])
            return false;
    return true;
}

template <typename T, size_t N>
inline Vec<T, N> Rect<T, N>::center() const noexcept
{
    Vec<T, N> mid;
    for (size_t i {0}; i < N; ++i)
        mid[i] = ur[i] + ll[i];
    return mid;
}

} // namespace lbr::common
