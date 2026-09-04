// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_LOCK_H
#define SLANG_PACKAGE_LOCK_H

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

Index findLockedPackageIndex(const LockFile& lock, const String& name);

/// An in-place edit whose package is not in the current lock. The checkout and
/// `slang-workspace.json` registration stay; a later solve that needs the package again reuses
/// them.
inline bool isParkedEdit(const LocalPackage& package, const LockFile& lock)
{
    return isEditedLocalPackage(package) && findLockedPackageIndex(lock, package.name) < 0;
}

/// Verify that a dependency is represented by a compatible package in the lock.
SlangResult validateLockedDependency(
    const Dependency& dependency,
    const LockFile& lock,
    Index& outPackageIndex,
    String& outError);

/// Verify that a package manifest matches the identity and exports recorded in the lock.
SlangResult validateLockedPackageManifest(
    const LockedPackage& package,
    const Manifest& manifest,
    String& outError);

/// Resolve the on-disk root for a lock entry: a registered edit/override, a path-only directory,
/// or the Git checkout under the workspace dependency directory.
SlangResult getLockedPackageRoot(
    const String& projectRoot,
    const String& depsDirectory,
    const LockedPackage& package,
    const List<LocalPackage>& localPackages,
    String& outRoot,
    String& outError);

/// Fail if any lock entry was never selected by a trusted edge.
SlangResult requireAllLockPackagesTrusted(
    const LockFile& lock,
    const List<bool>& trusted,
    String& outError);

/// True when two lock rows record the same identity, exports, and declared dependencies.
bool lockedPackagesEqual(const LockedPackage& left, const LockedPackage& right);

/// True when two locks select the same set of packages with identical rows.
///
/// Package names are unique within a lock, so this compares by name rather than by position: a
/// freshly resolved lock is sorted by name, while a committed lock may have been reordered by
/// hand without changing which releases the workspace selected.
bool lockFilesEqual(const LockFile& left, const LockFile& right);

} // namespace PackageTool
} // namespace Slang

#endif
