// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-tool.h"

#include "core/slang-io.h"
#include "package-git.h"
#include "package-json.h"
#include "package-resolver.h"

#include <stdio.h>

namespace Slang
{
namespace PackageTool
{

static const char* const kManifestName = "slang-package.json";
static const char* const kLockName = "slang-package.lock";

static void _printHelp()
{
    fprintf(
        stdout,
        "Usage: slang-package <command>\n"
        "\n"
        "Commands:\n"
        "  init             Create a package manifest and standard directories.\n"
        "  fetch [--locked] Materialize dependencies from the lock file.\n"
        "  update           Re-resolve dependencies and update the lock file.\n"
        "  edit <name>      Create an editable dependency checkout.\n"
        "  unedit <name>    Remove an editable checkout.\n"
        "  help             Show this help text.\n");
}

static SlangResult _getProjectRoot(String& outRoot, String& outError)
{
    if (SLANG_FAILED(Path::getCanonical(".", outRoot)))
    {
        outError = "Cannot determine the current directory.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static LockedPackage* _findLockedPackage(LockFile& lock, const String& name)
{
    for (auto& package : lock.packages)
    {
        if (package.name == name)
            return &package;
    }
    return nullptr;
}

static Index _findLockedPackageIndex(const LockFile& lock, const String& name)
{
    for (Index i = 0; i < lock.packages.getCount(); ++i)
    {
        if (lock.packages[i].name == name)
            return i;
    }
    return -1;
}

static SlangResult _validateLockedDependency(
    const Dependency& dependency,
    const LockFile& lock,
    Index& outPackageIndex,
    String& outError)
{
    outPackageIndex = _findLockedPackageIndex(lock, dependency.name);
    if (outPackageIndex < 0)
    {
        outError = String("Lock file does not contain dependency '") + dependency.name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }

    const LockedPackage& lockedPackage = lock.packages[outPackageIndex];
    if (lockedPackage.git != dependency.git)
    {
        outError = String("Lock file uses a different Git URL for dependency '") + dependency.name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }

    VersionConstraint constraint;
    SemanticVersion lockedVersion;
    SLANG_RETURN_ON_FAIL(parseVersionConstraint(dependency.tag, constraint, outError));
    if (SLANG_FAILED(parseReleaseTag(lockedPackage.tag, lockedVersion)) ||
        !constraint.matches(lockedVersion))
    {
        outError = String("Locked tag no longer satisfies dependency '") + dependency.name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _writeSearchPaths(
    const String& projectRoot,
    const LockFile& lock,
    String& outError)
{
    StringBuilder searchPaths;
    for (const auto& package : lock.packages)
    {
        String editablePath = Path::combine(".slang", "edit", package.name);
        String editableDirectory = Path::combine(projectRoot, editablePath);
        SlangPathType editablePathType;
        if (SLANG_SUCCEEDED(Path::getPathType(editableDirectory, &editablePathType)) &&
            editablePathType == SLANG_PATH_TYPE_DIRECTORY)
        {
            for (const auto& exportPath : package.exports)
                searchPaths << Path::combine(editablePath, exportPath) << "\n";
            continue;
        }

        String packageRoot = Path::combine(".slang", "packages", package.name);
        for (const auto& exportPath : package.exports)
            searchPaths << Path::combine(packageRoot, exportPath) << "\n";
    }

    String slangDirectory = Path::combine(projectRoot, ".slang");
    if (!Path::createDirectoryRecursive(slangDirectory) ||
        SLANG_FAILED(
            File::writeAllText(Path::combine(slangDirectory, "search-paths"), searchPaths)))
    {
        outError = "Cannot write .slang/search-paths.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _materialize(const String& projectRoot, const LockFile& lock, String& outError)
{
    String packagesRoot = Path::combine(projectRoot, ".slang", "packages");
    if (!Path::createDirectoryRecursive(packagesRoot))
    {
        outError = String("Cannot create package directory: ") + packagesRoot;
        return SLANG_FAIL;
    }

    for (const auto& package : lock.packages)
    {
        List<TagCandidate> candidates;
        SLANG_RETURN_ON_FAIL(listReleaseTags(package.git, candidates, outError));
        const TagCandidate* matchingTag = nullptr;
        for (const auto& candidate : candidates)
        {
            if (candidate.tag == package.tag)
            {
                matchingTag = &candidate;
                break;
            }
        }
        if (!matchingTag || matchingTag->commit != package.commit)
        {
            outError = String("Locked tag no longer identifies the locked commit for package '") +
                       package.name + "'.";
            return SLANG_FAIL;
        }

        String destination = Path::combine(packagesRoot, package.name);
        SLANG_RETURN_ON_FAIL(
            materializeRevision(projectRoot, package.git, package.commit, destination, outError));
    }
    return _writeSearchPaths(projectRoot, lock, outError);
}

static SlangResult _readProjectManifest(
    const String& projectRoot,
    Manifest& outManifest,
    String& outError)
{
    return readManifest(Path::combine(projectRoot, kManifestName), outManifest, outError);
}

static SlangResult _readProjectLock(const String& projectRoot, LockFile& outLock, String& outError)
{
    return readLockFile(Path::combine(projectRoot, kLockName), outLock, outError);
}

/// Verify that the root manifest's direct requirements are represented by the lock.
///
/// Transitive requirements are fixed by each locked package commit, so they cannot drift without
/// changing a commit recorded in the lock.
static SlangResult _validateLockAgainstManifest(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    String& outError)
{
    List<bool> reachablePackages;
    reachablePackages.setCount(lock.packages.getCount());
    for (auto& reachable : reachablePackages)
        reachable = false;
    List<Index> pendingPackages;
    for (const auto& dependency : manifest.dependencies)
    {
        Index packageIndex;
        SLANG_RETURN_ON_FAIL(_validateLockedDependency(dependency, lock, packageIndex, outError));
        if (!reachablePackages[packageIndex])
        {
            reachablePackages[packageIndex] = true;
            pendingPackages.add(packageIndex);
        }
    }

    for (Index pendingIndex = 0; pendingIndex < pendingPackages.getCount(); ++pendingIndex)
    {
        const LockedPackage& package = lock.packages[pendingPackages[pendingIndex]];
        String repositoryPath =
            Path::combine(projectRoot, Path::combine(".slang", "cache", package.name));
        SLANG_RETURN_ON_FAIL(ensureRepository(projectRoot, package.git, repositoryPath, outError));
        String manifestText;
        SLANG_RETURN_ON_FAIL(readFileAtRevision(
            repositoryPath,
            package.commit,
            kManifestName,
            manifestText,
            outError));
        Manifest packageManifest;
        SLANG_RETURN_ON_FAIL(readManifestText(
            package.git + "@" + package.commit + ":" + kManifestName,
            manifestText,
            packageManifest,
            outError));
        if (packageManifest.name != package.name)
        {
            outError = String("Locked package manifest has a different name: ") + package.name;
            return SLANG_FAIL;
        }
        SemanticVersion tagVersion;
        SemanticVersion manifestVersion;
        if (SLANG_FAILED(parseReleaseTag(package.tag, tagVersion)) ||
            SLANG_FAILED(SemanticVersion::parse(
                packageManifest.version.getUnownedSlice(),
                manifestVersion)) ||
            tagVersion != manifestVersion)
        {
            outError =
                String("Locked package manifest version does not match its tag: ") + package.name;
            return SLANG_FAIL;
        }
        for (const auto& dependency : packageManifest.dependencies)
        {
            Index dependencyIndex;
            SLANG_RETURN_ON_FAIL(
                _validateLockedDependency(dependency, lock, dependencyIndex, outError));
            if (!reachablePackages[dependencyIndex])
            {
                reachablePackages[dependencyIndex] = true;
                pendingPackages.add(dependencyIndex);
            }
        }
    }

    for (Index i = 0; i < reachablePackages.getCount(); ++i)
    {
        if (!reachablePackages[i])
        {
            outError = String("Lock file contains unreachable package '") + lock.packages[i].name +
                       "'. Run 'slang package update'.";
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

static SlangResult _init(const String& projectRoot, String& outError)
{
    String manifestPath = Path::combine(projectRoot, kManifestName);
    if (File::exists(manifestPath))
    {
        outError = "slang-package.json already exists.";
        return SLANG_FAIL;
    }

    Manifest manifest;
    manifest.name = Path::getFileName(projectRoot);
    if (!isValidPackageName(manifest.name))
    {
        outError = String("Directory name is not a valid package name: ") + manifest.name;
        return SLANG_FAIL;
    }

    static const char* const kDirectories[] = {"src", "tests", "docs"};
    for (auto directory : kDirectories)
    {
        String path = Path::combine(projectRoot, directory);
        if (!Path::createDirectoryRecursive(path))
        {
            outError = String("Cannot create directory: ") + path;
            return SLANG_FAIL;
        }
    }

    manifest.version = "0.1.0";
    manifest.exports.add("src");
    SLANG_RETURN_ON_FAIL(writeManifest(manifestPath, manifest, outError));
    fprintf(stdout, "Initialized package '%s'.\n", manifest.name.getBuffer());
    return SLANG_OK;
}

static SlangResult _fetch(const String& projectRoot, bool lockedOnly, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    String lockPath = Path::combine(projectRoot, kLockName);
    LockFile lock;
    if (File::exists(lockPath))
    {
        SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
        SLANG_RETURN_ON_FAIL(_validateLockAgainstManifest(projectRoot, manifest, lock, outError));
    }
    else
    {
        if (lockedOnly)
        {
            outError = "fetch --locked requires slang-package.lock.";
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(resolveDependencies(projectRoot, manifest, lock, outError));
        SLANG_RETURN_ON_FAIL(writeLockFile(lockPath, lock, outError));
    }

    SLANG_RETURN_ON_FAIL(_materialize(projectRoot, lock, outError));
    fprintf(stdout, "Fetched %lld package(s).\n", (long long)lock.packages.getCount());
    return SLANG_OK;
}

static SlangResult _update(const String& projectRoot, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    LockFile lock;
    SLANG_RETURN_ON_FAIL(resolveDependencies(projectRoot, manifest, lock, outError));
    SLANG_RETURN_ON_FAIL(writeLockFile(Path::combine(projectRoot, kLockName), lock, outError));
    SLANG_RETURN_ON_FAIL(_materialize(projectRoot, lock, outError));
    fprintf(stdout, "Updated %lld package(s).\n", (long long)lock.packages.getCount());
    return SLANG_OK;
}

static SlangResult _edit(const String& projectRoot, const String& name, String& outError)
{
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    LockedPackage* package = _findLockedPackage(lock, name);
    if (!package)
    {
        outError = String("Package is not present in the lock file: ") + name;
        return SLANG_FAIL;
    }
    String editRoot = Path::combine(projectRoot, ".slang", "edit");
    if (!Path::createDirectoryRecursive(editRoot))
    {
        outError = String("Cannot create editable package directory: ") + editRoot;
        return SLANG_FAIL;
    }
    String destination = Path::combine(editRoot, name);
    SlangPathType type;
    if (SLANG_SUCCEEDED(Path::getPathType(destination, &type)))
    {
        outError = String("Editable checkout path already exists: ") + destination;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        materializeRevision(projectRoot, package->git, package->commit, destination, outError));
    SLANG_RETURN_ON_FAIL(_writeSearchPaths(projectRoot, lock, outError));
    fprintf(stdout, "Package '%s' is now editable.\n", name.getBuffer());
    return SLANG_OK;
}

static SlangResult _unedit(const String& projectRoot, const String& name, String& outError)
{
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    LockedPackage* package = _findLockedPackage(lock, name);
    if (!package)
    {
        outError = String("Package is not present in the lock file: ") + name;
        return SLANG_FAIL;
    }
    String destination = Path::combine(projectRoot, Path::combine(".slang", "edit", name));
    SlangPathType type;
    if (SLANG_FAILED(Path::getPathType(destination, &type)) || type != SLANG_PATH_TYPE_DIRECTORY)
    {
        outError = String("Package is not editable: ") + name;
        return SLANG_FAIL;
    }
    bool isSafeToRemove = false;
    SLANG_RETURN_ON_FAIL(
        isWorkingTreeSafeToRemove(destination, package->commit, isSafeToRemove, outError));
    if (!isSafeToRemove)
    {
        outError =
            String("Editable checkout has local changes, commits, or stashes; refusing to remove "
                   "it: ") +
            destination;
        return SLANG_FAIL;
    }
    if (SLANG_FAILED(Path::removeNonEmpty(destination)))
    {
        outError = String("Cannot remove editable checkout: ") + destination;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(_writeSearchPaths(projectRoot, lock, outError));
    fprintf(stdout, "Package '%s' is no longer editable.\n", name.getBuffer());
    return SLANG_OK;
}

SlangResult executeInDirectory(
    const String& projectRoot,
    int argc,
    const char* const* argv,
    String& outError)
{
    if (argc < 2 || String(argv[1]) == "help" || String(argv[1]) == "-help" ||
        String(argv[1]) == "--help")
    {
        _printHelp();
        return SLANG_OK;
    }

    String command = argv[1];
    if (command == "init" && argc == 2)
        return _init(projectRoot, outError);
    if (command == "fetch")
    {
        if (argc == 2)
            return _fetch(projectRoot, false, outError);
        if (argc == 3 && String(argv[2]) == "--locked")
            return _fetch(projectRoot, true, outError);
    }
    if (command == "update" && argc == 2)
        return _update(projectRoot, outError);
    if (command == "edit" && argc == 3)
        return _edit(projectRoot, argv[2], outError);
    if (command == "unedit" && argc == 3)
        return _unedit(projectRoot, argv[2], outError);

    outError = String("Invalid command or arguments. Run '") + argv[0] + " help'.";
    return SLANG_FAIL;
}

int execute(int argc, const char* const* argv)
{
    String error;
    String projectRoot;
    if (SLANG_FAILED(_getProjectRoot(projectRoot, error)) ||
        SLANG_FAILED(executeInDirectory(projectRoot, argc, argv, error)))
    {
        fprintf(stderr, "slang-package: error: %s\n", error.getBuffer());
        return 1;
    }
    return 0;
}

} // namespace PackageTool
} // namespace Slang
