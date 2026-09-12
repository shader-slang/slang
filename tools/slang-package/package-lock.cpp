// package-lock.cpp

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
        if (!isPathOnlyLockedPackage(lockedPackage) || lockedPackage.version != dependency.as)
        {
            outError = String("Lock file does not use the declared path version for dependency '") +
                       dependency.name + "'. Run 'slang package update'.";
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
    }
    else if (lockedPackage.git != dependency.git)
    {
        outError = String("Lock file uses a different Git URL for dependency '") + dependency.name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }

    SemanticVersion lockedVersion;
    SLANG_RETURN_ON_FAIL(parseExactVersion(lockedPackage.version, lockedVersion, outError));
    if (dependency.version.getLength())
    {
        VersionConstraint constraint;
        SLANG_RETURN_ON_FAIL(parseDependencyConstraint(dependency, constraint, outError));
        if (!constraint.matches(lockedVersion))
        {
            outError = String("Locked version no longer satisfies dependency '") + dependency.name +
                       "'. Run 'slang package update'.";
            return SLANG_FAIL;
        }
    }
    if (dependency.ref.getLength())
    {
        if (!lockedPackage.path.getLength() && lockedPackage.ref != dependency.ref)
        {
            outError = String("Lock file no longer matches the pinned ref for dependency '") +
                       dependency.name + "'. Run 'slang package update'.";
            return SLANG_FAIL;
        }
        if (dependency.as.getLength() && lockedPackage.version != dependency.as)
        {
            outError = String("Lock file no longer matches the pinned ref for dependency '") +
                       dependency.name + "'. Run 'slang package update'.";
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

SlangResult validateLockedWorkspaceExclusions(
    const Manifest& workspaceManifest,
    const LockFile& lock,
    String& outError)
{
    for (const auto& exclusion : workspaceManifest.workspace.exclusions)
    {
        Index packageIndex = findLockedPackageIndex(lock, exclusion.packageName);
        if (packageIndex < 0)
            continue;
        const LockedPackage& package = lock.packages[packageIndex];
        if (package.path.getLength())
            continue;
        SemanticVersion version;
        String versionError;
        SLANG_RELEASE_ASSERT(
            SLANG_SUCCEEDED(parseExactVersion(package.version, version, versionError)));
        if (matchesVersionPolicy(exclusion.version, version))
        {
            outError = String("Locked package '") + package.name + "' version " + package.version +
                       " is excluded by the workspace: " + exclusion.reason +
                       ". Run 'slang package update'.";
            return SLANG_FAIL;
        }
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
    return SLANG_OK;
}

SlangResult getLockedPackageRoot(
    const String& projectRoot,
    const String& depsDirectory,
    const LockedPackage& package,
    const List<LocalPackage>& localPackages,
    String& outRoot,
    String& outError)
{
    Index localIndex = findActiveLocalPackageIndex(localPackages, package.name);
    if (localIndex >= 0)
        return getLocalPackageRoot(projectRoot, localPackages[localIndex], outRoot, outError);
    if (isLocalOverrideLockedPackage(package))
    {
        outError = String("Locked local override '") + package.name +
                   "' is not registered in slang-package-overlay.json.";
        return SLANG_FAIL;
    }
    if (isPathOnlyLockedPackage(package))
    {
        outRoot = Path::combine(projectRoot, package.path);
        return SLANG_OK;
    }
    outRoot = Path::combine(Path::combine(projectRoot, depsDirectory), package.name);
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

bool lockedPackagesEqual(const LockedPackage& left, const LockedPackage& right)
{
    return left.name == right.name && left.git == right.git && left.ref == right.ref &&
           left.version == right.version && left.commit == right.commit && left.path == right.path;
}

bool lockFilesEqual(const LockFile& left, const LockFile& right)
{
    if (left.packages.getCount() != right.packages.getCount())
        return false;
    for (const auto& package : left.packages)
    {
        Index index = findLockedPackageIndex(right, package.name);
        if (index < 0 || !lockedPackagesEqual(package, right.packages[index]))
            return false;
    }
    return true;
}

} // namespace PackageTool
} // namespace Slang
