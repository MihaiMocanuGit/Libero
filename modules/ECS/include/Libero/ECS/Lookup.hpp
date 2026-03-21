#pragma once

#include "Libero/ECS/Components.hpp"
#include "Libero/ECS/Entity.hpp"
#include "Libero/Utilities/ConditLock.hpp"
#include "detail/Lookup.hxx"

#include <algorithm>
#include <cassert>
#include <functional>
#include <mutex>
#include <ranges>
#include <shared_mutex>
#include <type_traits>
#include <utility>

namespace lbr::ecs
{
template <EMetaType EMT>
class Lookup
{
  public:
    explicit Lookup(SizeEid entitiesReserve = 1'000, SizeEid componentsReserve = 100) noexcept;

    // not move constructible due to mutexes
    Lookup(Lookup &&) = delete;
    Lookup &operator=(Lookup &&) = delete;
    Lookup(const Lookup &) = delete;
    Lookup &operator=(const Lookup &) = delete;

    template <bool LockCond>
    Entity createEntity() noexcept;

    template <bool LockCond, CType<EMT> T>
    void assignComponents(Entity::eid eid, T &&comp);
    template <bool LockCond, CType<EMT> T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    void assignComponent(Entity::eid eid, Args &&...args);
    template <bool LockCond, CType<EMT>... Ts>
    void assignComponents(Entity::eid eid, Ts &&...comps);

    template <bool LockCond, CType<EMT> T>
    bool hasComponent(Entity::eid eid) const;

    template <bool LockCond, CType<EMT> T>
    bool removeComponent_eid(Entity::eid eid);
    template <bool LockCond>
    bool removeComponent_eid(Entity::eid eid);
    template <bool LockCond, CType<EMT> T>
    bool removeComponent(SizeEid compIt);

    template <bool LockCond, CType<EMT> T>
    bool readComponent(Entity::eid eid, std::invocable<const T &> auto readFunct) const;
    /**
     * \note It won't propagate the current Entity as this would force an undesired lock on
     * entities. Use readGroupOfComponents<T>() instead.
     */
    template <bool LockCond, CType<EMT> T>
    void readAllComponents(std::invocable<const T &> auto funct) const;
    template <bool LockCond, CType<EMT> T, CType<EMT>... Ts>
    void readGroupOfComponents(std::invocable<Entity, const T &, const Ts &...> auto funct) const;

    template <bool LockCond, CType<EMT> T>
    bool modifyComponent(Entity::eid eid, std::invocable<T &> auto readFunct);
    template <bool LockCond, CType<EMT> T>
    void modifyAllComponents(std::invocable<T &> auto funct);
    // TODO: Transform the CType<EMT> pack by removing the repeated types and sorting
    // it in a predefined order. (In order to avoid cyclic deadlocks)
    template <bool LockCond, CType<EMT> T, CType<EMT>... Ts>
    void modifyGroupOfComponents(std::invocable<Entity, T &, Ts &...> auto funct);

    /**
     * \brief Removes the given set of Entity ids. It is recommended for this step to be the last
     * one in frame.
     * \note The previous Entity eids will get invalidated. More precisely, they will be mapped into
     * the [0, noOfEntities) range
     */
    template <bool LockCond>
    void removeEntities(std::vector<Entity::eid> eids);

    template <bool LockCond>
    size_t numberOfEntities() const noexcept;

    template <bool LockCond, CType<EMT> T>
    size_t numberOfComponents() const noexcept;

  private:
    template <bool LockCond>
    using shrLock = utl::ConditLock<std::shared_mutex, std::shared_lock, LockCond>;
    template <bool LockCond>
    using unqLock = utl::ConditLock<std::shared_mutex, std::unique_lock, LockCond>;

    detail::Entities<EMT> m_entities;
    detail::Components<EMT> m_components;
};

// Rest at this bonfire to replenish your strength before your adventure
//
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣤⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⣿⣿⣿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⣯⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢹⣿⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⢺⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡏⣧⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢶⣿⣋⣟⠭⣿⣿⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢻⣿⣭⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡏⢮⣳⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⡿⣦⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢿⠢⣽⣅⠀⠀⠀⠀⠀⠀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⡆⢤⣿⡇⠀⠀⠀⠀⣸⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢷⠸⣞⡇⠀⠀⠀⠀⡏⢧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡄⣿⣷⠀⠀⠀⠀⢻⡈⢣⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣇⢸⣿⡆⠀⠀⠀⠀⢳⣬⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢹⡈⣿⣧⠀⠀⢠⡄⣸⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⡇⢹⣿⡀⠀⢸⢧⠟⢹⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠸⣿⡇⣠⠋⢾⣾⢸⢀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡖⠀⠸⣿⣶⣿⣷⡏⢰⡿⢿⠏⣸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⡇⠀⣴⢋⣿⣿⣿⠇⡟⠁⣏⠀⣿⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⡞⣏⢦⠇⢸⡿⢿⠋⢀⣤⣀⡘⢦⡟⢸⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢹⣿⠃⢀⣴⡆⠀⠀⠈⣹⣿⡷⠆⠀⣧⠈⢿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠜⢁⡴⠋⡀⠙⢄⠀⣰⣿⣟⠓⠀⠀⢉⣴⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⣤⢰⡏⡠⠊⢀⡴⣇⠀⢀⡞⠉⠛⠀⡀⢀⣄⣩⠌⠙⢦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⣿⢣⠶⠖⠊⢀⣈⠉⣹⡷⢀⣴⡯⠔⣛⡵⠁⣠⡏⠸⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⠀⠀⠀⢹⡿⢿⠟⠀⣰⡞⠉⣿⡷⠇⠃⣠⢴⣶⣾⡋⢀⡴⣽⠁⠀⠘⣏⣀⢰⣆⠀⠀⠀⠀⠀
// ⠀⠀⠀⠀⠀⣠⣶⣶⣅⣠⣶⠀⠒⠟⢁⡴⠋⠀⠀⠀⢹⣿⣿⡋⣧⢸⡇⡏⣀⣀⠀⠙⣿⣉⠙⢤⡄⠀⠀⠀
// ⠀⠀⣠⣴⣺⢿⣿⣿⡛⠛⠿⠿⣯⣷⡲⣶⣟⣻⡀⠀⣠⣿⣿⣖⣸⣨⣿⠿⠛⣻⣿⣶⣾⣾⠇⠀⠻⣄⠀⠀
// ⠀⣾⢟⠿⠿⢶⣮⡙⢏⢢⡀⢠⡌⣿⣿⡿⠟⡿⢳⣼⣿⣿⣿⣾⣿⣧⣤⣤⣤⣿⣿⣭⣿⠁⠀⠀⣀⣈⣧⠀
// ⢺⣥⢿⠾⠿⠿⠿⡿⠚⢋⣠⠯⣿⢉⢉⠻⠾⠛⢿⣿⠻⠿⢛⢋⣤⣯⣭⠽⠶⣾⣻⢿⣻⢿⠶⢛⣻⡿⢽⠄
//
// Don't you dare go Hollow!

template <EMetaType EMT>
Lookup<EMT>::Lookup(SizeEid entitiesReserve, SizeEid componentsReserve) noexcept
{
    m_entities.reserve(entitiesReserve);
    m_components.reserve(componentsReserve);
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T>
inline size_t Lookup<EMT>::numberOfComponents() const noexcept
{
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    shrLock<LockCond> compLock {m_components.template getMutex<EType>()};
    return m_components.template size<EType>();
}

template <EMetaType EMT>
template <bool LockCond>
inline size_t Lookup<EMT>::numberOfEntities() const noexcept
{
    shrLock<LockCond> entLock {m_entities.getMutex()};
    return m_entities.size();
}

template <EMetaType EMT>
template <bool LockCond>
inline void Lookup<EMT>::removeEntities(std::vector<Entity::eid> eids)
{
    if (eids.empty())
        return;
    unqLock<LockCond> entLock {m_entities.getMutex()};
    std::vector<unqLock<LockCond>> compLocks;
    if constexpr (LockCond)
    {
        compLocks.reserve(std::to_underlying(EMT::countEType));
        std::apply([&](auto... ETypes)
                   { (compLocks.emplace_back(m_components.template getMutex<ETypes>()), ...); },
                   detail::ETypeTuple<EMT> {});
    }
    // sorting to make sure that swap with end won't interfere with the eids that follow. Duplicated
    // values are also removed.
    std::sort(eids.begin(), eids.end(), std::greater<SizeEid>());
    auto newEnd = std::unique(eids.begin(), eids.end());
    eids.erase(newEnd, eids.end());

    for (SizeEid i {0}; i < eids.size(); i++)
    {
        SizeEid current {eids[i]};
        // TODO: define a prepareRemoveComponent, so that one large remove can be made at the end,
        // instead of eids.Size() removes
        removeComponent_eid<false>(current);
        // at this point, the eid should have no Components. Now swap the Entity with the next end
        // position and update the refs of the moved Entity.
        m_entities.swapEnd(current, i);
        [&]<SizeEType... IETypes>(std::integer_sequence<SizeEType, IETypes...>)
        {
            (
                [&]
                {
                    static constexpr EMT EType {static_cast<EMT>(IETypes)};
                    SizeEid compRef {m_entities.template getComponentRefs<EType>(current)};
                    if (compRef == m_entities.NONE)
                        return;
                    m_components.template updateEntityRefs<EType>(compRef, current);
                }(),
                ...);
        }(std::make_integer_sequence<SizeEType, std::to_underlying(EMT::countEType)> {});
    };
    m_entities.removeLast(eids.size());
};

template <EMetaType EMT>
template <bool LockCond>
inline Entity Lookup<EMT>::createEntity() noexcept
{
    unqLock<LockCond> entLock {m_entities.getMutex()};
    return Entity {.id = m_entities.create()};
};

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T>
inline bool Lookup<EMT>::hasComponent(Entity::eid eid) const
{
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    shrLock<LockCond> entLock {m_entities.getMutex()};
    return m_entities.template containsComponents<EType>(eid);
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T, CType<EMT>... Ts>
inline void
    Lookup<EMT>::modifyGroupOfComponents(std::invocable<Entity, T &, Ts &...> auto modifyFunct)
{
    // Lock all given Components
    [[maybe_unused]] auto lockComps = utl::makeConditScopedLock<LockCond>(
        m_components.template getMutex<CType2EType<EMT, T>::EType>(),
        m_components.template getMutex<CType2EType<EMT, Ts>::EType>()...);
    shrLock<LockCond> lockEnt {m_entities.getMutex()};

    // Find the minimal component set to iterate over
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    [[maybe_unused]] SizeEid minSize {
        m_components.template size<EType>()}; // TODO: False positive for unused warning?
    EMT minEType {EType};
    (
        [&]
        {
            static constexpr EMT EType {CType2EType<EMT, Ts>::EType};
            SizeEid size {m_components.template size<EType>()};
            if (size < minSize)
            {
                minEType = EType;
                minSize = size;
            }
        }(),
        ...);
    const std::vector<Entity::eid> *minEids {nullptr};
    detail::ETypeRT2CT(minEType, [&]<EMT ETypeCT>
                       { minEids = &m_components.template getEntityRefs<ETypeCT>(); });
    assert(minEids);

    // Iterate over all entities having the requested Components
    for (Entity::eid eid : *minEids)
    {
        if (not m_entities.template containsComponents<CType2EType<EMT, T>::EType,
                                                       CType2EType<EMT, Ts>::EType...>(eid))
            continue;
        auto getCompLValue = [&]<CType<EMT> CT>(Entity::eid eid) -> CT &
        {
            static constexpr EMT EType {CType2EType<EMT, CT>::EType};
            return m_components
                .template getComponents<EType>()[m_entities.template getComponentRefs<EType>(eid)];
        };
        static_assert(std::is_same_v<decltype(getCompLValue.template operator()<T>(eid)), T &>,
                      "Wrong type");
        std::invoke(modifyFunct, Entity {.id = eid}, getCompLValue.template operator()<T>(eid),
                    getCompLValue.template operator()<Ts>(eid)...);
    }
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T>
inline void Lookup<EMT>::modifyAllComponents(std::invocable<T &> auto funct)
{
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    shrLock<LockCond> lockComp {m_components.template getMutex<EType>()};

    for (T &comp : m_components.template getComponents<EType>())
        std::invoke(funct, comp);
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T>
inline bool Lookup<EMT>::modifyComponent(Entity::eid eid, std::invocable<T &> auto modifyFunct)
{
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    shrLock<LockCond> lockComp {m_components.template getMutex<EType>()};
    shrLock<LockCond> lockEnt {m_entities.getMutex()};
    SizeEid comp {m_entities.template getComponentRefs<EType>(eid)};
    std::invoke(modifyFunct, m_components.template getComponents<EType>()[comp]);
    return true;
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T, CType<EMT>... Ts>
inline void Lookup<EMT>::readGroupOfComponents(
    std::invocable<Entity, const T &, const Ts &...> auto readFunct) const
{
    // Lock all given Components
    auto lockComps = utl::makeConditScopedLock<LockCond>(
        m_components.template getMutex<CType2EType<EMT, T>::EType>(),
        m_components.template getMutex<CType2EType<EMT, Ts>::EType>()...);
    shrLock<LockCond> lockEnt {m_entities.getMutex()};

    // Find the minimal component set to iterate over
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    [[maybe_unused]] SizeEid minSize {
        m_components.template size<EType>()}; // TODO: False positive for unused warning?
    EMT minEType {EType};
    (
        [&]
        {
            static constexpr EMT EType {CType2EType<EMT, Ts>::EType};
            SizeEid size {m_components.template size<EType>()};
            if (size < minSize)
            {
                minEType = EType;
                minSize = size;
            }
        }(),
        ...);
    const std::vector<Entity::eid> *minEids {nullptr};
    detail::ETypeRT2CT(minEType, [&]<EMT ETypeCT>
                       { minEids = &m_components.template getEntityRefs<ETypeCT>(); });
    assert(minEids);

    // Iterate over all entities having the requested components
    for (const Entity::eid eid : *minEids)
    {
        if (not m_entities.template containsComponents<CType2EType<EMT, T>::EType,
                                                       CType2EType<EMT, Ts>::EType...>(eid))
            continue;
        auto getCompLValue = [&]<CType<EMT> CT>(const Entity::eid eid) -> const CT &
        {
            static constexpr EMT EType {CType2EType<EMT, CT>::EType};
            return m_components
                .template getComponents<EType>()[m_entities.template getComponentRefs<EType>(eid)];
        };
        static_assert(
            std::is_same_v<decltype(getCompLValue.template operator()<T>(eid)), const T &>,
            "Wrong type");
        std::invoke(readFunct, Entity {.id = eid}, getCompLValue.template operator()<T>(eid),
                    getCompLValue.template operator()<Ts>(eid)...);
    }
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T>
inline void Lookup<EMT>::readAllComponents(std::invocable<const T &> auto readFunct) const
{
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    shrLock<LockCond> lockComp {m_components.template getMutex<EType>()};

    for (const T &comp : m_components.template getComponents<EType>())
        std::invoke(readFunct, comp);
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T>
inline bool Lookup<EMT>::readComponent(Entity::eid eid,
                                       std::invocable<const T &> auto readFunct) const
{
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    shrLock<LockCond> lockComp {m_components.template getMutex<EType>()};
    shrLock<LockCond> lockEnt {m_entities.getMutex()};
    SizeEid comp {m_entities.template getComponentRefs<EType>(eid)};
    std::invoke(readFunct, m_components.template getComponents<EType>()[comp]);
    return true;
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T>
inline bool Lookup<EMT>::removeComponent(SizeEid compIt)
{
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    unqLock<LockCond> lockComp {m_components.template getMutex<EType>()};
    shrLock<LockCond> lockEnt {m_entities.getMutex()};
    return removeComponent_eid<false, T>(m_components.template getEntityRefs<EType>(compIt));
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T>
inline bool Lookup<EMT>::removeComponent_eid(Entity::eid eid)
{
    static constexpr EMT EType {CType2EType<EMT, T>::EType};
    shrLock<LockCond> lockEnt {m_entities.getMutex()};
    SizeEid oldRef {m_entities.template removeComponentRef<EType>(eid)};
    if (oldRef == detail::Entities<EMT>::NONE)
        return false;

    unqLock<LockCond> lockComp {m_components.template getMutex<EType>()};
    m_components.template swapEnd<EType>(oldRef);
    m_entities.template assignComponent<EType>(m_components.template getEntityRefs<EType>()[oldRef],
                                               oldRef);
    // TODO: a m_prepareComponent_eid could be defined that does not do this last step. This way,
    // multiple components can be marked for removal and then chunk removed at the end.
    m_components.template removeLast<EType>();
    m_entities.template removeComponentRef<EType>(eid);
    return true;
}

template <EMetaType EMT>
template <bool LockCond>
inline bool Lookup<EMT>::removeComponent_eid(Entity::eid eid)
{
    shrLock<LockCond> lockEnt {m_entities.getMutex()};
    [&]<SizeEType... IETypes>(std::integer_sequence<SizeEType, IETypes...>)
    {
        (
            [&]
            {
                static constexpr EMT EType {static_cast<EMT>(IETypes)};
                SizeEid compRef {m_entities.template getComponentRefs<EType>(eid)};
                // early ref check to not lock for no reason.
                if (compRef == m_entities.NONE)
                    return;
                unqLock<LockCond> lockComp {m_components.template getMutex<EType>()};
                removeComponent_eid<false, typename EType2CType<EMT, EType>::CType>(eid);
            }(),
            ...);
    }(std::make_integer_sequence<SizeEType, std::to_underlying(EMT::countEType)> {});

    return true;
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT>... Ts>
inline void Lookup<EMT>::assignComponents(Entity::eid eid, Ts &&...comps)
{
    ((assignComponents<LockCond>(eid, std::forward<Ts>(comps))), ...);
}

template <EMetaType EMT>
template <bool LockCond, CType<EMT> T>
inline void Lookup<EMT>::assignComponents(Entity::eid eid, T &&comp)
{
    using ClearT = std::decay_t<T>;
    static constexpr EMT EType {CType2EType<EMT, ClearT>::EType};

    unqLock<LockCond> lockComp {m_components.template getMutex<EType>()};
    shrLock<LockCond> lockEnt {m_entities.getMutex()};

    auto compRef = m_components.template insert<EType>(eid, std::move(comp));
    m_entities.template assignComponent<EType>(eid, compRef);
};
} // namespace lbr::ecs
