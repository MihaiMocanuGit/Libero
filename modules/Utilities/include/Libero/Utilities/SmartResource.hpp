#pragma once

namespace lbr::utl
{
template <typename Resource, void CleanupFunction(Resource *)>
struct SmartResource
{
    /**
     * \brief Use direct access at your own risk.
     */
    Resource *res {nullptr};

    SmartResource() = default;
    explicit SmartResource(Resource *res) noexcept;

    SmartResource(const SmartResource &) = delete;
    SmartResource &operator=(const SmartResource &) = delete;

    SmartResource(SmartResource &&other) noexcept;
    SmartResource &operator=(SmartResource &&other) noexcept;

    ~SmartResource() noexcept;

    Resource *release() noexcept;
};

} // namespace lbr::utl

#include "detail/SmartResource.hxx"
