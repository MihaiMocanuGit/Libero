#pragma once

#include "Libero/Utilities/Vec.hpp"

#include <concepts>
#include <cstdint>
#include <type_traits>
#include <utility>
namespace lbr::ecs::components
{

using UnderlyingEMetaType = uint8_t;

template <typename EMT>
concept EMetaType = requires {
    requires std::is_enum_v<EMT>;
    requires std::same_as<std::underlying_type_t<EMT>, UnderlyingEMetaType>;
    { EMT::countEType };
    // TODO: check that countEType's value and its own position in EMT coincide
};

template <EMetaType EMT, EMT EType>
struct EType2CType
{
    // using CType = CType_void;
    static_assert(false, "Trying to use unspecialized template");
};

namespace detail
{
template <EMetaType EMT, class CT>
consteval bool ExistsETypeWithCType()
{
    return []<std::size_t... ETypes>(std::index_sequence<ETypes...>) -> bool
    {
        return (std::same_as<typename EType2CType<EMT, static_cast<EMT>(ETypes)>::CType, CT> ||
                ...);
    }();
};
}; // namespace detail

template <class CT, typename EMT>
concept CType = requires {
    requires EMetaType<EMT>;
    detail::ExistsETypeWithCType<EMT, CT>();
};

template <EMetaType EMT, CType<EMT> T>
struct CType2EType
{
    // static constexpr EnumTypes EType = EnumTypes::voidE;
    static_assert(false, "Trying to use unspecialized template");
};

} // namespace lbr::ecs::components
