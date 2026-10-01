#pragma once

#include "core/slang-list.h"
#include "core/slang-string.h"

namespace Slang
{
namespace PackageTool
{

/// A package release identity made of one or more decimal components.
///
/// Comparison is numeric from left to right. When one sequence is a prefix of the other, the
/// shorter sequence comes first, so `1.2.3 < 1.2.3.0 < 1.2.3.1 < 1.2.4`. The length is part of
/// the identity: `1.2.3` and `1.2.3.0` are different releases. This is a dotted release identifier
/// for the package solver. It is not the compiler's three-component `SemanticVersion`, and it is
/// not stored in a Slang module.
struct PackageVersion
{
    /// At most this many components are accepted. The limit keeps a hostile manifest from asking
    /// the solver to allocate an unbounded sequence.
    static const Index kMaxComponentCount = 32;

    List<uint32_t> components;

    PackageVersion() = default;

    /// Build the three-component release `major.minor.patch`.
    PackageVersion(uint32_t major, uint32_t minor, uint32_t patch)
    {
        components.add(major);
        components.add(minor);
        components.add(patch);
    }

    bool isEmpty() const { return components.getCount() == 0; }

    /// Parse `1`, `1.2`, `1.2.3`, or a longer dotted sequence. A `v` prefix, an empty component,
    /// more than `kMaxComponentCount` components, or a component above 32 bits is rejected.
    static SlangResult parse(const UnownedStringSlice& text, PackageVersion& outVersion);

    /// Write the components in order, without adding or removing trailing zeros.
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
