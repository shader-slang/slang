#pragma once

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

struct PrimaryModule
{
    String importPath;
    String packageName;
    String sourcePath;
};

/// One `.slang` file from a package export, keyed by its import-relative path in a flattened
/// searchable tree. Companion files keep their path below the primary, for example
/// `acme/noise/helper.slang`.
struct ExportedSourceFile
{
    String relativePath;
    String packageName;
    String sourcePath;
};

/// Return the placeholder text written by `slang package init`.
const char* getLicensePlaceholderText();

/// Validate that one package is suitable for sharing as a dependency.
///
/// In addition to the buildable source shape, this requires non-placeholder licenses and rejects a
/// path dependency that resolves outside the package. It does not inspect transitive package
/// source trees; closure-wide build invariants are checked separately.
SlangResult validatePublishablePackage(
    const String& packageRoot,
    const Manifest& manifest,
    String& outError);

/// Validate the identities and paths in a selected dependency graph using workspace-local data.
///
/// Walk declared edges from the root manifest, using an override's working-tree manifest when that
/// package is pinned locally, and otherwise the manifest of the locked version (Git at `commit`,
/// or the path directory). Every live edge must still select the same lock row; every lock row
/// must be reachable. This is the local "does the current graph still match the lock" check and
/// does not look for newer Git tags.
SlangResult validateWorkspaceResolvedProject(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
    List<String>* outWarnings = nullptr);

/// Validate that each Git pin in `lock` is represented in the package cache.
///
/// This local form does not contact origins.
SlangResult validateCachedResolvedProject(
    const String& projectRoot,
    const LockFile& lock,
    String& outError);

/// Refresh each package cache from origin, then validate every Git pin there.
///
/// This never copies objects from `deps/NAME` into the cache; an adopted commit becomes
/// upstream-valid only after it is pushed to origin and fetched back.
SlangResult refreshAndValidateUpstreamResolvedProject(
    const String& projectRoot,
    const LockFile& lock,
    String& outError);

/// Load the manifest used as the source of declared dependencies and exports for one lock row.
SlangResult loadLockedPackageGraphManifest(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockedPackage& package,
    const List<LocalPackage>& localPackages,
    Manifest& outManifest,
    String& outError);

/// Validate that a proposed materialized graph has the source shape needed by a build.
///
/// `fetch` and `update` use this after materialization so the proposed lock is the source of truth.
/// Every reachable package participates in graph-wide import uniqueness and toolchain selection,
/// including packages whose lock row did not change. `skipSourceValidation` still inventories
/// exports needed by build, but does not check source declarations or import uniqueness.
/// `assumeLegalGraph` skips the identity/toolchain walk when the caller already ran
/// `validateWorkspaceResolvedProject` on this lock.
SlangResult validateBuildableResolvedProject(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
    List<String>* outWarnings = nullptr,
    List<PrimaryModule>* outPrimaryModules = nullptr,
    List<ExportedSourceFile>* outSourceFiles = nullptr,
    bool skipSourceValidation = false,
    bool assumeLegalGraph = false);

/// Validate the workspace package and its materialized, locked dependency closure for a build.
///
/// When requested, return every primary module in import-path order and every exported `.slang`
/// file for bundle source copy. Licenses and publish portability are intentionally outside this
/// predicate.
SlangResult validateBuildableProject(
    const String& projectRoot,
    String& outError,
    List<String>* outWarnings = nullptr,
    List<PrimaryModule>* outPrimaryModules = nullptr,
    List<ExportedSourceFile>* outSourceFiles = nullptr,
    bool skipSourceValidation = false);

} // namespace PackageTool
} // namespace Slang
