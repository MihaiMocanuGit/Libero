#pragma once

#include "Utilities/Vec.hpp"

#include <concepts>
namespace lbr::ecs::components
{
enum class EnumTypes
{
    Transform = 0,
    Boundary,
    Controlable,
#ifdef ECS_COMPONENTS_CUSTOM              // it should be a build system define
#include ECS_COMPONENTS_CUSTOM_PATH_TYPES // it should be a build system define
#endif
    count, // Important to be last
};

template <EnumTypes ComponentE>
struct Id2Type;

template <class CompT>
concept IsComponent = requires {
    { std::decay_t<CompT>::id } -> std::convertible_to<EnumTypes>;
};

struct Transform
{
    static constexpr EnumTypes id {EnumTypes::Transform};
    utl::Vec3f pos;
    // uint8_t padding1;
    utl::Vec3f rot;
    // uint8_t padding2;
    utl::Vec3f size;
    // uint8_t padding2;
};
template <>
struct Id2Type<EnumTypes::Transform>
{
    using Type = Transform;
};

struct Boundary
{
    static constexpr EnumTypes id {EnumTypes::Boundary};
    enum class Type
    {
        BOX,              // lengths equal to size
        CYLINDRICAL,      // r=max(x1,x2), h=x3
        ELIPTIC_CYLINDER, // a=x1, b=x2, h=x3
    } type;
    utl::Vec3f size;
};
template <>
struct Id2Type<EnumTypes::Boundary>
{
    using Type = Boundary;
};

struct Controlable
{
    static constexpr EnumTypes id {EnumTypes::Controlable};
    bool userControlled;
    utl::Vec3f stepSize;
};
template <>
struct Id2Type<EnumTypes::Controlable>
{
    using Type = Controlable;
};

} // namespace lbr::ecs::components

// It must be injected outside the namespace
#ifdef ECS_COMPONENTS_CUSTOM
#include ECS_COMPONENTS_CUSTOM_PATH_IMPL // it should be a build system define
#endif
