#include "Libero/ECS/SystemCRTP.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>

class Derived_int : public SystemCRTP<Derived_int>
{
    using Base = SystemCRTP<Derived_int>;

  public:
    int test = 0;
    explicit Derived_int(uint64_t runEveryMS = 0) noexcept : Base(runEveryMS) {}

  private:
    int run_crtp(uint64_t) { return ++test; }
    friend Base;
};

class Derived_void : public SystemCRTP<Derived_void>
{
    using Base = SystemCRTP<Derived_void>;

  public:
    int test = 0;
    explicit Derived_void(uint64_t runEveryMS = 0) noexcept : Base(runEveryMS) {}

  private:
    void run_crtp(uint64_t)
    {
        test++;
        return;
    }
    friend Base;
};

class Derived_intRef : public SystemCRTP<Derived_intRef>
{
    using Base = SystemCRTP<Derived_intRef>;

  public:
    int test = 0;
    explicit Derived_intRef(uint64_t runEveryMS = 0) noexcept : Base(runEveryMS) {}

  private:
    int &run_crtp(uint64_t) { return ++test; }
    friend Base;
};

class Derived_intCRef : public SystemCRTP<Derived_intCRef>
{
    using Base = SystemCRTP<Derived_intCRef>;

  public:
    int test = 0;
    explicit Derived_intCRef(uint64_t runEveryMS = 0) noexcept : Base(runEveryMS) {}

  private:
    const int &run_crtp(uint64_t) { return ++test; }
    friend Base;
};

class Derived_intCRefC : public SystemCRTP<Derived_intCRefC>
{
    using Base = SystemCRTP<Derived_intCRefC>;

  public:
    mutable int test = 0;

    explicit Derived_intCRefC(uint64_t runEveryMS = 0) noexcept : Base(runEveryMS) {}

  private:
    const int &run_crtp(uint64_t) const { return ++test; }
    friend Base;
};

TEST_CASE("SystemCRTP usage", "[ECS][SystemCRTP]")
{
    Derived_int dint {100};
    // first run is always called
    REQUIRE(dint.run(0) == 1);
    // second run is too soon to be called
    REQUIRE(not dint.run(1).has_value());
    REQUIRE(dint.test == 1);
    // this is the last time it won't be called
    REQUIRE(not dint.run(99).has_value());
    REQUIRE(dint.test == 1);
    // this is the first time it will be called
    REQUIRE(dint.run(100) == 2);

    Derived_void dvoid {100};
    // first run is always called
    dvoid.run(0);
    REQUIRE(dvoid.test == 1);
    // second run is too soon to be called
    dvoid.run(1);
    REQUIRE(dvoid.test == 1);
    // this is the last time it won't be called
    dvoid.run(99);
    REQUIRE(dvoid.test == 1);
    // this is the first time it will be called
    dvoid.run(100);
    REQUIRE(dvoid.test == 2);

    Derived_intRef dintRef {100};
    // first run is always invoked
    REQUIRE(dintRef.run(0)->get() == 1);
    // second run is too soon to be invoked
    REQUIRE(not dintRef.run(1).has_value());
    REQUIRE(dintRef.test == 1);
    // this is the last time it won't be invoked
    REQUIRE(not dintRef.run(99).has_value());
    REQUIRE(dintRef.test == 1);
    // this is the first time it will be invoked
    REQUIRE(dintRef.run(100)->get() == 2);

    Derived_intCRef dintCRef {100};
    // first run is always invoked
    REQUIRE(dintCRef.run(0)->get() == 1);
    // second run is too soon to be invoked
    REQUIRE(not dintCRef.run(1).has_value());
    REQUIRE(dintCRef.test == 1);
    // this is the last time it won't be invoked
    REQUIRE(not dintCRef.run(99).has_value());
    REQUIRE(dintCRef.test == 1);
    // this is the first time it will be invoked
    REQUIRE(dintCRef.run(100)->get() == 2);

    Derived_intCRefC dintCRefC {100};
    // first run is always invoked
    REQUIRE(dintCRefC.run(0)->get() == 1);
    // second run is too soon to be invoked
    REQUIRE(not dintCRefC.run(1).has_value());
    REQUIRE(dintCRefC.test == 1);
    // this is the last time it won't be invoked
    REQUIRE(not dintCRefC.run(99).has_value());
    REQUIRE(dintCRefC.test == 1);
    // this is the first time it will be invoked
    REQUIRE(dintCRefC.run(100)->get() == 2);
}
