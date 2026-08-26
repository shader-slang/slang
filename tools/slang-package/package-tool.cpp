// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-tool.h"

#include "core/slang-io.h"
#include "core/slang-string-util.h"
#include "package-git.h"
#include "package-json.h"
#include "package-local.h"
#include "package-lock.h"
#include "package-resolver.h"
#include "package-validate.h"

#include <stdio.h>

namespace Slang
{
namespace PackageTool
{

static const char* const kManifestName = "slang-package.json";
static const char* const kLockName = "slang-package-lock.json";

static bool _isPathWithin(const String& canonicalRoot, const String& canonicalPath)
{
    if (canonicalPath == canonicalRoot)
        return true;
    UnownedStringSlice root = canonicalRoot.getUnownedSlice();
    UnownedStringSlice path = canonicalPath.getUnownedSlice();
    return path.startsWith(root) && path.getLength() > root.getLength() &&
           Path::isDelimiter(path[root.getLength()]);
}

static void _printHelp()
{
    fprintf(
        stdout,
        "Usage: slang-package <command>\n"
        "\n"
        "Commands:\n"
        "  init             Create a package manifest and standard directories.\n"
        "  fetch            Materialize dependencies from the lock file.\n"
        "  update           Re-resolve dependencies and update the lock file.\n"
        "  update --from-local\n"
        "                   Resolve registered local package manifests into the lock.\n"
        "  validate         Validate package structure and the locked dependency closure.\n"
        "  edit <name>      Create an editable dependency checkout.\n"
        "  unedit <name>    Remove an editable checkout.\n"
        "  override <name> <path>\n"
        "                   Use an existing local package directory.\n"
        "  unoverride <name>\n"
        "                   Stop using an existing local package directory.\n"
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

static SlangResult _writeSearchPaths(
    const String& projectRoot,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError)
{
    StringBuilder searchPaths;
    for (const auto& package : lock.packages)
    {
        Index localIndex = findLocalPackageIndex(localPackages, package.name);
        if (localIndex >= 0)
        {
            for (const auto& exportPath : package.exports)
                searchPaths << Path::combine(localPackages[localIndex].path, exportPath) << "\n";
            continue;
        }
        if (package.path.getLength())
        {
            if (package.git.getLength())
            {
                outError = String("Locked local override '") + package.name +
                           "' is not registered in .slang/overrides.json.";
                return SLANG_FAIL;
            }
            for (const auto& exportPath : package.exports)
                searchPaths << Path::combine(package.path, exportPath) << "\n";
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

static SlangResult _clearSearchPaths(const String& projectRoot, String& outError)
{
    String stateRoot = Path::combine(projectRoot, ".slang");
    if (!Path::createDirectoryRecursive(stateRoot) ||
        SLANG_FAILED(File::writeAllText(Path::combine(stateRoot, "search-paths"), "")))
    {
        outError = "Cannot clear .slang/search-paths before materializing packages.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _materialize(
    const String& projectRoot,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError)
{
    String packagesRoot = Path::combine(projectRoot, ".slang", "packages");
    if (!Path::createDirectoryRecursive(packagesRoot))
    {
        outError = String("Cannot create package directory: ") + packagesRoot;
        return SLANG_FAIL;
    }

    for (const auto& package : lock.packages)
    {
        if (findLocalPackageIndex(localPackages, package.name) >= 0)
            continue;
        if (package.path.getLength())
        {
            if (package.git.getLength())
            {
                outError = String("Locked local override '") + package.name +
                           "' is not registered in .slang/overrides.json.";
                return SLANG_FAIL;
            }
            continue;
        }

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
    return SLANG_OK;
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

/// Verify that the lock is exactly the reachable graph required by its stored package manifests.
///
/// Each lock entry stores the dependency requirements from the manifest that produced it. This
/// lets fetch validate both Git and local package graphs without rediscovering metadata.
static SlangResult _validateLockAgainstManifest(
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
        SLANG_RETURN_ON_FAIL(validateLockedDependency(dependency, lock, packageIndex, outError));
        if (!reachablePackages[packageIndex])
        {
            reachablePackages[packageIndex] = true;
            pendingPackages.add(packageIndex);
        }
    }

    for (Index pendingIndex = 0; pendingIndex < pendingPackages.getCount(); ++pendingIndex)
    {
        const LockedPackage& package = lock.packages[pendingPackages[pendingIndex]];
        for (const auto& dependency : package.dependencies)
        {
            Index dependencyIndex;
            SLANG_RETURN_ON_FAIL(
                validateLockedDependency(dependency, lock, dependencyIndex, outError));
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

/// Verify that every registered local tree matches its locked slot and every path lock is
/// registered.
static SlangResult _validateLocalPackages(
    const String& projectRoot,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError)
{
    for (const auto& localPackage : localPackages)
    {
        Index packageIndex = findLockedPackageIndex(lock, localPackage.name);
        if (packageIndex < 0)
        {
            outError =
                String("Registered local package is not present in the lock: ") + localPackage.name;
            return SLANG_FAIL;
        }
        const LockedPackage* package = &lock.packages[packageIndex];
        if (package->path.getLength() && package->path != localPackage.path)
        {
            outError = String("Locked path for package '") + package->name +
                       "' does not match .slang/overrides.json. Run "
                       "'slang package update --from-local'.";
            return SLANG_FAIL;
        }
        Manifest manifest;
        SLANG_RETURN_ON_FAIL(
            readLocalPackageManifest(projectRoot, localPackage, manifest, outError));
        if (SLANG_FAILED(validateLockedPackageManifest(*package, manifest, outError)))
        {
            outError = outError +
                       " Align the local manifest with the selected upstream graph, or run "
                       "'slang package update --from-local' to record local manifest changes.";
            return SLANG_FAIL;
        }
    }
    for (const auto& package : lock.packages)
    {
        if (package.path.getLength() && package.git.getLength() &&
            findLocalPackageIndex(localPackages, package.name) < 0)
        {
            outError = String("Locked local package '") + package.name +
                       "' is not registered in .slang/overrides.json. Run "
                       "'slang package update' to restore a published pin.";
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

static SlangResult _validateLockedPath(
    const String& projectRoot,
    const String& declaringRoot,
    const Dependency& dependency,
    const LockedPackage& lockedPackage,
    String& outError)
{
    if (!dependency.path.getLength())
        return SLANG_OK;
    String canonicalExpectedPath;
    String canonicalLockedPath;
    if (SLANG_FAILED(Path::getCanonical(
            Path::combine(declaringRoot, dependency.path),
            canonicalExpectedPath)) ||
        SLANG_FAILED(Path::getCanonical(
            Path::combine(projectRoot, lockedPackage.path),
            canonicalLockedPath)) ||
        canonicalExpectedPath != canonicalLockedPath)
    {
        outError = String("Locked path does not match dependency '") + dependency.name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }
    String canonicalStateRoot;
    String canonicalDeclaringRoot;
    if (SLANG_SUCCEEDED(
            Path::getCanonical(Path::combine(projectRoot, ".slang"), canonicalStateRoot)) &&
        SLANG_SUCCEEDED(Path::getCanonical(declaringRoot, canonicalDeclaringRoot)) &&
        _isPathWithin(canonicalStateRoot, canonicalExpectedPath) &&
        !_isPathWithin(canonicalDeclaringRoot, canonicalExpectedPath))
    {
        outError = String("Path dependency cannot use package-tool state under .slang: ") +
                   dependency.name;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

/// Verify the materialized manifest and path identity for every package in the lock.
static SlangResult _validateMaterializedManifests(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
    bool allowLocalManifestChanges = false)
{
    List<bool> trusted;
    List<bool> processed;
    trusted.setCount(lock.packages.getCount());
    processed.setCount(lock.packages.getCount());
    for (Index i = 0; i < lock.packages.getCount(); ++i)
    {
        trusted[i] = false;
        processed[i] = false;
    }
    List<Index> pending;

    for (const auto& dependency : rootManifest.dependencies)
    {
        Index packageIndex = findLockedPackageIndex(lock, dependency.name);
        SLANG_RELEASE_ASSERT(packageIndex >= 0);
        SLANG_RETURN_ON_FAIL(_validateLockedPath(
            projectRoot,
            projectRoot,
            dependency,
            lock.packages[packageIndex],
            outError));
        const LockedPackage& package = lock.packages[packageIndex];
        if (dependency.path.getLength() || package.git.getLength())
        {
            trusted[packageIndex] = true;
            pending.add(packageIndex);
        }
    }

    for (Index pendingIndex = 0; pendingIndex < pending.getCount(); ++pendingIndex)
    {
        Index packageIndex = pending[pendingIndex];
        if (processed[packageIndex])
            continue;
        processed[packageIndex] = true;
        const LockedPackage& package = lock.packages[packageIndex];
        String packageRoot;
        Index localIndex = findLocalPackageIndex(localPackages, package.name);
        if (localIndex >= 0)
        {
            SLANG_RETURN_ON_FAIL(
                getLocalPackageRoot(projectRoot, localPackages[localIndex], packageRoot, outError));
        }
        else if (package.path.getLength())
        {
            packageRoot = Path::combine(projectRoot, package.path);
        }
        else
        {
            packageRoot =
                Path::combine(Path::combine(projectRoot, ".slang", "packages"), package.name);
        }

        Manifest manifest;
        if (SLANG_FAILED(
                readManifest(Path::combine(packageRoot, kManifestName), manifest, outError)))
        {
            outError =
                String("Cannot read locked package manifest '") + package.name + "'. " + outError;
            return SLANG_FAIL;
        }
        if (!(allowLocalManifestChanges && localIndex >= 0))
            SLANG_RETURN_ON_FAIL(validateLockedPackageManifest(package, manifest, outError));
        const List<Dependency>& dependencies = allowLocalManifestChanges && localIndex >= 0
                                                   ? package.dependencies
                                                   : manifest.dependencies;
        for (const auto& dependency : dependencies)
        {
            Index dependencyIndex = findLockedPackageIndex(lock, dependency.name);
            SLANG_RELEASE_ASSERT(dependencyIndex >= 0);
            SLANG_RETURN_ON_FAIL(_validateLockedPath(
                projectRoot,
                packageRoot,
                dependency,
                lock.packages[dependencyIndex],
                outError));
            const LockedPackage& dependencyPackage = lock.packages[dependencyIndex];
            if (dependency.path.getLength() || dependencyPackage.git.getLength())
            {
                if (!trusted[dependencyIndex])
                {
                    trusted[dependencyIndex] = true;
                    pending.add(dependencyIndex);
                }
            }
        }
    }
    for (Index i = 0; i < lock.packages.getCount(); ++i)
    {
        if (!processed[i])
        {
            outError = String("Locked path package '") + lock.packages[i].name +
                       "' is not selected by a trusted path dependency.";
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

static SlangResult _writeValidatedSearchPathsAfterLocalChange(
    const String& projectRoot,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    SLANG_RETURN_ON_FAIL(
        _validateMaterializedManifests(projectRoot, manifest, lock, localPackages, outError, true));
    return _writeSearchPaths(projectRoot, lock, localPackages, outError);
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

    manifest.exports.add("src");
    manifest.licenseFiles.add("LICENSE");
    String licensePath = Path::combine(projectRoot, "LICENSE");
    if (!File::exists(licensePath) &&
        SLANG_FAILED(File::writeAllText(licensePath, getLicensePlaceholderText())))
    {
        outError = String("Cannot create license placeholder: ") + licensePath;
        return SLANG_FAIL;
    }
    String gitIgnorePath = Path::combine(projectRoot, ".gitignore");
    String gitIgnore;
    if (File::exists(gitIgnorePath) && SLANG_FAILED(File::readAllText(gitIgnorePath, gitIgnore)))
    {
        outError = String("Cannot read .gitignore: ") + gitIgnorePath;
        return SLANG_FAIL;
    }
    bool ignoresPackageState = false;
    for (auto line : LineParser(gitIgnore.getUnownedSlice()))
    {
        UnownedStringSlice trimmed = line.trim();
        ignoresPackageState = ignoresPackageState || trimmed == ".slang" || trimmed == ".slang/";
    }
    if (!ignoresPackageState)
    {
        StringBuilder updatedIgnore;
        updatedIgnore << gitIgnore;
        if (gitIgnore.getLength() && gitIgnore[gitIgnore.getLength() - 1] != '\n')
            updatedIgnore << "\n";
        updatedIgnore << ".slang/\n";
        if (SLANG_FAILED(File::writeAllText(gitIgnorePath, updatedIgnore)))
        {
            outError = String("Cannot add package-local state to .gitignore: ") + gitIgnorePath;
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(writeManifest(manifestPath, manifest, outError));
    fprintf(stdout, "Initialized package '%s'.\n", manifest.name.getBuffer());
    return SLANG_OK;
}

static SlangResult _fetch(const String& projectRoot, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    String lockPath = Path::combine(projectRoot, kLockName);
    if (!File::exists(lockPath))
    {
        outError =
            "fetch requires slang-package-lock.json. Run 'slang package update' to create it.";
        return SLANG_FAIL;
    }

    LockFile lock;
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_validateLockAgainstManifest(manifest, lock, outError));
    SLANG_RETURN_ON_FAIL(_validateLocalPackages(projectRoot, lock, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_clearSearchPaths(projectRoot, outError));
    SLANG_RETURN_ON_FAIL(_materialize(projectRoot, lock, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _validateMaterializedManifests(projectRoot, manifest, lock, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_writeSearchPaths(projectRoot, lock, localPackages, outError));
    fprintf(stdout, "Fetched %lld package(s).\n", (long long)lock.packages.getCount());
    return SLANG_OK;
}

static SlangResult _update(const String& projectRoot, bool fromLocal, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    if (fromLocal && localPackages.getCount() == 0)
    {
        outError = "update --from-local requires a registered local package.";
        return SLANG_FAIL;
    }

    LockFile lock;
    List<String> warnings;
    if (fromLocal)
    {
        SLANG_RETURN_ON_FAIL(resolveDependenciesFromLocalPackages(
            projectRoot,
            manifest,
            localPackages,
            lock,
            outError,
            &warnings));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(resolveDependencies(projectRoot, manifest, lock, outError, &warnings));
    }
    SLANG_RETURN_ON_FAIL(_validateLocalPackages(projectRoot, lock, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_clearSearchPaths(projectRoot, outError));
    SLANG_RETURN_ON_FAIL(_materialize(projectRoot, lock, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _validateMaterializedManifests(projectRoot, manifest, lock, localPackages, outError));
    SLANG_RETURN_ON_FAIL(writeLockFile(Path::combine(projectRoot, kLockName), lock, outError));
    SLANG_RETURN_ON_FAIL(_writeSearchPaths(projectRoot, lock, localPackages, outError));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    if (fromLocal)
    {
        fprintf(
            stdout,
            "The lock now contains local paths and requires this project's "
            ".slang/overrides.json.\n");
    }
    fprintf(stdout, "Updated %lld package(s).\n", (long long)lock.packages.getCount());
    return SLANG_OK;
}

static SlangResult _validate(const String& projectRoot, String& outError)
{
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(validateProject(projectRoot, outError, &warnings));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    fprintf(stdout, "Package and locked dependencies are valid.\n");
    return SLANG_OK;
}

static SlangResult _registerLocalPackage(
    const String& projectRoot,
    const String& name,
    const String& path,
    const String& baseCommit,
    List<LocalPackage>& ioPackages,
    String& outError)
{
    if (findLocalPackageIndex(ioPackages, name) >= 0)
    {
        outError = String("Package already has a registered local tree: ") + name;
        return SLANG_FAIL;
    }

    String inputPath = Path::isAbsolute(path) ? path : Path::combine(projectRoot, path);
    String canonicalPath;
    SlangPathType type;
    if (SLANG_FAILED(Path::getPathType(inputPath, &type)) || type != SLANG_PATH_TYPE_DIRECTORY ||
        SLANG_FAILED(Path::getCanonical(inputPath, canonicalPath)))
    {
        outError = String("Local package directory does not exist: ") + path;
        return SLANG_FAIL;
    }
    String relativePath = Path::getRelativePath(projectRoot, canonicalPath);
    if (Path::isAbsolute(relativePath))
    {
        outError = "Local package must be on the same filesystem as the project.";
        return SLANG_FAIL;
    }

    LocalPackage package;
    package.name = name;
    package.path = relativePath;
    package.baseCommit = baseCommit;
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(readLocalPackageManifest(projectRoot, package, manifest, outError));
    ioPackages.add(package);
    ioPackages.sort([](const LocalPackage& left, const LocalPackage& right)
                    { return left.name < right.name; });
    return writeProjectLocalPackages(projectRoot, ioPackages, outError);
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
    if (package->path.getLength())
    {
        outError = String("Package already has a local path in the lock: ") + name;
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
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    if (SLANG_FAILED(_registerLocalPackage(
            projectRoot,
            name,
            destination,
            package->commit,
            localPackages,
            outError)))
    {
        Path::removeNonEmpty(destination);
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
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
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    Index localIndex = findLocalPackageIndex(localPackages, name);
    if (localIndex < 0 || !localPackages[localIndex].baseCommit.getLength())
    {
        outError = String("Package is not editable: ") + name;
        return SLANG_FAIL;
    }
    if (package->path.getLength())
    {
        outError = String("The lock still points at this editable package. Run "
                          "'slang package update' before unedit.");
        return SLANG_FAIL;
    }
    String destination;
    SLANG_RETURN_ON_FAIL(
        getLocalPackageRoot(projectRoot, localPackages[localIndex], destination, outError));
    String canonicalEditRoot;
    if (SLANG_FAILED(
            Path::getCanonical(Path::combine(projectRoot, ".slang", "edit"), canonicalEditRoot)) ||
        Path::getParentDirectory(destination) != canonicalEditRoot)
    {
        outError = String("Editable package registration points outside .slang/edit: ") + name;
        return SLANG_FAIL;
    }
    SlangPathType type;
    if (SLANG_FAILED(Path::getPathType(destination, &type)) || type != SLANG_PATH_TYPE_DIRECTORY)
    {
        outError = String("Package is not editable: ") + name;
        return SLANG_FAIL;
    }
    bool isSafeToRemove = false;
    SLANG_RETURN_ON_FAIL(isWorkingTreeSafeToRemove(
        destination,
        localPackages[localIndex].baseCommit,
        isSafeToRemove,
        outError));
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
    localPackages.removeAt(localIndex);
    SLANG_RETURN_ON_FAIL(writeProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    fprintf(stdout, "Package '%s' is no longer editable.\n", name.getBuffer());
    return SLANG_OK;
}

static SlangResult _override(
    const String& projectRoot,
    const String& name,
    const String& path,
    String& outError)
{
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    LockedPackage* lockedPackage = _findLockedPackage(lock, name);
    if (lockedPackage && lockedPackage->path.getLength() && !lockedPackage->git.getLength())
    {
        outError = String("Manifest path dependency cannot be overridden: ") + name;
        return SLANG_FAIL;
    }

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _registerLocalPackage(projectRoot, name, path, String(), localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    fprintf(
        stdout,
        "Package '%s' now uses '%s'. Run 'slang package update --from-local' if its manifest "
        "differs from the lock.\n",
        name.getBuffer(),
        path.getBuffer());
    return SLANG_OK;
}

static SlangResult _unoverride(const String& projectRoot, const String& name, String& outError)
{
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    Index packageIndex = findLockedPackageIndex(lock, name);
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    Index localIndex = findLocalPackageIndex(localPackages, name);
    if (localIndex < 0)
    {
        outError = String("Package has no registered local tree: ") + name;
        return SLANG_FAIL;
    }
    if (localPackages[localIndex].baseCommit.getLength())
    {
        outError = String("Package is editable; use 'slang package unedit ") + name + "'.";
        return SLANG_FAIL;
    }
    if (packageIndex >= 0 && lock.packages[packageIndex].path.getLength())
    {
        outError = String("The lock still points at this local package. Run "
                          "'slang package update' before unoverride.");
        return SLANG_FAIL;
    }
    localPackages.removeAt(localIndex);
    SLANG_RETURN_ON_FAIL(writeProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    fprintf(stdout, "Package '%s' no longer uses a local override.\n", name.getBuffer());
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
    if (command == "fetch" && argc == 2)
        return _fetch(projectRoot, outError);
    if (command == "update" && argc == 2)
        return _update(projectRoot, false, outError);
    if (command == "update" && argc == 3 && String(argv[2]) == "--from-local")
        return _update(projectRoot, true, outError);
    if (command == "validate" && argc == 2)
        return _validate(projectRoot, outError);
    if (command == "edit" && argc == 3)
        return _edit(projectRoot, argv[2], outError);
    if (command == "unedit" && argc == 3)
        return _unedit(projectRoot, argv[2], outError);
    if (command == "override" && argc == 4)
        return _override(projectRoot, argv[2], argv[3], outError);
    if (command == "unoverride" && argc == 3)
        return _unoverride(projectRoot, argv[2], outError);

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
