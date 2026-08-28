// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_LOCK_H
#define SLANG_PACKAGE_LOCK_H

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

Index findLockedPackageIndex(const LockFile& lock, const String& name);

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

/// Return a short identity string for a lock row, including Git, path, and version.
String describeLockedPackage(const LockedPackage& package);

/// True when two lock rows record the same identity, exports, and declared dependencies.
bool lockedPackagesEqual(const LockedPackage& left, const LockedPackage& right);

/// Append human-readable lines describing how `next` differs from `previous`.
/// Pass a null `previous` when no lock exists yet.
void describeLockDiff(const LockFile* previous, const LockFile& next, List<String>& outLines);

} // namespace PackageTool
} // namespace Slang

#endif
