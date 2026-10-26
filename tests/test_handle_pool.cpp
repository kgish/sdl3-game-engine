#include <catch2/catch_test_macros.hpp>

#include "core/HandlePool.hpp"

using engine::HandlePool;

namespace {
struct TestTag;
using Pool = HandlePool<TestTag, int>;
} // namespace

TEST_CASE("Default handle is null", "[handle]")
{
    const Pool::HandleType handle;
    CHECK_FALSE(handle.valid());
    Pool pool;
    CHECK(pool.get(handle) == nullptr);
}

TEST_CASE("Inserted values resolve through their handle", "[handle]")
{
    Pool pool;
    const auto a = pool.insert(10);
    const auto b = pool.insert(20);

    REQUIRE(a.valid());
    REQUIRE(pool.get(a) != nullptr);
    CHECK(*pool.get(a) == 10);
    CHECK(*pool.get(b) == 20);
    CHECK(pool.size() == 2);
}

TEST_CASE("Stale handles do not resolve after slot reuse", "[handle]")
{
    Pool pool;
    const auto first = pool.insert(1);
    REQUIRE(pool.remove(first) == 1);

    const auto second = pool.insert(2);
    CHECK(second.index == first.index);  // slot recycled
    CHECK(second.generation != first.generation);

    CHECK(pool.get(first) == nullptr);
    CHECK_FALSE(pool.remove(first).has_value());
    CHECK(*pool.get(second) == 2);
}

TEST_CASE("forEach visits only live values", "[handle]")
{
    Pool pool;
    pool.insert(1);
    const auto removed = pool.insert(2);
    pool.insert(3);
    pool.remove(removed);

    int sum = 0;
    pool.forEach([&](int value) { sum += value; });
    CHECK(sum == 4);
}
