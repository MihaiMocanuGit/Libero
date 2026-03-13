# ECS documentation (WIP)

This module is composed of three main parts: Entity, Components and the Lookup class.

## Entity

It's just a struct containing an `uint32_t` id. Usually, the entity id is shortened to `eid`.
Additionally, `neid` is defined as a special `eid`, used to refer to an invalid entity id.

When referring to a number of entities or to an entity index in some data structure, the `SizeEid`
type is usually used. But, as `SizeEid` and `eid` denote the same type, the distinction between
these two types is not that well-defined.

## Components

This is the *pieces de resistance* of the ECS module. It defines the necessary steps to register all
the available components into the ECS module. Everything is done at compile-time with no RTTI. This
is a double-edged sword, so it's important to know if this is appropriate for your use case. E.q.
better performance for longer compile times. Can't define custom components at run-time, so
components cannot be registered in runtime loaded shared libs.

The main advantage of this module is that the components don't have to be changed in any way to work
with this system. In order to register them with the system, one must define an enum containing the
name of all the available components. This enum must satisfy the `EMetaType` concept. Moreover, for
every component, two templated structs must be specialized with the appropriate enum and concrete
types. More details about this will come up later.

Note that an up to date example should always be available at the start of the
`tests/Test_Lookup.cpp` file.

### Explaining the types 

The ECS system defines three types in the context of components:
1. `EMetaType`
1. `EType`
1. `CType`

Let's start with `EMetaType`. This is a concept that restricts an enum containing the name of all
available components to the appropriate structure. Note that `EMetaType` comes from `EnumMetaType`
and represents the type of an enum. Take for example:

```c++
enum class EnumTypes : SizeEType // It is important to have SizeEType as its underlying type.
{
    Transform = 0, // The first field must start at 0. An enum with only countEType = 0 is also
                   // valid, albeit untested.
    Boundary,
    Controllable,
    countEType, // Important to be after all components.
    voidEType,  // Other values can be defined after countEType, but the ECS module won't know about
                // them.
};
static_assert(EMetaType<EnumTypes>, "EnumTypes is not an EMetaType");
```

This enum called `EnumTypes` satisfies the `EMetaType` concept. 

Moving on to the next type, as the naming implies, each of the field from `EnumTypes` is an
EnumType, that is an `EType`. All `Etypes` in an `EMetaType` (or `EMT` for short) must span a
continuous range of integer values, starting at 0 and ending at `countEType`. All that remains now
is to link the `ETypes` with the next type: `CType` (meaning either `ComponentType` or
`ConcreteType`) 

Hence, an `CType` is the alias to the concrete component class defined by a user of this library. To
be more exact, `CType` is actually a concept that checks for the appropriate declarations.

Say a user needs to make this ECS module work with the following Components:

```c++
struct Transform
{
    utl::Vec3f pos;
    utl::Vec3f rot;
    utl::Vec3f size;
};

struct Boundary
{
    enum class Type
    {
        BOX,              // lengths equal to size
        CYLINDRICAL,      // r=max(x1,x2), h=x3
        ELIPTIC_CYLINDER, // a=x1, b=x2, h=x3
    } type;
    utl::Vec3f size;
};

struct Controllable
{
    bool userControlled;
    utl::Vec3f stepSize;
};
```

As it has been observed, the appropriate `EMetaType` enum has already been defined. To finalize the
registration, specializations with the appropriate types for the following structs need are defined: 

```c++
namespace lbr::ecs::components {
template <EMetaType EMT, EMT EType>
struct EType2CType
{
    // using CType = Component;
};

template <EMetaType EMT, CType<EMT> T>
struct CType2EType
{
    // static constexpr EnumTypes EType = EnumTypes::Component;
};
```

So, the specializations for the given example would look like:
```c++
// Only EType2CType and CType2EType have to be in the lbr::ecs::components namespace. The components
// themselves and the EMetaType can be defined outside, but they can also be places inside this
// namespace
namespace lbr::ecs::components
{

// TRANSFORM
template <>
struct EType2CType<EnumTypes, EnumTypes::Transform>
{
    using CType = Transform;
};
template <>
struct CType2EType<EnumTypes, Transform>
{
    static constexpr EnumTypes EType = EnumTypes::Transform;
};

// BOUNDARY
static_assert(CType<Transform, EnumTypes>, "Transform is not a CType");
template <>
struct EType2CType<EnumTypes, EnumTypes::Boundary>
{
    using CType = Boundary;
};
template <>
struct CType2EType<EnumTypes, Boundary>
{
    static constexpr EnumTypes EType = EnumTypes::Boundary;
};
static_assert(CType<Boundary, EnumTypes>, "Boundary is not a CType");

// CONTROLLABLE
template <>
struct EType2CType<EnumTypes, EnumTypes::Controllable>
{
    using CType = Controllable;
};
template <>
struct CType2EType<EnumTypes, Controllable>
{
    static constexpr EnumTypes EType = EnumTypes::Controllable;
};
static_assert(CType<Controllable, EnumTypes>, "Boundary is not a CType");

} // namespace lbr::ecs::components
```

And this is all one needs to do.

## Lookup
This class is the one responsible for holding and managing all entities and components. It can be
used either asynchronously or concurrently, depending on your own requirements. To use any member
method asynchronously, just set the template parameter `<bool LockCond>` to true and false
otherwise. 

Special care needs to be taken in the asynchronous case if you call another Lookup member method
inside a member method that accepts a functor.

Take for example the following function, that needs a functor:
```c++
template <bool LockCond, CType<EMT> T>
void readAllComponents(std::invocable<const T &> auto funct) const;
```

Which might be used like so:
```c++
lookup.readAllComponents<true, Boundary>([](const Boundary &){
.........
    if (lookup.numberOfComponents<true, Boundary>() > 123)
........
});
```

But this will lead to a deadlock. To remedy this, use instead its non asynchronous version:
```c++
lookup.readAllComponents<true, Boundary>([](const Boundary &){
.........
    if (lookup.numberOfComponents<false, Boundary>() > 123)
........
});
```

If the `numberOfComponents()` function would've been called for another CType, say `Transform`, then
`LockCond=true` should be used.

In principle, there's a shared mutex defined for every collection of elements of a given CType.
Similarly, there's a shared mutex for the collection of entities.

It is important to know that modifications that are localized to a single element at a time do not
unique lock the whole collection. In this case, only a shared lock is applied to the whole set. A
not so obvious statement is that adding or removing a component can modify the whole collection, so
in that case, unique locks are applied.

A more in-depth documentation to the locking strategy will be created later on. 

The `eids` are not stable w.r.t the removals of some entities. That is, the removal of `N` entities
will change the `eids` of at most `N` other entities. To be more precise, the `eids` of the last `N`
entities will be changed to the `eids` of the removed entities. (Swap and Erase idiom)

## Final Notes

While the concepts are defined to make sure that a user defines everything that's needed, they
aren't foolproof. Some details aren't checked for now, even though the library code assumes
them to be present. For example, currently `EMetaType` doesn't check if all the `ETypes` are defined
continuously, it only verifies that it has a zero `EType` and that `EMT::countEtype` is present.


