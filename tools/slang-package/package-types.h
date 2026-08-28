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
    /// Range constraint for a Git dependency, written without a `v` prefix.
    String version;
    /// An opaque Git branch or tag name that pins this edge instead of selecting a release tag.
    String ref;
    /// Exact semantic version that a pinned Git ref or path dependency provides.
    String as;
};

/// Publisher-authored advice to avoid selecting releases matching `version`.
///
/// The resolver reads these entries from the highest release tag. Existing locks remain valid, but
/// a later `update` skips retracted candidates.
struct Retraction
{
    String version;
    String reason;
};

/// Workspace-authored policy that prevents selecting matching releases of `packageName`.
struct Exclusion
{
    String packageName;
    String version;
    String reason;
};

/// Optional workspace outputs written under `workspace.build/bundle`.
///
/// Consider this example:
///
/// ```json
/// "workspace": { "bundle": { "modules": true, "source": false } }
/// ```
///
/// `slang package build` then writes `.slang-module` files under `build/bundle/modules` and skips
/// the flattened source tree. Both flags default to true when `bundle` is omitted, so a workspace
/// that never mentions the object still produces a complete bundle.
struct BundleSettings
{
    bool modules = true;
    bool source = true;
};

/// Root-only workspace layout from `slang-package.json`. Dependency manifests may contain these
/// fields, but only the manifest that starts resolution controls materialization and output.
struct WorkspaceSettings
{
    String depsDirectory;
    String buildDirectory;
    BundleSettings bundle;
    List<Exclusion> exclusions;
};

/// Root-only native host output from `slang-package.json`. Dependency manifests may declare
/// this field, but only the workspace that starts a build produces executables.
///
/// Each entry in `executables` is both the output filename (without a platform suffix) and the
/// stem of a workspace primary: `video-preview` requires an exported `video-preview.slang`.
/// `defaultExecutable` is the artifact `slang package run` executes when no executable name is
/// given.
struct HostSettings
{
    List<String> executables;
    String defaultExecutable;
};

/// Graph-wide requirement from a package's `tools.slang-toolchain.version`.
///
/// Consider this example: a package uses the `neural` builtin, which only exists after a given
/// compiler release. It cannot depend on that builtin as a source package, so it declares
/// `tools.slang-toolchain.version` as the minimum installed compiler that ships a working copy.
/// Schema 1 only understands that one tool name; the check is against the compiler that provides
/// `slang-package`, regardless of distribution.
struct ToolchainConstraint
{
    String packageName;
    String constraint;
};

struct Manifest
{
    String name;
    List<String> exports;
    List<String> licenseFiles;
    List<Dependency> dependencies;
    List<Retraction> retractions;
    WorkspaceSettings workspace;
    HostSettings host;
    /// Version constraint from `tools.slang-toolchain`, or empty when omitted.
    ///
    /// This is a check against the installed compiler, not a lock row. A package uses it as a
    /// minimum-compiler requirement when it needs a builtin, standard-library API, or compiler
    /// fix that only exists after that version. Other `tools` keys are reserved for later
    /// system-tool or component checks.
    String slangToolchainConstraint;
};

inline bool hasHostExecutables(const Manifest& manifest)
{
    return manifest.host.executables.getCount() != 0;
}

inline bool isHostExecutableName(const Manifest& manifest, const String& name)
{
    for (const auto& executable : manifest.host.executables)
    {
        if (executable == name)
            return true;
    }
    return false;
}

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
/// - Git pin: `git`, `ref`, `version`, and `commit` are set; `path` is empty. Fetch materializes
///   `workspace.deps/<name>` at that commit. An in-place edit keeps this lock shape; the
///   gitignored `slang-workspace.json` changes ownership of the checkout.
/// - Path-only: `path` and `version` are set; `git` is empty. The package is used in place and is
///   trusted only when a manifest `path` edge selected it.
/// - Local override: `git` and `path` are both set. The Git identity is retained, but the tree at
///   `path` is used instead; `version` is the version it provides and `slang-workspace.json` must
///   register the same path.
struct LockedPackage
{
    String name;
    String git;
    String ref;
    String version;
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
    /// Optional exact version for an override. When absent, `update --from-local` uses the version
    /// from the lock row being replaced.
    String as;
    LocalPackageKind kind = LocalPackageKind::Override;
};

inline bool isEditedLocalPackage(const LocalPackage& package)
{
    return package.kind == LocalPackageKind::Edit;
}

enum class VersionComparison
{
    Equal,
    NotEqual,
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

/// One AND-group in a version constraint: every predicate must match.
struct VersionClause
{
    List<VersionPredicate> predicates;
};

/// Version matcher: clauses are OR, predicates inside a clause are AND.
struct VersionConstraint
{
    List<VersionClause> clauses;

    bool matches(const SemanticVersion& version) const;
};

/// Return whether a release version matches a validated policy constraint.
bool matchesVersionPolicy(const String& constraintText, const SemanticVersion& version);

struct TagCandidate
{
    String ref;
    String commit;
    String path;
    bool isEdit = false;
    SemanticVersion version;
};

/// Parse a version constraint written without a `v` prefix.
///
/// Consider this example: `>=1.2.0 !=1.3.0` matches 1.2.0 and 1.4.0 but not 1.3.0.
/// `>=1.0.0 <1.3.0 || >=1.3.1 <2.0.0` matches either interval. Whitespace-separated
/// comparisons in one clause are AND; `||` joins clauses as OR. A clause may be a single
/// exact version such as `1.4.0`.
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

/// Convert a dependency's `version` into a solver constraint.
SlangResult parseDependencyConstraint(
    const Dependency& dependency,
    VersionConstraint& outConstraint,
    String& outError);

SlangResult parseReleaseTag(const UnownedStringSlice& tag, SemanticVersion& outVersion);
inline SlangResult parseReleaseTag(const String& tag, SemanticVersion& outVersion)
{
    return parseReleaseTag(tag.getUnownedSlice(), outVersion);
}

/// Parse an exact semantic version written without a `v` prefix.
SlangResult parseExactVersion(
    const UnownedStringSlice& text,
    SemanticVersion& outVersion,
    String& outError);
inline SlangResult parseExactVersion(
    const String& text,
    SemanticVersion& outVersion,
    String& outError)
{
    return parseExactVersion(text.getUnownedSlice(), outVersion, outError);
}

inline String formatExactVersion(const SemanticVersion& version)
{
    return String(version.m_major) + "." + String(version.m_minor) + "." + String(version.m_patch);
}

inline constexpr char kSlangToolchainName[] = "slang-toolchain";

/// Record a package's `tools.slang-toolchain` constraint when the field is present.
void addSlangToolchainConstraint(
    const Manifest& manifest,
    List<ToolchainConstraint>& ioConstraints);

/// Return the sibling compiler's exact `MAJOR.MINOR.PATCH` identity.
SlangResult getInstalledSlangToolchainVersion(
    SemanticVersion& outVersion,
    String& outExactText,
    String& outError);

/// Check that the installed compiler satisfies every `tools.slang-toolchain` constraint.
///
/// Consider this example: the workspace requires `>=2026.8.0` because it uses `neural`, and a
/// dependency requires `>=2026.14.0` because of a later compiler fix. The installed compiler next
/// to `slang-package` must satisfy both lower bounds. The installed version is not written to the
/// lock.
SlangResult selectSlangToolchain(const List<ToolchainConstraint>& constraints, String& outError);

/// Record a warning when a dependency lists `workspace.excludes` that this workspace did not copy.
///
/// Consider this example: package `display` excludes `color-encoding` `1.0.0` because of a
/// gradient bug, but the workspace that consumes `display` has no matching exclude. Nested
/// `workspace` objects are ignored, so resolution can still select `1.0.0`. This warning tells
/// the workspace author to copy the entry if this project should skip that release too.
void addUnadoptedWorkspaceExclusionWarnings(
    const Manifest& rootManifest,
    const String& packageName,
    const Manifest& packageManifest,
    List<String>* ioWarnings);

/// Append `advice` as a following line of `ioError`.
///
/// Path-bearing errors are written without a trailing period so the path stays copyable, for
/// example `Cannot read JSON file: /tmp/pkg/slang-package.json`. Gluing the next sentence on the
/// same line with a space makes `Run` look like part of the filename. A newline keeps the path
/// intact and still lets `slang-package: error:` introduce the whole report.
inline void appendErrorAdvice(String& ioError, const char* advice)
{
    if (advice)
    {
        while (*advice == ' ')
            ++advice;
    }
    if (!advice || !*advice)
        return;
    if (!ioError.getLength())
    {
        ioError = advice;
        return;
    }
    if (ioError[ioError.getLength() - 1] != '\n')
        ioError.appendChar('\n');
    ioError = ioError + advice;
}

inline void appendErrorAdvice(String& ioError, const String& advice)
{
    appendErrorAdvice(ioError, advice.getBuffer());
}

} // namespace PackageTool
} // namespace Slang

#endif
