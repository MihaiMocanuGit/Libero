#pragma once
#include <mutex>
namespace lbr::utl
{
template <class MtxT, template <class> class LockT, bool LockCond>
struct ConditLock;
template <class MtxT, template <class> class LockT>
struct ConditLock<MtxT, LockT, true>
{
    LockT<MtxT> lock;
    explicit ConditLock(MtxT &mtx) : lock {mtx} {}
};
template <class MtxT, template <class> class LockT>
struct ConditLock<MtxT, LockT, false>
{
    ConditLock([[maybe_unused]] MtxT &mtx) noexcept {}
};

template <bool LockCond, class... MtxTs>
struct ConditScopedLock;
template <class... MtxTs>
struct ConditScopedLock<true, MtxTs...>
{
    std::scoped_lock<MtxTs...> lock;
    explicit ConditScopedLock(MtxTs &...mtxs) : lock {mtxs...} {}
};
template <class... MtxTs>
struct ConditScopedLock<false, MtxTs...>
{
    ConditScopedLock([[maybe_unused]] MtxTs &...mtxs) noexcept {}
};

template <bool LockCond, class... MtxTs>
auto makeConditScopedLock(MtxTs &...mtxs)
{
    return ConditScopedLock<LockCond, MtxTs...>(mtxs...);
}
} // namespace lbr::utl
