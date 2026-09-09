#include "../../lib/parser/parser.h"
#include <catch2/catch_test_macros.hpp>

using namespace spvdb;

// Minimal valid SPIR-V module with only a magic number + header.
static const uint32_t kMinimalSpv[] = {
    0x07230203, // magic
    0x00010600, // version 1.6
    0x00000000, // generator
    0x00000001, // bound = 1
    0x00000000, // schema
};

TEST_CASE("parse_spirv: minimal module")
{
    auto r = parse_spirv(std::span(kMinimalSpv));
    REQUIRE(r.ok());
    CHECK(r->version == 0x00010600);
    CHECK(r->id_bound == 1);
    CHECK(r->instructions.empty());
}

TEST_CASE("parse_spirv: bad magic")
{
    uint32_t bad[] = {0xDEADBEEF, 0, 0, 1, 0};
    auto r = parse_spirv(std::span(bad));
    REQUIRE(!r.ok());
}

TEST_CASE("parse_spirv: byte-swapped magic")
{
    uint32_t swapped[] = {0x03022307, 0x03060100, 0, 1, 0};
    auto r = parse_spirv(std::span(swapped));
    REQUIRE(r.ok());
}

TEST_CASE("parse_spirv: too short")
{
    uint32_t tiny[] = {0x07230203, 0};
    auto r = parse_spirv(std::span(tiny));
    REQUIRE(!r.ok());
}
