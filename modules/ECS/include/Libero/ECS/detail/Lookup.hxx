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

using namespace lbr::ecs;
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

template <components::EMetaType EMT, EMT EType>
struct ComponentData
{
    mutable std::shared_mutex mtx;
    std::vector<typename components::EType2CType<EMT, EType>::CType> components;
    std::vector<entity::eid> entityRefs;
};

template <components::EMetaType EMT>
struct Entities
{
    mutable std::shared_mutex mtx;
    using CompRefT = boost::container::flat_map<EMT, ComponentVectorIt::iterator>;
    std::vector<CompRefT> data;
    entity::Entity entity(entity::eid eid) const { return {.id = eid}; }
};

template <components::EMetaType EMT>
struct CompList
{
  private:
    template <EMT EType, template <components::EMetaType, EMT> class DataWrapper>
    struct Node
    {
      private:
        using NodeDataType = DataWrapper<EMT, EType>;
        static constexpr auto NextEnumType = static_cast<EMT>(std::to_underlying(EType) + 1);

      public:
        using Type = decltype(std::tuple_cat(
            std::declval<std::tuple<NodeDataType>>(),
            std::declval<typename Node<NextEnumType, DataWrapper>::Type>()));
    };

    // End
    template <template <components::EMetaType, EMT> class DataWrapper>
    struct Node<EMT::countEType, DataWrapper>
    {
        using Type = std::tuple<>;
    };

    static constexpr EMT FirstEnumType = static_cast<EMT>(0);

  public:
    template <EMT EType>
    struct Identity
    {
        static constexpr EMT x = EType;
    };

    template <template <components::EMetaType, EMT> class DataWrapper = Identity>
    using Root = Node<FirstEnumType, DataWrapper>::Type;
};

} // namespace lbr::ecs::lookup::detail
