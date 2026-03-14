#pragma once

#include <cstdint>
#include <optional>
#include <utility>

template <class Derived>
class SystemCRTP
{
  public:
    template <typename... Args>
    auto run(uint64_t dt, Args &&...args);

  protected:
    explicit SystemCRTP(uint64_t runEveryMS = 0) noexcept;

    uint64_t runEveryMS;
    uint64_t lastRunTime {0};
    bool isFirstRun {true};
};

template <class SystemCRTPDerived, typename... Args>
concept IsSystemCRTP = std::is_base_of_v<SystemCRTP<SystemCRTPDerived>, SystemCRTPDerived> &&
                       requires(SystemCRTPDerived &self, uint64_t dt, Args &&...args) {
                           self.run_crtp(dt, std::forward<Args>(args)...);
                       };

template <class Derived>
inline SystemCRTP<Derived>::SystemCRTP(uint64_t runEveryMS) noexcept : runEveryMS {runEveryMS}
{
}

template <class Derived>
template <typename... Args>
inline auto SystemCRTP<Derived>::run(uint64_t currentTime, Args &&...args)
{
    static_assert(IsSystemCRTP<Derived, Args...>,
                  "Derived does not satisfy the IsSystemCRTP<Derived> concept");

    using ReturnType_run_crtp =
        decltype(std::declval<Derived>().run_crtp(0, std::forward<Args>(args)...));
    // Special case for when run_crtp is a void function (std::optional<void> is not valid).
    if constexpr (std::is_same_v<ReturnType_run_crtp, void>)
    {
        uint64_t dt = currentTime - lastRunTime;
        if (dt < runEveryMS)
        {
            [[likely]] if (not isFirstRun)
                return;
            // the first run shouldn't be skipped.
            lastRunTime = currentTime;
            dt = 0;
        }
        static_cast<Derived *>(this)->run_crtp(dt, std::forward<Args>(args)...);
        isFirstRun = false;
        lastRunTime = currentTime;
        return;
    }
    else
    {
        // NOTE: the else clause must be present, otherwise something like this will fail:
        // if constexpr(true) return;
        // static_assert(false);

        // Now that the return type isn't void, std::optional can be used.
        // TODO: find a way to get rid of code duplication.

        // Small problem, can't have references in a std::optional, so we're conditionally using
        // std::optional<std::reference_wrapper<>> in this case.
        using ReturnType_run = std::optional<std::conditional_t<
            std::is_reference_v<ReturnType_run_crtp>,                             // ?
            std::reference_wrapper<std::remove_reference_t<ReturnType_run_crtp>>, // :
            ReturnType_run_crtp>>;
        ReturnType_run rt = std::nullopt;

        uint64_t dt = currentTime - lastRunTime;
        if (dt < runEveryMS)
        {
            [[likely]] if (not isFirstRun)
                return rt;
            // the first run shouldn't be skipped.
            lastRunTime = currentTime;
            dt = 0;
        }
        rt = static_cast<Derived *>(this)->run_crtp(dt, std::forward<Args>(args)...);
        isFirstRun = false;
        lastRunTime = currentTime;
        return rt;
    }
};
