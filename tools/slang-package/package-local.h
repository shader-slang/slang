// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_LOCAL_H
#define SLANG_PACKAGE_LOCAL_H

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

Index findLocalPackageIndex(const List<LocalPackage>& packages, const String& name);
Index findActiveLocalPackageIndex(const List<LocalPackage>& packages, const String& name);

/// Return whether an override uses the checkout path owned by this workspace.
bool isInPlaceLocalPackage(const Manifest& manifest, const LocalPackage& package);

/// Read `slang-pkg-workspace.json`, treating an absent file as no overrides.
SlangResult readProjectLocalPackages(
    const String& projectRoot,
    List<LocalPackage>& outPackages,
    String& outError);

/// Write local override state to `slang-pkg-workspace.json`.
SlangResult writeProjectLocalPackages(
    const String& projectRoot,
    const List<LocalPackage>& packages,
    String& outError);

/// Resolve a registered path and verify that it names a package directory.
SlangResult getLocalPackageRoot(
    const String& projectRoot,
    const LocalPackage& package,
    String& outRoot,
    String& outError);

/// Read the manifest from a registered local package and verify its package name.
SlangResult readLocalPackageManifest(
    const String& projectRoot,
    const LocalPackage& package,
    Manifest& outManifest,
    String& outError);

} // namespace PackageTool
} // namespace Slang

#endif
