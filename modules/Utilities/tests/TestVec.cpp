#include "Libero/Utilities/Vec.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace lbr::utl;

TEST_CASE("CanonicalBasis", "[Vec]")
{
    Vec3f::BaseType e = Vec3f::canonicalBasis();
    REQUIRE(e[0] == Vec3f {1, 0, 0});
    REQUIRE(e[1] == Vec3f {0, 1, 0});
    REQUIRE(e[2] == Vec3f {0, 0, 1});
}

TEST_CASE("Norm", "[Vec]")
{
    SECTION("v = 0")
    {
        Vec3f v = {0, 0, 0};
        REQUIRE(v.norm() == 0.0f);
        REQUIRE(v.norm() == Vec3f::norm(v));
        REQUIRE(sqrt(v.norm2()) == v.norm());
    }

    SECTION("v = e_0")
    {
        Vec3f v = {1, 0, 0};
        REQUIRE(v.norm() == 1.0f);
        REQUIRE(v.norm() == Vec3f::norm(v));
        REQUIRE(sqrt(v.norm2()) == v.norm());
    }

    SECTION("v = e_1")
    {
        Vec3f v = {0, 1, 0};
        REQUIRE(v.norm() == 1.0f);
        REQUIRE(v.norm() == Vec3f::norm(v));
        REQUIRE(sqrt(v.norm2()) == v.norm());
    }

    SECTION("v = e_2")
    {
        Vec3f v = {0, 0, 1};
        REQUIRE(v.norm() == 1.0f);
        REQUIRE(v.norm() == Vec3f::norm(v));
        REQUIRE(sqrt(v.norm2()) == v.norm());
    }
}
