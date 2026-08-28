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

/// Validate one package tree independently of dependency resolution.
///
/// This checks licenses, exported source roots, and every `.slang` file's primary `module` or
/// companion `implementing` declaration. Graph-wide import collisions are checked by
/// `validateResolvedProject`.
SlangResult validatePackageTree(
    const String& packageRoot,
    const Manifest& manifest,
    String& outError,
    bool skipSourceValidation = false);

/// Validate a proposed materialized graph without reading its lock from disk.
///
/// `fetch` and `update` use this after materialization so the lock being fetched or proposed by
/// the solver is the source of truth. Every reachable package is validated, even when its lock row
/// did not change, because module import uniqueness is a graph-wide invariant.
SlangResult validateResolvedProject(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
    List<String>* outWarnings = nullptr,
    List<PrimaryModule>* outPrimaryModules = nullptr,
    List<ExportedSourceFile>* outSourceFiles = nullptr,
    bool skipSourceValidation = false);

/// Validate the workspace package and its materialized, locked dependency closure. When requested,
/// return every primary module in the resolved graph in import-path order, and every exported
/// `.slang` file for bundle source copy. `skipSourceValidation` still walks exports for the build
/// inventory, but does not enforce licenses, first-declaration placement, or import uniqueness.
SlangResult validateProject(
    const String& projectRoot,
    String& outError,
    List<String>* outWarnings = nullptr,
    List<PrimaryModule>* outPrimaryModules = nullptr,
    List<ExportedSourceFile>* outSourceFiles = nullptr,
    bool skipSourceValidation = false);

} // namespace PackageTool
} // namespace Slang

#endif
