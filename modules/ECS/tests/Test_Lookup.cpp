#include "Libero/ECS/Components.hpp"
#include "Libero/ECS/Entity.hpp"
#include "Libero/ECS/Lookup.hpp"
#include "Libero/Utilities/Vec.hpp"

#include <catch2/catch_test_macros.hpp>
#include <numeric>

using namespace lbr;
using namespace lbr::ecs::entity;
using namespace lbr::ecs::components;
using namespace lbr::ecs::lookup;

// Only EType2CType and CType2EType have to be in the lbr::ecs::components namespace. The components
// themselves and the EMetaType can be defined outside, but it makes more sense if all of them are
// grouped under the same namespace
namespace lbr::ecs::components
{
enum class EnumTypes : SizeEType
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

struct Transform
{
    utl::Vec3f pos;
    // uint8_t padding1;
    utl::Vec3f rot;
    // uint8_t padding2;
    utl::Vec3f size;
    // uint8_t padding2;
};
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
static_assert(CType<Transform, EnumTypes>, "Transform is not a CType");

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

struct Controllable
{
    bool userControlled;
    utl::Vec3f stepSize;
};
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

TEST_CASE("Entity Creation", "[ECS][Lookup]")
{
    Lookup<EnumTypes> lk;
    Entity ent0 = lk.createEntity<true>();
    REQUIRE(ent0.id == 0);
    Entity ent1 = lk.createEntity<true>();
    REQUIRE(ent1.id == 1);
}

TEST_CASE("Component Creation", "[ECS][Lookup]")
{
    Lookup<EnumTypes> lk;
    Entity ent {lk.createEntity<true>()};

    // Testing with rvalue ref
    lk.assignComponents<true>(ent.id, Transform {.pos = {1.0f, 2.0f, 3.0f},
                                                 .rot = {0.0f, 0.1f, 0.2f},
                                                 .size = {100.0f, 200.0f, 300.0f}});
    bool invoked {false};
    REQUIRE(lk.readComponent<true, Transform>(ent.id,
                                              [&invoked](const Transform &comp)
                                              {
                                                  invoked = true;
                                                  REQUIRE(comp.pos.x[0] == 1.0f);
                                                  REQUIRE(comp.rot.x[0] == 0.0f);
                                                  REQUIRE(comp.size.x[0] == 100.0f);
                                              }));
    REQUIRE(invoked);
    REQUIRE(lk.hasComponent<true, Transform>(ent.id));
    REQUIRE(not lk.hasComponent<true, Boundary>(ent.id));
    REQUIRE(not lk.hasComponent<true, Controllable>(ent.id));

    // Testing with lvalue
    Boundary bd {.type = Boundary::Type::ELIPTIC_CYLINDER, .size = {50.0f, 100.0f, 150.0f}};
    lk.assignComponents<true>(ent.id, bd);
    invoked = false;
    REQUIRE(lk.readComponent<true, Boundary>(ent.id,
                                             [&invoked](const Boundary &comp)
                                             {
                                                 invoked = true;
                                                 REQUIRE(comp.size.x[0] == 50.0f);
                                             }));
    REQUIRE(invoked);
    REQUIRE(lk.hasComponent<true, Transform>(ent.id));
    REQUIRE(lk.hasComponent<true, Boundary>(ent.id));
    REQUIRE(not lk.hasComponent<true, Controllable>(ent.id));
}

TEST_CASE("Multiple entities", "[ECS][Lookup]")
{
    constexpr unsigned NO_ENT {1'000};
    Lookup<EnumTypes> lk;
    for (unsigned i {0}; i < NO_ENT; ++i)
    {
        Entity ent {lk.createEntity<true>()};
        lk.assignComponents<true>(ent.id,
                                  Transform {.pos = utl::Vec3f::mul(i, {0.2f, 0.3f, 0.5f}),
                                             .rot = utl::Vec3f::mul(i, {0.7f, 0.11f, 0.13f}),
                                             .size = utl::Vec3f::mul(i, {0.17f, 0.19f, 0.23f})});
        if (i < NO_ENT / 2)
            lk.assignComponents<true>(ent.id,
                                      Boundary {.type = Boundary::Type::BOX,
                                                .size = utl::Vec3f::mul(i, {0.7f, 0.8f, 0.9f})});
    }

    unsigned i {0};
    lk.modifyAllComponents<true, Transform>(
        [&i](const Transform &comp)
        {
            REQUIRE(comp.pos.x[0] == i * 0.2f);
            REQUIRE(comp.rot.x[0] == i * 0.7f);
            REQUIRE(comp.size.x[0] == i * 0.17f);
            i++;
        });
    REQUIRE(i == NO_ENT);
    i = 0;
    lk.modifyAllComponents<true, Boundary>(
        [&i](const Boundary &comp)
        {
            REQUIRE(comp.size.x[0] == i * 0.7f);
            i++;
        });
    REQUIRE(i == NO_ENT / 2);
}

TEST_CASE("Read Component Group", "[ECS][Lookup]")
{
    constexpr unsigned NO_ENT {1'000};
    Lookup<EnumTypes> lk;
    for (unsigned i {0}; i < NO_ENT; ++i)
    {
        Entity ent {lk.createEntity<true>()};
        lk.assignComponents<true>(ent.id,
                                  Transform {.pos = utl::Vec3f::mul(i, {0.2f, 0.3f, 0.5f}),
                                             .rot = utl::Vec3f::mul(i, {0.7f, 0.11f, 0.13f}),
                                             .size = utl::Vec3f::mul(i, {0.17f, 0.19f, 0.23f})});
        if (i < NO_ENT / 2)
            lk.assignComponents<true>(ent.id,
                                      Boundary {.type = Boundary::Type::BOX,
                                                .size = utl::Vec3f::mul(i, {0.7f, 0.8f, 0.9f})});
    }

    SECTION("Single component, all entities")
    {
        unsigned i {0};
        lk.readGroupOfComponents<true, Transform>(
            [&i](Entity ent, const Transform &comp)
            {
                REQUIRE(comp.pos.x[0] == i * 0.2f);
                REQUIRE(comp.rot.x[0] == i * 0.7f);
                REQUIRE(comp.size.x[0] == i * 0.17f);
                REQUIRE(ent.id < NO_ENT);
                i++;
            });
        REQUIRE(i == NO_ENT);
    }

    SECTION("Single component, some entities")
    {
        unsigned i {0};
        lk.readGroupOfComponents<true, Boundary>(
            [&i](Entity ent, const Boundary &comp)
            {
                REQUIRE(comp.size.x[0] == i * 0.7f);
                REQUIRE(ent.id < NO_ENT / 2);
                i++;
            });
        REQUIRE(i == NO_ENT / 2);
    }

    SECTION("Single component, no matching entity")
    {
        unsigned i {0};
        lk.readGroupOfComponents<true, Controllable>(
            [&i]([[maybe_unused]] Entity ent, [[maybe_unused]] const Controllable &comp) { i++; });
        REQUIRE(i == 0);
    }

    SECTION("Both components")
    {
        unsigned i {0};
        lk.readGroupOfComponents<true, Transform, Boundary>(
            [&i](Entity ent, const Transform &tr, const Boundary &bd)
            {
                REQUIRE(tr.pos.x[0] == i * 0.2f);
                REQUIRE(tr.rot.x[0] == i * 0.7f);
                REQUIRE(tr.size.x[0] == i * 0.17f);
                REQUIRE(bd.size.x[0] == i * 0.7f);
                REQUIRE(ent.id < NO_ENT / 2);
                i++;
            });
        REQUIRE(i == NO_ENT / 2);
    }

    SECTION("Both components, swapped order")
    {
        unsigned i {0};
        lk.readGroupOfComponents<true, Boundary, Transform>(
            [&i](Entity ent, const Boundary &bd, const Transform &tr)
            {
                REQUIRE(tr.pos.x[0] == i * 0.2f);
                REQUIRE(tr.rot.x[0] == i * 0.7f);
                REQUIRE(tr.size.x[0] == i * 0.17f);
                REQUIRE(bd.size.x[0] == i * 0.7f);
                REQUIRE(ent.id < NO_ENT / 2);
                i++;
            });
        REQUIRE(i == NO_ENT / 2);
    }

    SECTION("Both components plus one with no entity associated")
    {
        unsigned i {0};
        lk.readGroupOfComponents<true, Transform, Boundary, Controllable>(
            [&i]([[maybe_unused]] Entity ent, [[maybe_unused]] const Transform &tr,
                 [[maybe_unused]] const Boundary &bd, [[maybe_unused]] const Controllable &ctrl)
            { i++; });
        REQUIRE(i == 0);
    }
}

TEST_CASE("Remove entities", "[ECS][Lookup]")
{
    constexpr unsigned NO_ENT {1'000};
    Lookup<EnumTypes> lk;
    for (unsigned i {0}; i < NO_ENT; ++i)
    {
        Entity ent {lk.createEntity<true>()};
        lk.assignComponents<true>(ent.id,
                                  Transform {.pos = utl::Vec3f::mul(i, {0.2f, 0.3f, 0.5f}),
                                             .rot = utl::Vec3f::mul(i, {0.7f, 0.11f, 0.13f}),
                                             .size = utl::Vec3f::mul(i, {0.17f, 0.19f, 0.23f})});
        if (i < NO_ENT / 2)
            lk.assignComponents<true>(ent.id,
                                      Boundary {.type = Boundary::Type::BOX,
                                                .size = utl::Vec3f::mul(i, {0.7f, 0.8f, 0.9f})});
    }

    SECTION("Remove none")
    {
        lk.removeEntities<true>({});
        REQUIRE(lk.numberOfEntities<true>() == NO_ENT);
        REQUIRE(lk.numberOfComponents<true, Transform>() == lk.numberOfEntities<true>());
        REQUIRE(lk.numberOfComponents<true, Boundary>() == NO_ENT / 2);

        lk.readGroupOfComponents<true, Transform>([](Entity ent, const auto &)
                                                  { REQUIRE(ent.id < NO_ENT); });
        lk.readGroupOfComponents<true, Boundary>([](Entity ent, const auto &)
                                                 { REQUIRE(ent.id < NO_ENT); });
    }

    SECTION("Remove all")
    {
        std::vector<eid> eids(NO_ENT, 0);
        std::iota(eids.begin(), eids.end(), 0);
        lk.removeEntities<true>(eids);
        REQUIRE(lk.numberOfEntities<true>() == 0);
        REQUIRE(lk.numberOfComponents<true, Transform>() == 0);
        REQUIRE(lk.numberOfComponents<true, Boundary>() == 0);

        lk.readGroupOfComponents<true, Transform>([](Entity, const auto &) { REQUIRE(false); });
        lk.readGroupOfComponents<true, Boundary>([](Entity, const auto &) { REQUIRE(false); });
    }

    SECTION("Remove first half")
    {
        std::vector<eid> eids(NO_ENT / 2, 0);
        std::iota(eids.begin(), eids.end(), 0);
        lk.removeEntities<true>(eids);
        REQUIRE(lk.numberOfEntities<true>() == NO_ENT - eids.size());
        REQUIRE(lk.numberOfComponents<true, Transform>() == lk.numberOfEntities<true>());
        REQUIRE(lk.numberOfComponents<true, Boundary>() == 0);

        lk.readGroupOfComponents<true, Transform>(
            [&lk](Entity ent, const auto &) { REQUIRE(ent.id < lk.numberOfEntities<false>()); });
        lk.readGroupOfComponents<true, Boundary>([](Entity, const auto &) { REQUIRE(false); });
    }

    SECTION("Remove single, but repeated eid")
    {
        std::vector<eid> eids(NO_ENT, 0);
        lk.removeEntities<true>(eids);
        REQUIRE(lk.numberOfEntities<true>() == NO_ENT - 1);
        REQUIRE(lk.numberOfComponents<true, Transform>() == lk.numberOfEntities<true>());
        REQUIRE(lk.numberOfComponents<true, Boundary>() == NO_ENT / 2 - 1);

        lk.readGroupOfComponents<true, Transform>(
            [&lk](Entity ent, const auto &) { REQUIRE(ent.id < lk.numberOfEntities<false>()); });
        lk.readGroupOfComponents<true, Boundary>([](Entity ent, const auto &)
                                                 { REQUIRE(ent.id <= NO_ENT / 2 - 1); });
    }
}
