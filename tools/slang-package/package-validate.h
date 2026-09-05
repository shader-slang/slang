// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_VALIDATE_H
#define SLANG_PACKAGE_VALIDATE_H

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

/// Validate the identities and paths in a selected dependency graph.
///
/// This checks dependency-to-lock correspondence, trusted reachability, package manifests, local
/// registrations, path identities, and the combined toolchain constraint. Git pins without an
/// active local path are read from `.slang/cache` at the locked commit, so `deps/` need not exist.
/// It deliberately does not inspect licenses, exports, or source declarations.
SlangResult validateLegalResolvedProject(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
    List<String>* outWarnings = nullptr);

/// Validate that a proposed materialized graph has the source shape needed by a build.
///
/// `fetch` and `update` use this after materialization so the proposed lock is the source of truth.
/// Every reachable package participates in graph-wide import uniqueness and toolchain selection,
/// including packages whose lock row did not change. `skipSourceValidation` still inventories
/// exports needed by build, but does not check source declarations or import uniqueness.
/// `assumeLegalGraph` skips the identity/toolchain walk when the caller already ran
/// `validateLegalResolvedProject` on this lock.
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

#endif
