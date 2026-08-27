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
        if (lockedPackage.version != dependency.as ||
            (!lockedPackage.path.getLength() && lockedPackage.ref != dependency.ref))
        {
            outError = String("Lock file no longer matches the pinned ref for dependency '") +
                       dependency.name + "'. Run 'slang package update'.";
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
                dependency.ref == lockedDependency.ref && dependency.as == lockedDependency.as)
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
    const String& depsDirectory,
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
                   "' is not registered in slang-workspace.json.";
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

static bool _dependenciesEqual(const List<Dependency>& left, const List<Dependency>& right)
{
    if (left.getCount() != right.getCount())
        return false;
    for (const auto& dependency : left)
    {
        bool found = false;
        for (const auto& other : right)
        {
            if (dependency.name == other.name && dependency.git == other.git &&
                dependency.path == other.path && dependency.version == other.version &&
                dependency.ref == other.ref && dependency.as == other.as)
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    return true;
}

static bool _stringSetsEqual(const List<String>& left, const List<String>& right)
{
    if (left.getCount() != right.getCount())
        return false;
    for (const auto& value : left)
    {
        if (!right.contains(value))
            return false;
    }
    return true;
}

static bool _lockedPackagesEqual(const LockedPackage& left, const LockedPackage& right)
{
    return left.name == right.name && left.git == right.git && left.ref == right.ref &&
           left.version == right.version && left.commit == right.commit &&
           left.path == right.path && _stringSetsEqual(left.exports, right.exports) &&
           _dependenciesEqual(left.dependencies, right.dependencies);
}

static String _describeLockedPackage(const LockedPackage& package)
{
    if (isLocalOverrideLockedPackage(package))
        return package.git + " via " + package.path + " as " + package.version;
    if (isPathOnlyLockedPackage(package))
        return String("path ") + package.path + " as " + package.version;
    String description = package.git;
    if (package.ref.getLength())
        description = description + " " + package.ref + " as " + package.version;
    if (package.commit.getLength())
        description = description + " (" + package.commit + ")";
    return description;
}

static void _addSortedUniqueName(List<String>& names, const String& name)
{
    for (Index i = 0; i < names.getCount(); ++i)
    {
        if (names[i] == name)
            return;
        if (names[i] > name)
        {
            names.insert(i, name);
            return;
        }
    }
    names.add(name);
}

void describeLockDiff(const LockFile* previous, const LockFile& next, List<String>& outLines)
{
    outLines.clear();
    List<String> names;
    if (previous)
    {
        for (const auto& package : previous->packages)
            _addSortedUniqueName(names, package.name);
    }
    for (const auto& package : next.packages)
        _addSortedUniqueName(names, package.name);

    for (const auto& name : names)
    {
        Index previousIndex = previous ? findLockedPackageIndex(*previous, name) : -1;
        Index nextIndex = findLockedPackageIndex(next, name);
        if (previousIndex < 0)
        {
            outLines.add(
                String("Would add '") + name + "' as " +
                _describeLockedPackage(next.packages[nextIndex]));
            continue;
        }
        if (nextIndex < 0)
        {
            outLines.add(
                String("Would remove '") + name + "' (" +
                _describeLockedPackage(previous->packages[previousIndex]) + ")");
            continue;
        }
        const LockedPackage& before = previous->packages[previousIndex];
        const LockedPackage& after = next.packages[nextIndex];
        if (_lockedPackagesEqual(before, after))
            continue;
        outLines.add(
            String("Would change '") + name + "' from " + _describeLockedPackage(before) + " to " +
            _describeLockedPackage(after));
    }
}

} // namespace PackageTool
} // namespace Slang
