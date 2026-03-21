#include "Libero/Utilities/SpaceMapping.hpp"
#include "Libero/Utilities/Vec.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
using namespace lbr::utl;
using namespace lbr::utl::spacemapping;

TEST_CASE("Relative", "[SpaceMapping]")
{
    Rect3f baseSpace = {.ul = {-1, -1, -1}, .lr = {1, 1, 1}};
    Rect3f targetSpace = {.ul = {-2, -2, -2}, .lr = {2, 2, 2}};
    SECTION("Origin")
    {
        Vec3f x = {0, 0, 0};
        Vec3f x_prime = relativeSpaceMapping(x, baseSpace, targetSpace);
        REQUIRE(x_prime == x);
    }
    SECTION("Middle")
    {
        Vec3f x = {-0.5, -0.5, -0.5};
        Vec3f x_prime = relativeSpaceMapping(x, baseSpace, targetSpace);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("x' = ({}, {}, {})", x_prime[0], x_prime[1], x_prime[2]));
        REQUIRE(x_prime == Vec3f {-1, -1, -1});
    }
    SECTION("Boundary")
    {
        Vec3f x = {-1, -1, -1};
        Vec3f x_prime = relativeSpaceMapping(x, baseSpace, targetSpace);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("x' = ({}, {}, {})", x_prime[0], x_prime[1], x_prime[2]));
        REQUIRE(x_prime == Vec3f {-2, -2, -2});
    }
    SECTION("Outside Boundary")
    {
        Vec3f x = {-2, -2, -2};
        Vec3f x_prime = relativeSpaceMapping(x, baseSpace, targetSpace);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("x' = ({}, {}, {})", x_prime[0], x_prime[1], x_prime[2]));
        REQUIRE(x_prime == Vec3f {-4, -4, -4});
    }
    SECTION("Different coeff")
    {
        Vec3f x = {-2, +1, -0.5};
        Vec3f x_prime = relativeSpaceMapping(x, baseSpace, targetSpace);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("x' = ({}, {}, {})", x_prime[0], x_prime[1], x_prime[2]));
        REQUIRE(x_prime == Vec3f {-4, +2, -1});
    }
    SECTION("Swapped target with base space")
    {
        Vec3f x = {-4, +2, -1};
        Vec3f x_prime = relativeSpaceMapping(x, targetSpace, baseSpace);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("x' = ({}, {}, {})", x_prime[0], x_prime[1], x_prime[2]));
        REQUIRE(x_prime == Vec3f {-2, +1, -0.5});
    }
}

TEST_CASE("HyperplaneToSpace", "[SpaceMapping]")
{
    SECTION("n = e_0, x = 0")
    {
        Vec3f x = {0, 0, 0};
        Vec3f O = {0, 0, 0};
        Vec3f n = {1, 0, 0};
        Vec2f x_prime = hyperplaneToSpaceMapping(x, O, n);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted x' = (0, 0)"));
        REQUIRE(x_prime == Vec2f {0, 0});
    }

    SECTION("n = e_1, x = 0")
    {
        Vec3f x = {0, 0, 0};
        Vec3f O = {0, 0, 0};
        Vec3f n = {0, 1, 0};
        Vec2f x_prime = hyperplaneToSpaceMapping(x, O, n);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted x' = (0, 0)"));
        REQUIRE(x_prime == Vec2f {0, 0});
    }

    SECTION("n = e_2, x = 0")
    {
        Vec3f x = {0, 0, 0};
        Vec3f O = {0, 0, 0};
        Vec3f n = {0, 0, 1};
        Vec2f x_prime = hyperplaneToSpaceMapping(x, O, n);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted x' = (0, 0)"));
        REQUIRE(x_prime == Vec2f {0, 0});
    }

    SECTION("n = e_0, x = e_1")
    {
        Vec3f x = {0, 1, 0};
        Vec3f O = {0, 0, 0};
        Vec3f n = {1, 0, 0};
        Vec2f x_prime = hyperplaneToSpaceMapping(x, O, n);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("n = ({}, {}, {})", n[0], n[1], n[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted x' = (0, 1)"));
        REQUIRE(x_prime == Vec2f {0, 1});
    }

    SECTION("n = e_0, x = e_2")
    {
        Vec3f x = {0, 0, 1};
        Vec3f O = {0, 0, 0};
        Vec3f n = {1, 0, 0};
        Vec2f x_prime = hyperplaneToSpaceMapping(x, O, n);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("n = ({}, {}, {})", n[0], n[1], n[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted x' = (-1, 0)"));
        REQUIRE(x_prime == Vec2f {1, 0});
    }

    SECTION("n = e_1, x = e_0")
    {
        Vec3f x = {1, 0, 0};
        Vec3f O = {0, 0, 0};
        Vec3f n = {0, 1, 0};
        Vec2f x_prime = hyperplaneToSpaceMapping(x, O, n);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("n = ({}, {}, {})", n[0], n[1], n[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted (1, 0)"));
        REQUIRE(x_prime == Vec2f {1, 0});
    }

    SECTION("n = e_2, x = e_0")
    {
        Vec3f x = {1, 0, 0};
        Vec3f O = {0, 0, 0};
        Vec3f n = {0, 0, 1};
        Vec2f x_prime = hyperplaneToSpaceMapping(x, O, n);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("n = ({}, {}, {})", n[0], n[1], n[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted (1, 0)"));
        REQUIRE(x_prime == Vec2f {1, 0});
    }

    SECTION("n = e_2, moving x in e_0 x e_1")
    {
        for (float i = 0.5f; i < 10.0f; i += 0.5f)
        {
            Vec3f x = {i, -i, 0};
            Vec3f O = {0, 0, 0};
            Vec3f n = {0, 0, 1};
            Vec2f x_prime = hyperplaneToSpaceMapping(x, O, n);
            INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
            INFO(std::format("n = ({}, {}, {})", n[0], n[1], n[2]));
            INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
            INFO(std::format("Wanted ({}, {})", i, -i));
            REQUIRE(x_prime == Vec2f {i, -i});
        }
    }
}

TEST_CASE("Projective", "[SpaceMapping]")
{
    SECTION("TopDown, x in origin")
    {
        Vec3f x = {0, 0, 0};  // point
        Vec3f n = {0, 0, -1}; // camera orientiation
        Vec3f c = {0, 0, 8};  // camera position
        Vec2f x_prime = projectiveMapping(x, n, c);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("n = ({}, {}, {})", n[0], n[1], n[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted (0, 0)"));
        REQUIRE(x_prime == Vec2f {0.0f, 0.0f});
    }

    SECTION("TopDown, x in e0")
    {
        Vec3f x = {1, 0, 0};  // point
        Vec3f n = {0, 0, -1}; // camera orientiation
        Vec3f c = {0, 0, 8};  // camera position
        Vec2f x_prime = projectiveMapping(x, n, c);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("n = ({}, {}, {})", n[0], n[1], n[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted (1/8, 0)"));
        REQUIRE(x_prime == Vec2f {1.0f / 8.0f, 0.0f});
    }

    SECTION("TopDown, x in 2*e0")
    {
        Vec3f x = {2, 0, 0};  // point
        Vec3f n = {0, 0, -1}; // camera orientiation
        Vec3f c = {0, 0, 8};  // camera position
        Vec2f x_prime = projectiveMapping(x, n, c);
        INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
        INFO(std::format("n = ({}, {}, {})", n[0], n[1], n[2]));
        INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
        INFO(std::format("Wanted (2/8, 0)"));
        REQUIRE(x_prime == Vec2f {2.0f / 8.0f, 0.0f});
    }

    SECTION("TopDown, moving x")
    {
        for (float i = 0.5f; i < 10.0f; i += 0.5f)
        {
            Vec3f x = {i, -i, 0}; // point
            Vec3f n = {0, 0, -1}; // camera orientiation
            Vec3f c = {0, 0, 8};  // camera position
            Vec2f x_prime = projectiveMapping(x, n, c);
            INFO(std::format("x = ({}, {}, {})", x[0], x[1], x[2]));
            INFO(std::format("n = ({}, {}, {})", n[0], n[1], n[2]));
            INFO(std::format("x' = ({}, {})", x_prime[0], x_prime[1]));
            INFO(std::format("Wanted ({}, {})", i / 8.0f, -i / 8.0f));
            REQUIRE(x_prime == Vec2f {i / 8.0f, -i / 8.0f});
        }
    }
}
