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

} // namespace PackageTool
} // namespace Slang

#endif
