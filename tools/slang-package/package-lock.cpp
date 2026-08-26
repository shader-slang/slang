// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-lock.h"

#include "core/slang-io.h"
#include "package-local.h"

namespace Slang
{
namespace PackageTool
{

Index findLockedPackageIndex(const LockFile& lock, const String& name)
{
    for (Index i = 0; i < lock.packages.getCount(); ++i)
    {
        if (lock.packages[i].name == name)
            return i;
    }
    return -1;
}

SlangResult validateLockedDependency(
    const Dependency& dependency,
    const LockFile& lock,
    Index& outPackageIndex,
    String& outError)
{
    outPackageIndex = findLockedPackageIndex(lock, dependency.name);
    if (outPackageIndex < 0)
    {
        outError = String("Lock file does not contain dependency '") + dependency.name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }

    const LockedPackage& lockedPackage = lock.packages[outPackageIndex];
    if (dependency.path.getLength())
    {
        if (!isPathOnlyLockedPackage(lockedPackage))
        {
            outError = String("Lock file does not use a path for dependency '") + dependency.name +
                       "'. Run 'slang package update'.";
            return SLANG_FAIL;
        }
        return SLANG_OK;
    }
    if (lockedPackage.path.getLength())
    {
        if (isLocalOverrideLockedPackage(lockedPackage) && lockedPackage.git != dependency.git)
        {
            outError = String("Lock file path for Git dependency '") + dependency.name +
                       "' uses a different Git location. Run 'slang package update'.";
            return SLANG_FAIL;
        }
        // A manifest path or project-local override shadows the Git requirement. Its transitive
        // requirements remain locked, but it has no release version to compare with this range.
        return SLANG_OK;
    }
    if (lockedPackage.git != dependency.git)
    {
        outError = String("Lock file uses a different Git URL for dependency '") + dependency.name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }

    VersionConstraint constraint;
    SemanticVersion lockedVersion;
    SlangResult constraintResult = parseDependencyConstraint(dependency, constraint, outError);
    if (SLANG_FAILED(constraintResult))
        return constraintResult;
    SlangResult versionResult = parseReleaseTag(lockedPackage.tag, lockedVersion);
    if (SLANG_FAILED(versionResult) || !constraint.matches(lockedVersion))
    {
        outError = String("Locked version no longer satisfies dependency '") + dependency.name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult validateLockedPackageManifest(
    const LockedPackage& package,
    const Manifest& manifest,
    String& outError)
{
    if (manifest.name != package.name)
    {
        outError = String("Locked package manifest has a different name: ") + package.name;
        return SLANG_FAIL;
    }

    // Exports are an unordered set of source roots, so reordering the manifest array must not
    // invalidate a lock that already records the same roots. Manifest and lock reading both reject
    // duplicate entries, so equal counts plus containment mean the two sets are equal.
    if (manifest.exports.getCount() != package.exports.getCount())
    {
        outError = String("Locked package manifest exports do not match its lock: ") + package.name;
        return SLANG_FAIL;
    }
    for (const auto& exportPath : manifest.exports)
    {
        if (!package.exports.contains(exportPath))
        {
            outError =
                String("Locked package manifest exports do not match its lock: ") + package.name;
            return SLANG_FAIL;
        }
    }

    if (manifest.dependencies.getCount() != package.dependencies.getCount())
    {
        outError = String("Package manifest dependencies do not match its lock: ") + package.name;
        return SLANG_FAIL;
    }
    for (const auto& dependency : manifest.dependencies)
    {
        bool found = false;
        for (const auto& lockedDependency : package.dependencies)
        {
            if (dependency.name == lockedDependency.name &&
                dependency.git == lockedDependency.git &&
                dependency.path == lockedDependency.path &&
                dependency.version == lockedDependency.version &&
                dependency.tag == lockedDependency.tag)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            outError =
                String("Package manifest dependencies do not match its lock: ") + package.name;
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

SlangResult getLockedPackageRoot(
    const String& projectRoot,
    const LockedPackage& package,
    const List<LocalPackage>& localPackages,
    String& outRoot,
    String& outError)
{
    Index localIndex = findLocalPackageIndex(localPackages, package.name);
    if (localIndex >= 0)
        return getLocalPackageRoot(projectRoot, localPackages[localIndex], outRoot, outError);
    if (isLocalOverrideLockedPackage(package))
    {
        outError = String("Locked local override '") + package.name +
                   "' is not registered in .slang/overrides.json.";
        return SLANG_FAIL;
    }
    if (isPathOnlyLockedPackage(package))
    {
        outRoot = Path::combine(projectRoot, package.path);
        return SLANG_OK;
    }
    outRoot = Path::combine(Path::combine(projectRoot, ".slang", "packages"), package.name);
    return SLANG_OK;
}

SlangResult requireAllLockPackagesTrusted(
    const LockFile& lock,
    const List<bool>& trusted,
    String& outError)
{
    SLANG_RELEASE_ASSERT(trusted.getCount() == lock.packages.getCount());
    for (Index i = 0; i < lock.packages.getCount(); ++i)
    {
        if (trusted[i])
            continue;
        outError =
            isPathOnlyLockedPackage(lock.packages[i])
                ? String("Locked path package '") + lock.packages[i].name +
                      "' is not selected by a trusted path dependency."
                : String("Lock file contains unreachable package '") + lock.packages[i].name + "'.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
