#pragma once

#include "Libero/ECS/Components.hpp"
#include "Libero/ECS/Entity.hpp"
#include "Libero/Utilities/ConditLock.hpp"
#include "detail/Lookup.hxx"

#include <cassert>
#include <mutex>
#include <ranges>
#include <shared_mutex>
#include <type_traits>
#include <utility>

namespace lbr::ecs::lookup
{
template <components::EMetaType EMT>
class Lookup
{
  public:
    explicit Lookup(uint32_t entitiesReserve = 1'000, uint32_t componentsReserve = 100) noexcept;

    Lookup(const Lookup &) = delete;
    Lookup &operator=(const Lookup &) = delete;
    // not move constructible due to mutexes
    Lookup(Lookup &&) = delete;
    Lookup &operator=(Lookup &&) = delete;

    using CompIt = detail::ComponentVectorIt::iterator;
    using CompCIt = detail::ComponentVectorIt::citerator;
    using EntIt = detail::EntityVectorIt::iterator;
    using EntCIt = detail::EntityVectorIt::citerator;

    template <bool LockCond>
    entity::Entity createEntity() noexcept;

    // TODO: Also accept EMT EType, besides components::CType<EMT> CT. They are
    // equivalent after all.
    template <bool LockCond, components::CType<EMT> T>
    void assignComponents(entity::eid eid, T &&comp);
    template <bool LockCond, components::CType<EMT> T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    void assignComponent(entity::eid eid, Args &&...args);
    template <bool LockCond, components::CType<EMT>... Ts>
    void assignComponents(entity::eid eid, Ts &&...comps);

    template <bool LockCond, components::CType<EMT> T>
    bool hasComponent(entity::eid eid) const;

    template <bool LockCond, components::CType<EMT> T>
    bool removeComponent_eid(entity::eid eid);
    template <bool LockCond, components::CType<EMT> T>
    bool removeComponent(CompIt compIt);

    template <bool LockCond, components::CType<EMT> T>
    bool readComponent(entity::eid eid, std::invocable<const T &> auto readFunct) const;
    /**
     * \note It won't propagate the current Entity as this would force an undesired lock on
     * entities. Use readGroupOfComponents<T>() instead.
     */
    template <bool LockCond, components::CType<EMT> T>
    void readAllComponents(std::invocable<const T &> auto funct) const;
    template <bool LockCond, components::CType<EMT> T, components::CType<EMT>... Ts>
    void readGroupOfComponents(
        std::invocable<entity::Entity, const T &, const Ts &...> auto funct) const;

    template <bool LockCond, components::CType<EMT> T>
    bool modifyComponent(entity::eid eid, std::invocable<T &> auto readFunct);
    template <bool LockCond, components::CType<EMT> T>
    void modifyAllComponents(std::invocable<T &> auto funct);
    // TODO: Transform the components::CType<EMT> pack by removing the repeated types and sorting
    // it in a predefined order. (In order to avoid cyclic deadlocks)
    template <bool LockCond, components::CType<EMT> T, components::CType<EMT>... Ts>
    void modifyGroupOfComponents(std::invocable<entity::Entity, T &, Ts &...> auto funct);

    /**
     * \brief Removes the given set of entity ids. It is recommended for this step to be the last
     * one in frame.
     * \note The previous entity eids will get invalidated. More precisely, they will be mapped into
     * the [0, noOfEntities) range
     */
    template <bool LockCond>
    void removeEntities(std::vector<entity::eid> eids);

    template <bool LockCond>
    size_t numberOfEntities() const noexcept;

    template <bool LockCond, components::CType<EMT> T>
    size_t numberOfComponents() const noexcept;

  private:
    // TODO: Improve the locking performance. We lock the whole vector when just a single
    // element is modified.
    template <bool LockCond>
    using shrLock = utl::ConditLock<std::shared_mutex, std::shared_lock, LockCond>;
    template <bool LockCond>
    using unqLock = utl::ConditLock<std::shared_mutex, std::unique_lock, LockCond>;

    // TODO: For now, these structures contain only data and have no methods of their own. They
    // should have their own CRUD interface. It's not the role of Lookup to know how to deal
    // with the subtypes the types. So, after the Lookup logic is properly defined, refactor the
    // structs.
    detail::Entities<EMT> m_entities;
    detail::CompList<EMT>::template Root<detail::ComponentData> m_components;

    template <components::CType<EMT> T>
    const auto &m_getCompVec() const;
    template <EMT EType>
    const auto &m_getCompVec() const;

    template <components::CType<EMT> T>
    auto &m_getCompVec();
    template <EMT EType>
    auto &m_getCompVec();
};

template <components::EMetaType EMT>
Lookup<EMT>::Lookup(uint32_t entitiesReserve, uint32_t componentsReserve) noexcept
{
    m_entities.data.reserve(entitiesReserve);
    std::apply(
        [componentsReserve](auto &&...args)
        {
            ((args.components.reserve(componentsReserve)), ...);
            ((args.entityRefs.reserve(componentsReserve)), ...);
        },
        m_components);
}
template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T>
inline size_t Lookup<EMT>::numberOfComponents() const noexcept
{
    shrLock<LockCond> compLock {m_getCompVec<T>().mtx};
    return m_getCompVec<T>().components.size();
}

template <components::EMetaType EMT>
template <bool LockCond>
inline size_t Lookup<EMT>::numberOfEntities() const noexcept
{
    shrLock<LockCond> entLock {m_entities.mtx};
    return m_entities.data.size();
}

template <components::EMetaType EMT>
template <bool LockCond>
inline void Lookup<EMT>::removeEntities(std::vector<entity::eid> eids)
{
    if (eids.empty())
        return;

    unqLock<LockCond> entLock {m_entities.mtx};
    std::vector<unqLock<LockCond>> compLocks;
    compLocks.reserve(std::to_underlying(EMT::countEType));
    // Lock all components, even though not all are touched. Optimize later if needed.
    [&]<std::size_t... I>(std::index_sequence<I...>)
    {
        ((compLocks.emplace_back(m_getCompVec<static_cast<EMT>(I)>().mtx)), ...);
    }(std::make_index_sequence<std::to_underlying(EMT::countEType)> {});

    // Remove all components for each entity.
    for (const entity::eid currId : eids)
    {
        // Get the compile time ComponentId from the runtime ComponentId. Then remove Component.
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            (
                [&]()
                {
                    // Note that we don't lock in removeComponent
                    if (m_entities.data[currId].contains(static_cast<EMT>(I)))
                        removeComponent_eid<false, typename components::EType2CType<EMT, static_cast<
                                                       EMT>(I)>::CType>(currId);
                }(),
                ...);
        }(std::make_index_sequence<std::to_underlying(EMT::countEType)> {});
    }

    // It is important to sort them in decreasing order so that a swap with the backIndex won't
    // interfere with the next eids.
    std::sort(eids.begin(), eids.end(), std::greater<entity::eid>());

    // Now we can remove the entities
    entity::eid backIndex {static_cast<entity::eid>(m_entities.data.size())};
    entity::eid prevEid {entity::nid};
    for (entity::eid eid : eids)
    {
        // The eids might contain duplicates.
        if (eid == prevEid)
            continue;

        // Go over all components of the backIndex entity and update them to reflect the new
        // position.
        --backIndex;
        [&]<std::size_t... ETypeI>(std::index_sequence<ETypeI...>)
        {
            (
                [&]
                {
                    if (m_entities.data[backIndex].contains(static_cast<EMT>(ETypeI)))
                        m_getCompVec<static_cast<EMT>(ETypeI)>()
                            .entityRefs[m_entities.data[backIndex].at(
                                static_cast<EMT>(ETypeI))] = eid;
                }(),
                ...);
        }(std::make_index_sequence<std::to_underlying(EMT::countEType)> {});

        std::swap(m_entities.data[eid], m_entities.data[backIndex]);
        prevEid = eid;
    }
    // Now all eids are in the region [backIndex, m_entities.end())
    m_entities.data.erase(m_entities.data.begin() + backIndex, m_entities.data.end());
};

template <components::EMetaType EMT>
template <EMT EType>
inline auto &Lookup<EMT>::m_getCompVec()
{
    return m_getCompVec<typename components::EType2CType<EMT,EType>::CType>();
}

template <components::EMetaType EMT>
template <EMT EType>
inline const auto &Lookup<EMT>::m_getCompVec() const
{
    return m_getCompVec<typename components::EType2CType<EMT, EType>::CType>();
}

template <components::EMetaType EMT>
template <bool LockCond>
inline entity::Entity Lookup<EMT>::createEntity() noexcept
{
    unqLock<LockCond> _ {m_entities.mtx};
    entity::eid id = m_entities.data.size();
    m_entities.data.push_back({});
    return entity::Entity {.id = id};
};

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T>
inline bool Lookup<EMT>::hasComponent(entity::eid eid) const
{
    shrLock<LockCond> lockEnt {m_entities.mtx};
    return m_entities.data[eid].contains(components::CType2EType<EMT, T>::EType);
}

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T, components::CType<EMT>... Ts>
inline void
    Lookup<EMT>::modifyGroupOfComponents(std::invocable<entity::Entity, T &, Ts &...> auto funct)
{
    constexpr unsigned NO_TYPES = 1 + sizeof...(Ts);

    // Make sure to lock all the requested components
    std::vector<unqLock<LockCond>> lockComps;
    if constexpr (LockCond)
    {
        lockComps.reserve(NO_TYPES);
        lockComps.emplace_back(m_getCompVec<T>().mtx);
        ((lockComps.emplace_back(m_getCompVec<Ts>().mtx)), ...);
    }

    const std::array<size_t, NO_TYPES> compVecSizes = {m_getCompVec<T>().components.size(),
                                                       m_getCompVec<Ts>().components.size()...};
    size_t indexSmallestCompVec =
        std::min_element(compVecSizes.begin(), compVecSizes.end()) - compVecSizes.begin();

    // obtain the smallest set of candidate entities. These entities will then be filtered to
    // obtain the ones having all the necessary components
    // TODO: reference_wrapper?
    auto *backRefs = &m_getCompVec<T>().entityRefs;
    size_t i {1};
    ((backRefs = (i++ == indexSmallestCompVec) ? &m_getCompVec<Ts>().entityRefs : backRefs), ...);

    // the entity remains untouched, so no need to unqLock<LockCond> it.
    shrLock<LockCond> lockEnt {m_entities.mtx};
    auto hasAll = [&](entity::eid eid)
    {
        const auto &map = m_entities.data[eid];
        return (map.contains(components::CType2EType<EMT, T>::EType) && ... && map.contains(components::CType2EType<EMT, Ts>::EType));
    };
    for (entity::eid eid : *backRefs)
    {
        const auto &entCompMap {m_entities.data[eid]};
        if (not hasAll(eid))
            continue;

        std::invoke(funct, m_entities.entity(eid),
                    m_getCompVec<T>().components[entCompMap.at(components::CType2EType<EMT, T>::EType)],
                    m_getCompVec<Ts>().components[entCompMap.at(components::CType2EType<EMT, Ts>::EType)]...);
    }
}

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T, components::CType<EMT>... Ts>
inline void Lookup<EMT>::readGroupOfComponents(
    std::invocable<entity::Entity, const T &, const Ts &...> auto funct) const
{
    constexpr unsigned NO_TYPES = 1 + sizeof...(Ts);

    // It seems that CTAD does not work in this context
    utl::ConditScopedLock<LockCond, decltype(m_getCompVec<T>().mtx),
                          decltype(m_getCompVec<Ts>().mtx)...>
        lockComps {m_getCompVec<T>().mtx, m_getCompVec<Ts>().mtx...};

    const std::array<size_t, NO_TYPES> compVecSizes = {m_getCompVec<T>().components.size(),
                                                       m_getCompVec<Ts>().components.size()...};
    size_t indexSmallestCompVec =
        std::min_element(compVecSizes.begin(), compVecSizes.end()) - compVecSizes.begin();

    // obtain the smallest set of candidate entities. These entities will then be filtered to
    // obtain the ones having all the necessary components
    // TODO: reference_wrapper?
    auto *backRefs = &m_getCompVec<T>().entityRefs;
    size_t i {1};
    ((backRefs = (i++ == indexSmallestCompVec) ? &m_getCompVec<Ts>().entityRefs : backRefs), ...);

    shrLock<LockCond> lockEnt {m_entities.mtx};
    auto hasAll = [&](entity::eid eid)
    {
        const auto &map = m_entities.data[eid];
        return (map.contains(components::CType2EType<EMT, T>::EType) && ... && map.contains(components::CType2EType<EMT, Ts>::EType));
    };
    for (entity::eid eid : *backRefs)
    {
        const auto &entCompMap {m_entities.data[eid]};
        if (not hasAll(eid))
            continue;

        std::invoke(funct, m_entities.entity(eid),
                    m_getCompVec<T>().components[entCompMap.at(components::CType2EType<EMT, T>::EType)],
                    m_getCompVec<Ts>().components[entCompMap.at(components::CType2EType<EMT, Ts>::EType)]...);
    }
}

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T>
inline void Lookup<EMT>::readAllComponents(std::invocable<const T &> auto funct) const
{
    const auto &compVec = m_getCompVec<T>();
    shrLock<LockCond> lockComp {compVec.mtx};
    for (const T &comp : compVec.components)
        std::invoke(funct, comp);
}

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T>
inline void Lookup<EMT>::modifyAllComponents(std::invocable<T &> auto funct)
{
    auto &compVec = m_getCompVec<T>();
    unqLock<LockCond> lockComp {compVec.mtx};
    for (T &comp : compVec.components)
        std::invoke(funct, comp);
}

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T>
inline bool Lookup<EMT>::readComponent(entity::eid eid,
                                       std::invocable<const T &> auto readFunct) const
{
    const auto &compVec = m_getCompVec<T>();
    CompCIt compIt;
    {
        shrLock<LockCond> lockEnt {m_entities.mtx};
        if (not m_entities.data[eid].contains(components::CType2EType<EMT, T>::EType))
            return false;
        compIt = m_entities.data[eid].at(components::CType2EType<EMT, T>::EType);
    }
    shrLock<LockCond> lockComp {compVec.mtx};
    std::invoke(readFunct, compVec.components[compIt]);
    return true;
}

template <components::EMetaType EMT>
template <components::CType<EMT> T>
inline const auto &Lookup<EMT>::m_getCompVec() const
{
    return std::get<detail::ComponentData<EMT, components::CType2EType<EMT, T>::EType>>(m_components);
}

template <components::EMetaType EMT>
template <components::CType<EMT> T>
inline auto &Lookup<EMT>::m_getCompVec()
{
    return std::get<detail::ComponentData<EMT, components::CType2EType<EMT, std::decay_t<T>>::EType>>(m_components);
}

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T>
inline bool Lookup<EMT>::removeComponent(CompIt compIt)
{
    auto &compVec = m_getCompVec<T>();
    unqLock<LockCond> lockComp {compVec.mtx};
    // saving the ref to entity as it will be deleted;
    EntIt entIt = compVec.entityRefs[compIt];

    std::swap(compVec.components[compIt], compVec.components.back());
    compVec.components.pop_back();

    std::swap(compVec.entityRefs[compIt], compVec.entityRefs.back());
    compVec.entityRefs.pop_back();

    unqLock<LockCond> lockEnt {m_entities.mtx};
    m_entities.data[entIt].erase(components::CType2EType<EMT, T>::EType);
    // update the compRef held by the entity of the swapped component
    if (compIt < compVec.components.size()) // checks if compIt wasn't already the last element
        m_entities.data[compVec.entityRefs[compIt]].at(components::CType2EType<EMT, T>::EType) = compIt;
    return true;
}

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T>
inline bool Lookup<EMT>::removeComponent_eid(entity::eid eid)
{
    CompIt compIt;
    {
        shrLock<LockCond> lockEnt {m_entities.mtx};
        auto &entCompMap = m_entities.data[eid];
        if (not entCompMap.contains(components::CType2EType<EMT, T>::EType))
            return false;
        compIt = entCompMap.at(components::CType2EType<EMT, T>::EType);
    } // note that a deadlock would happen if lockEnt is not released
    return removeComponent<LockCond, T>(compIt);
}

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT>... Ts>
inline void Lookup<EMT>::assignComponents(entity::eid eid, Ts &&...comps)
{
    ((assignComponents<LockCond>(eid, std::forward<Ts>(comps))), ...);
}

template <components::EMetaType EMT>
template <bool LockCond, components::CType<EMT> T>
inline void Lookup<EMT>::assignComponents(entity::eid eid, T &&comp)
{
    using ClearT = std::decay_t<T>;
    auto &compVec = m_getCompVec<ClearT>();
    EMT compId = components::CType2EType<EMT, ClearT>::EType;
    unqLock<LockCond> lockComp {compVec.mtx};
    compVec.components.emplace_back(std::forward<ClearT>(comp));
    compVec.entityRefs.push_back(eid);

    unqLock<LockCond> lockEnt {m_entities.mtx};
    m_entities.data[eid].insert({compId, compVec.components.size() - 1});
};
} // namespace lbr::ecs::lookup
