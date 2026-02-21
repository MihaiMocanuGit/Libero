#include "Libero/ECS/Components.hpp"
#include "Libero/ECS/Entity.hpp"
#include "Libero/ECS/Lookup.hpp"
#include "Libero/Utilities/Vec.hpp"

#include <catch2/catch_test_macros.hpp>
#include <numeric>

using namespace lbr;
using namespace lbr::ecs;
using namespace lookup;
TEST_CASE("Entity Creation", "[ECS][Lookup]")
{
    Lookup lk;
    entity::Entity ent0 = lk.createEntity<true>();
    REQUIRE(ent0.id == 0);
    entity::Entity ent1 = lk.createEntity<true>();
    REQUIRE(ent1.id == 1);
}

TEST_CASE("Component Creation", "[ECS][Lookup]")
{
    Lookup lk;
    entity::Entity ent {lk.createEntity<true>()};

    // Testing with rvalue ref
    lk.assignComponents<true>(ent.id, components::Transform {.pos = {1.0f, 2.0f, 3.0f},
                                                             .rot = {0.0f, 0.1f, 0.2f},
                                                             .size = {100.0f, 200.0f, 300.0f}});
    bool invoked {false};
    REQUIRE(
        lk.readComponent<true, components::Transform>(ent.id,
                                                      [&invoked](const components::Transform &comp)
                                                      {
                                                          invoked = true;
                                                          REQUIRE(comp.pos.x[0] == 1.0f);
                                                          REQUIRE(comp.rot.x[0] == 0.0f);
                                                          REQUIRE(comp.size.x[0] == 100.0f);
                                                      }));
    REQUIRE(invoked);
    REQUIRE(lk.hasComponent<true, components::Transform>(ent.id));
    REQUIRE(not lk.hasComponent<true, components::Boundary>(ent.id));
    REQUIRE(not lk.hasComponent<true, components::Controlable>(ent.id));

    // Testing with lvalue
    components::Boundary bd {.type = components::Boundary::Type::ELIPTIC_CYLINDER,
                             .size = {50.0f, 100.0f, 150.0f}};
    lk.assignComponents<true>(ent.id, bd);
    invoked = false;
    REQUIRE(
        lk.readComponent<true, components::Boundary>(ent.id,
                                                     [&invoked](const components::Boundary &comp)
                                                     {
                                                         invoked = true;
                                                         REQUIRE(comp.size.x[0] == 50.0f);
                                                     }));
    REQUIRE(invoked);
    REQUIRE(lk.hasComponent<true, components::Transform>(ent.id));
    REQUIRE(lk.hasComponent<true, components::Boundary>(ent.id));
    REQUIRE(not lk.hasComponent<true, components::Controlable>(ent.id));
}

TEST_CASE("Multiple entities", "[ECS][Lookup]")
{
    constexpr unsigned NO_ENT {1'000};
    Lookup lk;
    for (unsigned i {0}; i < NO_ENT; ++i)
    {
        entity::Entity ent {lk.createEntity<true>()};
        lk.assignComponents<true>(
            ent.id, components::Transform {.pos = utl::Vec3f::mul(i, {0.2f, 0.3f, 0.5f}),
                                           .rot = utl::Vec3f::mul(i, {0.7f, 0.11f, 0.13f}),
                                           .size = utl::Vec3f::mul(i, {0.17f, 0.19f, 0.23f})});
        if (i < NO_ENT / 2)
            lk.assignComponents<true>(
                ent.id, components::Boundary {.type = components::Boundary::Type::BOX,
                                              .size = utl::Vec3f::mul(i, {0.7f, 0.8f, 0.9f})});
    }

    unsigned i {0};
    lk.modifyAllComponents<true, components::Transform>(
        [&i](const components::Transform &comp)
        {
            REQUIRE(comp.pos.x[0] == i * 0.2f);
            REQUIRE(comp.rot.x[0] == i * 0.7f);
            REQUIRE(comp.size.x[0] == i * 0.17f);
            i++;
        });
    REQUIRE(i == NO_ENT);
    i = 0;
    lk.modifyAllComponents<true, components::Boundary>(
        [&i](const components::Boundary &comp)
        {
            REQUIRE(comp.size.x[0] == i * 0.7f);
            i++;
        });
    REQUIRE(i == NO_ENT / 2);
}

TEST_CASE("Read Component Group", "[ECS][Lookup]")
{
    constexpr unsigned NO_ENT {1'000};
    Lookup lk;
    for (unsigned i {0}; i < NO_ENT; ++i)
    {
        entity::Entity ent {lk.createEntity<true>()};
        lk.assignComponents<true>(
            ent.id, components::Transform {.pos = utl::Vec3f::mul(i, {0.2f, 0.3f, 0.5f}),
                                           .rot = utl::Vec3f::mul(i, {0.7f, 0.11f, 0.13f}),
                                           .size = utl::Vec3f::mul(i, {0.17f, 0.19f, 0.23f})});
        if (i < NO_ENT / 2)
            lk.assignComponents<true>(
                ent.id, components::Boundary {.type = components::Boundary::Type::BOX,
                                              .size = utl::Vec3f::mul(i, {0.7f, 0.8f, 0.9f})});
    }

    SECTION("Single component, all entities")
    {
        unsigned i {0};
        lk.readGroupOfComponents<true, components::Transform>(
            [&i](entity::Entity ent, const components::Transform &comp)
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
        lk.readGroupOfComponents<true, components::Boundary>(
            [&i](entity::Entity ent, const components::Boundary &comp)
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
        lk.readGroupOfComponents<true, components::Controlable>(
            [&i]([[maybe_unused]] entity::Entity ent,
                 [[maybe_unused]] const components::Controlable &comp) { i++; });
        REQUIRE(i == 0);
    }

    SECTION("Both components")
    {
        unsigned i {0};
        lk.readGroupOfComponents<true, components::Transform, components::Boundary>(
            [&i](entity::Entity ent, const components::Transform &tr,
                 const components::Boundary &bd)
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
        lk.readGroupOfComponents<true, components::Boundary, components::Transform>(
            [&i](entity::Entity ent, const components::Boundary &bd,
                 const components::Transform &tr)
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
        lk.readGroupOfComponents<true, components::Transform, components::Boundary,
                                 components::Controlable>(
            [&i]([[maybe_unused]] entity::Entity ent,
                 [[maybe_unused]] const components::Transform &tr,
                 [[maybe_unused]] const components::Boundary &bd,
                 [[maybe_unused]] const components::Controlable &ctrl) { i++; });
        REQUIRE(i == 0);
    }
}

TEST_CASE("Remove entities", "[ECS][Lookup]")
{
    constexpr unsigned NO_ENT {1'000};
    Lookup lk;
    for (unsigned i {0}; i < NO_ENT; ++i)
    {
        entity::Entity ent {lk.createEntity<true>()};
        lk.assignComponents<true>(
            ent.id, components::Transform {.pos = utl::Vec3f::mul(i, {0.2f, 0.3f, 0.5f}),
                                           .rot = utl::Vec3f::mul(i, {0.7f, 0.11f, 0.13f}),
                                           .size = utl::Vec3f::mul(i, {0.17f, 0.19f, 0.23f})});
        if (i < NO_ENT / 2)
            lk.assignComponents<true>(
                ent.id, components::Boundary {.type = components::Boundary::Type::BOX,
                                              .size = utl::Vec3f::mul(i, {0.7f, 0.8f, 0.9f})});
    }

    SECTION("Remove none")
    {
        lk.removeEntities<true>({});
        REQUIRE(lk.numberOfEntities<true>() == NO_ENT);
        REQUIRE(lk.numberOfComponents<true, components::Transform>() ==
                lk.numberOfEntities<true>());
        REQUIRE(lk.numberOfComponents<true, components::Boundary>() == NO_ENT / 2);

        lk.readGroupOfComponents<true, components::Transform>([](entity::Entity ent, const auto &)
                                                              { REQUIRE(ent.id < NO_ENT); });
        lk.readGroupOfComponents<true, components::Boundary>([](entity::Entity ent, const auto &)
                                                             { REQUIRE(ent.id < NO_ENT); });
    }

    SECTION("Remove all")
    {
        std::vector<entity::eid> eids(NO_ENT, 0);
        std::iota(eids.begin(), eids.end(), 0);
        lk.removeEntities<true>(eids);
        REQUIRE(lk.numberOfEntities<true>() == 0);
        REQUIRE(lk.numberOfComponents<true, components::Transform>() == 0);
        REQUIRE(lk.numberOfComponents<true, components::Boundary>() == 0);

        lk.readGroupOfComponents<true, components::Transform>([](entity::Entity, const auto &)
                                                              { REQUIRE(false); });
        lk.readGroupOfComponents<true, components::Boundary>([](entity::Entity, const auto &)
                                                             { REQUIRE(false); });
    }

    SECTION("Remove first half")
    {
        std::vector<entity::eid> eids(NO_ENT / 2, 0);
        std::iota(eids.begin(), eids.end(), 0);
        lk.removeEntities<true>(eids);
        REQUIRE(lk.numberOfEntities<true>() == NO_ENT - eids.size());
        REQUIRE(lk.numberOfComponents<true, components::Transform>() ==
                lk.numberOfEntities<true>());
        REQUIRE(lk.numberOfComponents<true, components::Boundary>() == 0);

        lk.readGroupOfComponents<true, components::Transform>(
            [&lk](entity::Entity ent, const auto &)
            { REQUIRE(ent.id < lk.numberOfEntities<false>()); });
        lk.readGroupOfComponents<true, components::Boundary>([](entity::Entity, const auto &)
                                                             { REQUIRE(false); });
    }

    SECTION("Remove single, but repeated eid")
    {
        std::vector<entity::eid> eids(NO_ENT, 0);
        lk.removeEntities<true>(eids);
        REQUIRE(lk.numberOfEntities<true>() == NO_ENT - 1);
        REQUIRE(lk.numberOfComponents<true, components::Transform>() ==
                lk.numberOfEntities<true>());
        REQUIRE(lk.numberOfComponents<true, components::Boundary>() == NO_ENT / 2 - 1);

        lk.readGroupOfComponents<true, components::Transform>(
            [&lk](entity::Entity ent, const auto &)
            { REQUIRE(ent.id < lk.numberOfEntities<false>()); });
        lk.readGroupOfComponents<true, components::Boundary>(
            [](entity::Entity ent, const auto &) { REQUIRE(ent.id <= NO_ENT / 2 - 1); });
    }
}
