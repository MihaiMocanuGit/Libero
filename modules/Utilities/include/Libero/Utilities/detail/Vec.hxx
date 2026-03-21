#pragma once
#include "../Vec.hpp"

#include <cmath>

namespace lbr::utl
{

template <VecSize N, typename T>
inline constexpr T Vec<N, T>::dot(const Vector &vec1, const Vector &vec2) noexcept
{
    T res = {0};
    for (VecSize i {0}; i < N; ++i)
        res += vec1[i] * vec2[i];
    return res;
}

template <VecSize N, typename T>
inline constexpr T Vec<N, T>::norm2(const Vector &vec) noexcept
{
    return dot(vec, vec);
}

template <VecSize N, typename T>
inline constexpr T Vec<N, T>::norm(const Vector &vec) noexcept
{
    return std::sqrt(norm2(vec));
}

template <VecSize N, typename T>
inline constexpr bool Vec<N, T>::equal(const Vector &lhs, const Vector &rhs, T eps) noexcept
{
    bool result = true;
    for (VecSize i {0}; i < N; ++i)
        result = result and std::abs(lhs[i] - rhs[i]) <= eps;
    return result;
}

template <VecSize N, typename T>
inline constexpr typename Vec<N, T>::Vector Vec<N, T>::add(const Vector &vec1,
                                                           const Vector &vec2) noexcept
{
    Vector res;
    for (VecSize i {0}; i < N; ++i)
        res[i] = vec1[i] + vec2[i];
    return res;
}

template <VecSize N, typename T>
inline constexpr typename Vec<N, T>::Vector Vec<N, T>::mul(T scalar, const Vector &vec) noexcept
{
    Vector res;
    for (VecSize i {0}; i < N; ++i)
        res[i] = scalar * vec[i];
    return res;
}

template <VecSize N, typename T>
inline constexpr Vec<N, T>::BaseType Vec<N, T>::canonicalBasis() noexcept
{
    Vector::BaseType basis {};
    for (VecSize i {0}; i < N; ++i)
    {
        Vector v = {0};
        v[i] = 1;
        basis[i] = v;
    }
    return basis;
};

template <VecSize N, typename T>
inline constexpr typename Vec<N, T>::Vector Vec<N, T>::minus(const Vector &vec) noexcept
{
    Vector res = {};
    for (VecSize i {0}; i < N; ++i)
        res[i] = -vec[i];
    return res;
}

template <VecSize N, typename T>
inline constexpr T Vec<N, T>::norm2() const noexcept
{
    return norm(*this);
}

template <VecSize N, typename T>
inline constexpr T Vec<N, T>::norm() const noexcept
{
    return norm(*this);
}

template <VecSize N, typename T>
inline constexpr T Vec<N, T>::operator[](VecSize i) const noexcept
{
    return x[i];
}

template <VecSize N, typename T>
inline constexpr T &Vec<N, T>::operator[](VecSize i) noexcept
{
    return x[i];
}

template <VecSize N, typename T>
inline constexpr bool Vec<N, T>::operator==(const Vector &rhs) const noexcept
{
    return Vector::equal(*this, rhs);
}

template <VecSize N, typename T>
inline constexpr bool Rect<N, T>::isInside(const Vector &point) const noexcept
{
    bool result = true;
    for (VecSize i {0}; i < N; ++i)
    {
        T min {std::min(ul[i], lr[i])};
        T max {std::max(ul[i], lr[i])};
        result = result and (min <= point[i] and point[i] <= max);
    }
    return result;
}

template <VecSize N, typename T>
inline constexpr bool Rect<N, T>::isInside(const Rectangle &rect) const noexcept
{
    return isInside(rect.ul) or isInside(rect.lr);
}

template <VecSize N, typename T>
inline constexpr bool Rect<N, T>::isFullyInside(const Rectangle &rect) const noexcept
{
    return isInside(rect.ul) and isInside(rect.lr);
}

template <VecSize N, typename T>
inline constexpr Vec<N, T> Rect<N, T>::center() const noexcept
{
    Vector mid;
    for (VecSize i {0}; i < N; ++i)
    {
        // using min and max to have valid unsigned subtraction (no underflow)
        T min {std::min(ul[i], lr[i])};
        T max {std::max(ul[i], lr[i])};
        mid[i] = min + (max - min) / static_cast<T>(2);
    }
    return mid;
}

} // namespace lbr::utl
