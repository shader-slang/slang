// unit-test-semantic-version.cpp
//
// Contract under test
// -------------------
// `SemanticVersion` is a (major, minor, patch) triple used for
// downstream-tool version compatibility checks (DXC, FXC, Metal
// SDK, etc.). The contract this suite pins:
//
//   * Parse accepts `M`, `M.N`, `M.N.P` decimal forms with `.` (or
//     a custom char) as separator. Empty / non-numeric / >3-segment
//     inputs fail.
//   * `append` produces a deterministic textual form: patch is
//     elided when zero (`1.2.0` formats as `1.2`); non-zero patch
//     is always kept.
//   * Ordering is lexicographic on (major, minor, patch) — same
//     order as comparing the `RawValue` packed integer.
//   * `fromRaw(getRawValue(v)) == v` — the raw form is a bijection.
//   * `isBackwardsCompatibleWith(other)` returns true iff the major
//     versions match AND `this.minor >= other.minor`. Patch is
//     intentionally ignored — the check is "could code targeting
//     `other` work against `this`?", which doesn't depend on patch.
//   * `getEarliest` / `getLatest` over an array — min and max under
//     the same lexicographic ordering. Empty input returns the
//     default (zero) version.

#include "../../source/core/slang-semantic-version.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

String formatVersion(const SemanticVersion& v)
{
    StringBuilder buf;
    v.append(buf);
    return buf.produceString();
}

SemanticVersion parseOrFail(const char* text)
{
    SemanticVersion v;
    SlangResult r = SemanticVersion::parse(UnownedStringSlice(text), v);
    SLANG_ASSERT(SLANG_SUCCEEDED(r));
    return v;
}

} // namespace

SLANG_UNIT_TEST(semanticVersionParse)
{
    SemanticVersion v;

    // Major-only.
    SLANG_CHECK(SLANG_SUCCEEDED(SemanticVersion::parse(UnownedStringSlice("3"), v)));
    SLANG_CHECK(v.m_major == 3 && v.m_minor == 0 && v.m_patch == 0);

    // Major.minor.
    SLANG_CHECK(SLANG_SUCCEEDED(SemanticVersion::parse(UnownedStringSlice("12.7"), v)));
    SLANG_CHECK(v.m_major == 12 && v.m_minor == 7 && v.m_patch == 0);

    // Major.minor.patch.
    SLANG_CHECK(SLANG_SUCCEEDED(SemanticVersion::parse(UnownedStringSlice("1.2.3"), v)));
    SLANG_CHECK(v.m_major == 1 && v.m_minor == 2 && v.m_patch == 3);

    // Large patch — patch is uint32_t so values well above 65535 are valid.
    SLANG_CHECK(SLANG_SUCCEEDED(SemanticVersion::parse(UnownedStringSlice("0.0.123456789"), v)));
    SLANG_CHECK(v.m_major == 0 && v.m_minor == 0 && v.m_patch == 123456789u);
}

SLANG_UNIT_TEST(semanticVersionParseSeparator)
{
    SemanticVersion v;
    // Custom separator (used for some downstream-compiler version strings).
    SLANG_CHECK(SLANG_SUCCEEDED(SemanticVersion::parse(UnownedStringSlice("4-5-6"), '-', v)));
    SLANG_CHECK(v.m_major == 4 && v.m_minor == 5 && v.m_patch == 6);

    // The default-separator overload uses '.'; '-' should be rejected.
    SLANG_CHECK(SLANG_FAILED(SemanticVersion::parse(UnownedStringSlice("1-2-3"), v)));
}

SLANG_UNIT_TEST(semanticVersionParseInvalid)
{
    SemanticVersion v;
    // Empty.
    SLANG_CHECK(SLANG_FAILED(SemanticVersion::parse(UnownedStringSlice(""), v)));
    // Non-numeric.
    SLANG_CHECK(SLANG_FAILED(SemanticVersion::parse(UnownedStringSlice("abc"), v)));
    // Mixed digits + letters.
    SLANG_CHECK(SLANG_FAILED(SemanticVersion::parse(UnownedStringSlice("1.x.3"), v)));
    // Too many segments (more than major.minor.patch).
    SLANG_CHECK(SLANG_FAILED(SemanticVersion::parse(UnownedStringSlice("1.2.3.4"), v)));
}

SLANG_UNIT_TEST(semanticVersionRoundTrip)
{
    // SemanticVersion::append elides ".0" patch, so "1.2.0" → "1.2".
    // Verify both directions match the documented format.
    struct Case
    {
        const char* input;
        const char* expectedAppend;
    };
    Case samples[] = {
        {"1.0.0", "1.0"},
        {"0.1.0", "0.1"},
        {"0.0.1", "0.0.1"},
        {"1.2.3", "1.2.3"},
        {"10.20.30", "10.20.30"},
        {"32767.32767.65535", "32767.32767.65535"},
    };
    for (const auto& c : samples)
    {
        SemanticVersion v = parseOrFail(c.input);
        String formatted = formatVersion(v);
        SLANG_CHECK(formatted == c.expectedAppend);
    }
}

SLANG_UNIT_TEST(semanticVersionOrdering)
{
    SemanticVersion v1_0_0(1, 0, 0);
    SemanticVersion v1_0_1(1, 0, 1);
    SemanticVersion v1_1_0(1, 1, 0);
    SemanticVersion v2_0_0(2, 0, 0);

    SLANG_CHECK(v1_0_0 < v1_0_1);
    SLANG_CHECK(v1_0_1 < v1_1_0);
    SLANG_CHECK(v1_1_0 < v2_0_0);
    SLANG_CHECK(v2_0_0 > v1_0_0);
    SLANG_CHECK(v1_0_0 == SemanticVersion(1, 0, 0));
    SLANG_CHECK(v1_0_0 != v1_0_1);
    SLANG_CHECK(v1_0_0 <= v1_0_0);
    SLANG_CHECK(v1_0_0 >= v1_0_0);
}

SLANG_UNIT_TEST(semanticVersionRawValueRoundTrip)
{
    SemanticVersion samples[] = {
        SemanticVersion(0, 0, 0),
        SemanticVersion(1, 2, 3),
        // patch above 0x7FFFFFFF round-trips through int in setRawValue
        // and asserts; stay under that.
        SemanticVersion(32767, 32767, 0x7FFFFFFF),
    };
    for (const auto& v : samples)
    {
        SemanticVersion::RawValue raw = v.getRawValue();
        SemanticVersion roundTrip = SemanticVersion::fromRaw(raw);
        SLANG_CHECK(roundTrip == v);
    }
}

SLANG_UNIT_TEST(semanticVersionIsSet)
{
    SLANG_CHECK(!SemanticVersion().isSet());
    SLANG_CHECK(SemanticVersion(1, 0, 0).isSet());
    SLANG_CHECK(SemanticVersion(0, 1, 0).isSet());
    SLANG_CHECK(SemanticVersion(0, 0, 1).isSet());

    SemanticVersion v(1, 2, 3);
    v.reset();
    SLANG_CHECK(!v.isSet());
}

SLANG_UNIT_TEST(semanticVersionBackwardsCompat)
{
    // Implementation: same major required; this.minor must be >= other.minor;
    // patch is intentionally ignored.
    SemanticVersion v1_2_5(1, 2, 5);

    SLANG_CHECK(v1_2_5.isBackwardsCompatibleWith(SemanticVersion(1, 2, 5)));
    SLANG_CHECK(v1_2_5.isBackwardsCompatibleWith(SemanticVersion(1, 2, 4)));
    SLANG_CHECK(v1_2_5.isBackwardsCompatibleWith(SemanticVersion(1, 1, 99)));
    SLANG_CHECK(v1_2_5.isBackwardsCompatibleWith(SemanticVersion(1, 2, 6))); // patch ignored
    SLANG_CHECK(!v1_2_5.isBackwardsCompatibleWith(SemanticVersion(1, 3, 0))); // newer minor
    SLANG_CHECK(!v1_2_5.isBackwardsCompatibleWith(SemanticVersion(2, 0, 0))); // diff major
    SLANG_CHECK(!v1_2_5.isBackwardsCompatibleWith(SemanticVersion(0, 9, 9))); // older major
}

SLANG_UNIT_TEST(semanticVersionEarliestLatest)
{
    SemanticVersion versions[] = {
        SemanticVersion(2, 0, 0),
        SemanticVersion(1, 5, 3),
        SemanticVersion(2, 1, 0),
        SemanticVersion(0, 9, 9),
        SemanticVersion(2, 0, 5),
    };
    Count count = SLANG_COUNT_OF(versions);

    SLANG_CHECK(SemanticVersion::getEarliest(versions, count) == SemanticVersion(0, 9, 9));
    SLANG_CHECK(SemanticVersion::getLatest(versions, count) == SemanticVersion(2, 1, 0));

    // Empty input → default-constructed version.
    SLANG_CHECK(SemanticVersion::getEarliest(versions, 0) == SemanticVersion());
    SLANG_CHECK(SemanticVersion::getLatest(versions, 0) == SemanticVersion());
}

SLANG_UNIT_TEST(semanticVersionHashCode)
{
    // Same version → same hash; different version → different hash.
    SemanticVersion a(1, 2, 3);
    SemanticVersion b(1, 2, 3);
    SemanticVersion c(1, 2, 4);

    SLANG_CHECK(a.getHashCode() == b.getHashCode());
    SLANG_CHECK(a.getHashCode() != c.getHashCode());
}
