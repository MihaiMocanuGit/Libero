#pragma once

#include "../SmartResource.hpp"

namespace lbr::utl
{
template <typename Resource, void CleanupFunction(Resource *)>
SmartResource<Resource, CleanupFunction>::SmartResource(Resource *res) noexcept : res {res}
{
}

template <typename Resource, void CleanupFunction(Resource *)>
SmartResource<Resource, CleanupFunction>::SmartResource(SmartResource &&other) noexcept
{
    this->res = other.res;
    other.res = nullptr;
};

template <typename Resource, void CleanupFunction(Resource *)>
SmartResource<Resource, CleanupFunction> &
    SmartResource<Resource, CleanupFunction>::operator=(SmartResource &&other) noexcept
{
    if (this != &other)
    {
        this->res = other.res;
        other.res = nullptr;
    }
    return *this;
}

template <typename Resource, void CleanupFunction(Resource *)>
SmartResource<Resource, CleanupFunction>::~SmartResource() noexcept
{
    if (res != nullptr)
        CleanupFunction(res);
}

template <typename Resource, void CleanupFunction(Resource *)>
Resource *SmartResource<Resource, CleanupFunction>::release() noexcept
{
    auto moved = res;
    res = nullptr;
    return moved;
}

} // namespace lbr::utl
