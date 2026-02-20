#pragma once

#include "../Components.hpp"
#include "../Entity.hpp"

#include <atomic>
#include <boost/container/flat_map.hpp>
#include <boost/dynamic_bitset.hpp>
#include <shared_mutex>
#include <utility>
#include <vector>

namespace lbr::ecs::lookup::detail
{

namespace ComponentVectorIt
{
// while they are identical, different names are used to signal the intent
using iterator = entity::eid;
using citerator = iterator;
} // namespace ComponentVectorIt

namespace EntityVectorIt
{
using iterator = entity::eid;
using citerator = iterator;
} // namespace EntityVectorIt

template <components::EnumTypes E>
struct ComponentData
{
    mutable std::shared_mutex mtx;
    std::vector<typename components::Id2Type<E>::Type> components;
    std::vector<entity::eid> entityRefs;
};

struct Entities
{
    mutable std::shared_mutex mtx;
    using CompRefT = boost::container::flat_map<components::EnumTypes, ComponentVectorIt::iterator>;
    std::vector<CompRefT> data;
    entity::Entity entity(entity::eid eid) const { return {.id = eid}; }
};

struct CompList
{
  private:
    template <components::EnumTypes N, template <components::EnumTypes> class DataWrapper>
    struct Node
    {
      private:
        using NodeDataType = DataWrapper<N>;
        static constexpr auto NextEnumType =
            static_cast<components::EnumTypes>(std::to_underlying(N) + 1);

      public:
        using Type = decltype(std::tuple_cat(
            std::declval<std::tuple<NodeDataType>>(),
            std::declval<typename Node<NextEnumType, DataWrapper>::Type>()));
    };

    // End
    template <template <components::EnumTypes> class DataWrapper>
    struct Node<components::EnumTypes::count, DataWrapper>
    {
        using Type = std::tuple<>;
    };

    static constexpr components::EnumTypes FirstEnumType = static_cast<components::EnumTypes>(0);

  public:
    template <components::EnumTypes N>
    struct Identity
    {
        static constexpr components::EnumTypes x = N;
    };

    template <template <components::EnumTypes> class DataWrapper = Identity>
    using Root = Node<FirstEnumType, DataWrapper>::Type;
};

} // namespace lbr::ecs::lookup::detail
