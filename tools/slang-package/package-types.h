// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_TYPES_H
#define SLANG_PACKAGE_TYPES_H

#include "core/slang-list.h"
#include "core/slang-semantic-version.h"
#include "core/slang-string.h"

namespace Slang
{
namespace PackageTool
{

struct Dependency
{
    String name;
    String git;
    String tag;
};

struct Manifest
{
    String name;
    String version;
    List<String> exports;
    List<Dependency> dependencies;
};

struct LockedPackage
{
    String name;
    String git;
    String tag;
    String commit;
    List<String> exports;
};

struct LockFile
{
    Int lockVersion = 1;
    List<LockedPackage> packages;
};

enum class VersionComparison
{
    Equal,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
};

struct VersionPredicate
{
    VersionComparison comparison;
    SemanticVersion version;
};

struct VersionConstraint
{
    List<VersionPredicate> predicates;

    bool matches(const SemanticVersion& version) const;
};

struct TagCandidate
{
    String tag;
    String commit;
    SemanticVersion version;
};

SlangResult parseVersionConstraint(
    const UnownedStringSlice& text,
    VersionConstraint& outConstraint,
    String& outError);
inline SlangResult parseVersionConstraint(
    const String& text,
    VersionConstraint& outConstraint,
    String& outError)
{
    return parseVersionConstraint(text.getUnownedSlice(), outConstraint, outError);
}

SlangResult parseReleaseTag(const UnownedStringSlice& tag, SemanticVersion& outVersion);
inline SlangResult parseReleaseTag(const String& tag, SemanticVersion& outVersion)
{
    return parseReleaseTag(tag.getUnownedSlice(), outVersion);
}

} // namespace PackageTool
} // namespace Slang

#endif
