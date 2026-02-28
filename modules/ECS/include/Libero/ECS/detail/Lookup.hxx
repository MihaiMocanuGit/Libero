#pragma once

#include "../Components.hpp"
#include "../Entity.hpp"

#include <atomic>
#include <boost/container/flat_map.hpp>
#include <boost/dynamic_bitset.hpp>
#include <shared_mutex>
#include <tuple>
#include <utility>
#include <vector>

namespace lbr::ecs::lookup::detail
{

using namespace lbr::ecs;

using SizeEid = entity::eid;

// TODO: move them to a TMP utility header.
template <components::EMetaType EMT>
using ETypeTuple =
    decltype([]<SizeEType... IETypes>(std::integer_sequence<SizeEType, IETypes...>)               //
             {                                                                                    //
                 return std::tuple<std::integral_constant<EMT, static_cast<EMT>(IETypes)>...> {}; //
             }                                                                                    //
             ( std::make_integer_sequence<SizeEType, std::to_underlying(EMT::countEType)> {}));   //

template <components::EMetaType EMT, template <EMT> class Wrapper>
using ETypeTupleWrap =
    decltype([]<SizeEType... IETypes>(std::integer_sequence<SizeEType, IETypes...>)             //
             {                                                                                  //
                 return std::tuple<Wrapper<static_cast<EMT>(IETypes)>...> {};                   //
             }                                                                                  //
             ( std::make_integer_sequence<SizeEType, std::to_underlying(EMT::countEType)> {})); //

template <typename EMT, typename F>
void ETypeRT2CT(EMT runtime, F &&f)
{
    static constexpr auto N = std::to_underlying(EMT::countEType);
    [&]<SizeEType... IETypes>(std::integer_sequence<SizeEType, IETypes...>)
    {
        bool matched =
            ((runtime == static_cast<EMT>(IETypes)
                  ? (std::forward<F>(f).template operator()<static_cast<EMT>(IETypes)>(), true)
                  : false) ||
             ...);
        [[assume(matched)]];
    }(std::make_integer_sequence<SizeEType, N> {});
}

template <components::EMetaType EMT>
struct Components
{
    template <EMT EType>
    struct TypeData
    {
        using CompT = typename components::EType2CType<EMT, EType>::CType;
        using CompVecT = std::vector<CompT>;
        using EntRVecT = std::vector<entity::eid>;
        std::vector<CompT> components;
        std::vector<entity::eid> entityRefs;
    };
    using Types = ETypeTupleWrap<EMT, TypeData>;
    mutable std::array<std::shared_mutex, std::to_underlying(EMT::countEType)> mtxs;

    // clang-format off
      /*
       * CompTN |__components: c1   c2  ... ckN       |
       *        |__entityRefs: 0    1                 |
       *                                              |
       * ....................                         |
       *                                              |
       * CompT2 |__components: c1  ...   ck2          |
       *        |__entityRefs: 2                      |
       *                                              |
       * CompT1 |__components: c1   c2   c3 ... ck1   |
       *        |__entityRefs: 0    1    2            |
       *                                              |
       * CompT0 |__components: c1   c2   ... ck0      |
       *        |__entityRefs: 0    1                 |
       *              Indexes: 0    1     2  ...      | mtx
       */
    // clang-format on
    Types types;

    template <EMT EType>
    SizeEid insert(entity::eid eid, components::EType2CType<EMT, EType>::CType &&comp)
    {
        SizeEid compRef {size<EType>()};
        getComponents<EType>().push_back(std::move(comp));
        getEntityRefs<EType>().push_back(eid);
        return compRef;
    };

    template <EMT EType>
    std::shared_mutex &getMutex() const noexcept
    {
        return mtxs[std::to_underlying(EType)];
    }

    template <EMT EType>
    const typename TypeData<EType>::CompVecT &getComponents() const noexcept
    {
        return std::get<TypeData<EType>>(types).components;
    }

    template <EMT EType>
    typename TypeData<EType>::CompVecT &getComponents() noexcept
    {
        return std::get<TypeData<EType>>(types).components;
    }

    template <EMT EType>
    const typename TypeData<EType>::EntRVecT &getEntityRefs() const noexcept
    {
        return std::get<TypeData<EType>>(types).entityRefs;
    }

    template <EMT EType>
    typename TypeData<EType>::EntRVecT &getEntityRefs() noexcept
    {
        return std::get<TypeData<EType>>(types).entityRefs;
    }

    template <EMT EType>
    void swap(SizeEid comp1, SizeEid comp2)
    {
        std::swap(getComponents<EType>()[comp1], getComponents<EType>()[comp2]);
        std::swap(getEntityRefs<EType>()[comp1], getEntityRefs<EType>()[comp2]);
    }

    template <EMT EType>
    void swapEnd(SizeEid comp, SizeEid endPos = 0)
    {
        swap<EType>(comp, size<EType>() - endPos - 1);
    }

    template <EMT EType>
    void updateEntityRefs(SizeEid comp, SizeEid ent)
    {
        getEntityRefs<EType>()[comp] = ent;
    };

    template <EMT EType>
    void removeLast(SizeEid numComponents = 1)
    {
        getComponents<EType>().erase(getComponents<EType>().end() - numComponents,
                                     getComponents<EType>().end());
        getEntityRefs<EType>().erase(getEntityRefs<EType>().end() - numComponents,
                                     getEntityRefs<EType>().end());
    };

    template <EMT EType>
    SizeEid size() const noexcept
    {
        return getComponents<EType>().size();
    }

    template <EMT EType>
    void reserve(SizeEid newCapacity) noexcept
    {
        getComponents<EType>().reserve(newCapacity);
        getEntityRefs<EType>().reserve(newCapacity);
    }

    void reserve(SizeEid newCapacity) noexcept
    {
        [&]<SizeEType... IETypes>(std::integer_sequence<SizeEType, IETypes...>)
        {
            (reserve<static_cast<EMT>(IETypes)>(newCapacity), ...);
        }(std::make_integer_sequence<SizeEType, std::to_underlying(EMT::countEType)> {});
    }
};

template <components::EMetaType EMT>
struct Entities
{
    static constexpr auto NONE {static_cast<SizeEid>(-1)};

    // Note: Using SizeEType instead of EType to be able to represent the NONE state.
    // TODO: Try to use atomic<SizeEType>
    using Col = std::array<SizeEid, std::to_underlying(EMT::countEType)>;

    // clang-format off
      /*
       * CompTN|  0 |  1 | -1 |...| kN |
       * ......
       * CompT2| -1 | -1 |  1 |...| -1 |
       * CompT1|  0 |  1 |  2 |...| k1 |
       * CompT0|  0 |  1 | -1 |...| k0 |
       *        Ent0 Ent1 Ent2 ... EntM
       */
    // clang-format on
    mutable std::shared_mutex mtx;
    std::vector<Col> compRefs;

    /**
     * \brief Creates numEntities Entities, with no components associated.
     * \returns The eid of the first entity. The range [eid, eid + numEntities - 1] represents
     all
     * added entities.
     * \lock Should be Unique Locked because a resize might happen during the operation. Shared
     lock
     * can be used if you know what you're doing (no resize, no back() read/modify, etc.)
     * */
    entity::eid create(SizeEid numEntities = 1)
    {
        auto firstEid {static_cast<entity::eid>(compRefs.size())};
        static Col col;
        col.fill(NONE); // this should get optimised away.
        compRefs.insert(compRefs.end(), numEntities, col);
        return firstEid;
    };

    void swap(entity::eid eid1, entity::eid eid2)
    {
        std::swap(getComponentRefs(eid1), getComponentRefs(eid2));
    };

    // NOTE: it is assumed that eid is not in the region [endPos, end]
    void swapEnd(entity::eid eid, SizeEid endPos = 0) { swap(eid, size() - endPos - 1); };

    entity::eid removeLast(SizeEid numEntities = 1)
    {
        auto firstEid {static_cast<entity::eid>(compRefs.size() - numEntities + 1)};
        compRefs.erase(compRefs.end() - numEntities, compRefs.end());
        return firstEid;
    };

    template <EMT EType>
    SizeEid getComponentRefs(entity::eid eid) const
    {
        return compRefs[eid][std::to_underlying(EType)];
    }

    template <EMT EType>
    SizeEid &getComponentRefs(entity::eid eid)
    {
        return compRefs[eid][std::to_underlying(EType)];
    }

    const Col &getComponentRefs(entity::eid eid) const { return compRefs[eid]; }

    Col &getComponentRefs(entity::eid eid) { return compRefs[eid]; }

    template <EMT EType>
    bool containsComponents(entity::eid eid) const
    {
        return getComponentRefs<EType>(eid) != NONE;
    };

    template <EMT EType1, EMT EType2, EMT... ETypes>
    bool containsComponents(entity::eid eid) const
    {
        return containsComponents<EType1>(eid) && containsComponents<EType2>(eid) &&
               (containsComponents<ETypes>(eid) && ...);
    };

    template <EMT EType>
    SizeEid removeComponentRef(entity::eid eid)
    {
        SizeEid oldRef {getComponentRefs<EType>(eid)};
        getComponentRefs<EType>(eid) = NONE;
        return oldRef;
    }

    void reserve(SizeEid newCapacity) noexcept { compRefs.reserve(newCapacity); }

    SizeEid size() const noexcept { return compRefs.size(); }

    std::shared_mutex &getMutex() const noexcept { return mtx; }

    template <EMT EType>
    void assignComponent(entity::eid eid, SizeEid compRef)
    {
        getComponentRefs<EType>(eid) = compRef;
    }
};
} // namespace lbr::ecs::lookup::detail
