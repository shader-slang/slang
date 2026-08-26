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

/// Schema version written in `slang-package.json`, `slang-package-lock.json`, and
/// `slang-workspace.json`. The prototype currently accepts only `1`.
inline constexpr Int kSchemaVersion = 1;

struct Dependency
{
    String name;
    String git;
    String path;
    /// Range constraint for a Git dependency, written without a `v` prefix (for example `>=1.2.0`).
    /// This is not a package self-version; a package's published identity is its Git release tag.
    String version;
    String tag;
};

/// Root-only workspace layout from `slang-package.json`. Dependency manifests may contain these
/// fields, but only the manifest that starts resolution controls materialization and output.
struct WorkspaceSettings
{
    String depsDirectory;
    String buildDirectory;
};

struct Manifest
{
    String name;
    List<String> exports;
    List<String> licenseFiles;
    List<Dependency> dependencies;
    WorkspaceSettings workspace;
};

inline String getWorkspaceDepsDirectory(const Manifest& manifest)
{
    return manifest.workspace.depsDirectory.getLength() ? manifest.workspace.depsDirectory : "deps";
}

inline String getWorkspaceBuildDirectory(const Manifest& manifest)
{
    return manifest.workspace.buildDirectory.getLength() ? manifest.workspace.buildDirectory
                                                         : "build";
}

/// One package in `slang-package-lock.json`. Exactly one of these shapes is legal:
///
/// - Git pin: `git`, `tag`, and `commit` are set; `path` is empty. Fetch materializes
///   `workspace.deps/<name>` at that commit. An in-place edit keeps this lock shape; the
///   gitignored `slang-workspace.json` changes ownership of the checkout.
/// - Path-only: `path` is set; `git` is empty. The package is used in place and is trusted only
///   when a manifest `path` edge selected it.
/// - Local override: `git` and `path` are both set. The Git identity is retained, but the tree at
///   `path` is used instead; `slang-workspace.json` must register the same path.
struct LockedPackage
{
    String name;
    String git;
    String tag;
    String commit;
    String path;
    List<String> exports;
    List<Dependency> dependencies;
};

inline bool isGitBackedLockedPackage(const LockedPackage& package)
{
    return package.git.getLength() != 0;
}

inline bool isPathOnlyLockedPackage(const LockedPackage& package)
{
    return package.path.getLength() != 0 && package.git.getLength() == 0;
}

inline bool isLocalOverrideLockedPackage(const LockedPackage& package)
{
    return package.path.getLength() != 0 && package.git.getLength() != 0;
}

/// A lock row may be used only if a declaring package selected it with a `path` edge, or if the
/// row is Git-backed. A path-only row in the lock is not enough: otherwise a Git dependency could
/// be redirected to an arbitrary directory by editing the lock.
inline bool isTrustedLockSelection(const Dependency& dependency, const LockedPackage& locked)
{
    return dependency.path.getLength() != 0 || isGitBackedLockedPackage(locked);
}

struct LockFile
{
    List<LockedPackage> packages;
};

enum class LocalPackageKind
{
    Override,
    Edit,
};

struct LocalPackage
{
    String name;
    String path;
    LocalPackageKind kind = LocalPackageKind::Override;
};

inline bool isEditedLocalPackage(const LocalPackage& package)
{
    return package.kind == LocalPackageKind::Edit;
}

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
    String path;
    bool isEdit = false;
    SemanticVersion version;
};

/// Parse a version constraint written without a `v` prefix, such as `>=1.2.0 <2.0.0`.
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

/// Convert a dependency's `tag` or `version` into a solver constraint.
///
/// `tag` names one Git release tag and wins over `version` when both are present.
SlangResult parseDependencyConstraint(
    const Dependency& dependency,
    VersionConstraint& outConstraint,
    String& outError);

SlangResult parseReleaseTag(const UnownedStringSlice& tag, SemanticVersion& outVersion);
inline SlangResult parseReleaseTag(const String& tag, SemanticVersion& outVersion)
{
    return parseReleaseTag(tag.getUnownedSlice(), outVersion);
}

} // namespace PackageTool
} // namespace Slang

#endif
