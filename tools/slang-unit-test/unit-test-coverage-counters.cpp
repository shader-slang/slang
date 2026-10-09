#include "shader-coverage-common/coverage-counters.h"
#include "unit-test/slang-unit-test.h"

using coverageDemo::decodeCoverageCounters;

SLANG_UNIT_TEST(coverageCountersDecode32)
{
    const uint8_t bytes[][4] = {
        {0, 0, 0, 0},             // Zero hits.
        {1, 0, 0, 0},             // One hit (also the covered value in boolean mode).
        {0x78, 0x56, 0x34, 0x12}, // Distinct bytes check little-endian order.
        {0xff, 0xff, 0xff, 0xff}, // UINT32_MAX must be zero-extended, not sign-extended.
    };
    const std::vector<uint64_t> expected = {0, 1, 0x12345678, UINT32_MAX};
    auto hits = decodeCoverageCounters(bytes, sizeof(bytes), 4);
    SLANG_CHECK(hits == expected);
}

SLANG_UNIT_TEST(coverageCountersDecode64)
{
    const uint8_t bytes[][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},                         // Zero hits.
        {1, 0, 0, 0, 0, 0, 0, 0},                         // One hit.
        {0, 0, 0, 0, 1, 0, 0, 0},                         // First value above UINT32_MAX.
        {0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01}, // Distinct bytes in both halves.
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // UINT64_MAX.
    };
    const std::vector<uint64_t> expected =
        {0, 1, UINT64_C(0x100000000), UINT64_C(0x0123456789abcdef), UINT64_MAX};
    auto hits = decodeCoverageCounters(bytes, sizeof(bytes), 8);
    SLANG_CHECK(hits == expected);
}

SLANG_UNIT_TEST(coverageCountersDecodeEmpty)
{
    SLANG_CHECK(decodeCoverageCounters(nullptr, 0, 4).empty());
    SLANG_CHECK(decodeCoverageCounters(nullptr, 0, 8).empty());
}

SLANG_UNIT_TEST(coverageCountersDecodeUnaligned)
{
    // The sentinel leaves the counter at an unaligned address. Decoding must not
    // alter the raw snapshot that the examples later write to counters.bin.
    alignas(uint64_t) uint8_t bytes[] = {0xaa, 0x78, 0x56, 0x34, 0x12, 1, 0, 0, 0};
    const std::vector<uint8_t> original(bytes, bytes + sizeof(bytes));
    const std::vector<uint64_t> expected32 = {0x12345678, 1};
    const std::vector<uint64_t> expected64 = {UINT64_C(0x112345678)};
    SLANG_CHECK(decodeCoverageCounters(bytes + 1, 8, 4) == expected32);
    SLANG_CHECK(decodeCoverageCounters(bytes + 1, 8, 8) == expected64);
    SLANG_CHECK(std::vector<uint8_t>(bytes, bytes + sizeof(bytes)) == original);
}
