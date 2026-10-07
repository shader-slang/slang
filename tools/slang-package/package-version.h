#pragma once

#include "core/slang-list.h"
#include "core/slang-string.h"

namespace Slang
{
namespace PackageTool
{

/// A package release identity made of one or more decimal components.
///
/// Trailing zero components are not part of the identity, following the same rule as Python's
/// release versions: `1.2.3`, `1.2.3.0`, and `1.2.3.0.0` are one release, and the stored spelling
/// is `1.2.3`. A zero that is not trailing stays, so `1.2.0.1` is a different release from `1.2`.
/// Comparison is numeric from left to right, and a stored sequence that is a proper prefix of
/// another comes first, so `1.2.3 < 1.2.3.1 < 1.2.4`. This is a dotted release identifier for the
/// package solver. It is not the compiler's three-component `SemanticVersion`, and it is not
/// stored in a Slang module.
struct PackageVersion
{
    /// At most this many components are accepted, counted before trailing zeros are removed. The
    /// limit keeps a hostile manifest from asking the solver to allocate an unbounded sequence.
    static const Index kMaxComponentCount = 32;

    /// Components with no trailing zeros. At least one component is present after a successful
    /// parse, so `0.0.0` is stored as `0`.
    List<uint32_t> components;

    PackageVersion() = default;

    /// Build the release `major.minor.patch`, dropping trailing zero components.
    PackageVersion(uint32_t major, uint32_t minor, uint32_t patch);

    bool isEmpty() const { return components.getCount() == 0; }

    /// Parse `1`, `1.2`, `1.2.3`, or a longer dotted sequence and drop trailing zero components.
    /// A `v` prefix, an empty component, more than `kMaxComponentCount` components, or a component
    /// above 32 bits is rejected. `1.2.0` and `1.2` parse as the same release.
    static SlangResult parse(const UnownedStringSlice& text, PackageVersion& outVersion);

    /// Parse `text` without dropping trailing zero components.
    ///
    /// `^` and `~` use the components as written to choose an upper bound, so `^0.0.0` stays
    /// narrower than `^0`. The bounds are then stored with trailing zeros removed.
    static SlangResult parsePreservingTrailingZeros(
        const UnownedStringSlice& text,
        PackageVersion& outVersion);

    /// Write the stored components. The result has no leading zeros and no trailing zero
    /// components, so it is the canonical spelling of the release.
    String format() const;

    int compare(const PackageVersion& other) const;

    bool operator==(const PackageVersion& other) const { return compare(other) == 0; }
    bool operator!=(const PackageVersion& other) const { return compare(other) != 0; }
    bool operator<(const PackageVersion& other) const { return compare(other) < 0; }
    bool operator>(const PackageVersion& other) const { return compare(other) > 0; }
    bool operator<=(const PackageVersion& other) const { return compare(other) <= 0; }
    bool operator>=(const PackageVersion& other) const { return compare(other) >= 0; }
};

} // namespace PackageTool
} // namespace Slang
