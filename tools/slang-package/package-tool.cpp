// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-tool.h"

#include "core/slang-io.h"
#include "core/slang-platform.h"
#include "core/slang-process-util.h"
#include "core/slang-string-util.h"
#include "package-docs.h"
#include "package-git.h"
#include "package-json.h"
#include "package-local.h"
#include "package-lock.h"
#include "package-path.h"
#include "package-resolver.h"
#include "package-validate.h"

#include <stdio.h>

namespace Slang
{
namespace PackageTool
{

static const char* const kManifestName = "slang-package.json";
static const char* const kLockName = "slang-package-lock.json";

static void _printHelp()
{
    fprintf(
        stdout,
        "Usage: slang-package <command>\n"
        "\n"
        "Commands:\n"
        "  init             Create a package manifest and standard directories.\n"
        "  fetch [--clean]  Materialize dependencies from the lock file.\n"
        "  update [--clean] Re-resolve dependencies and update the lock file.\n"
        "  update --from-local [--clean]\n"
        "                   Resolve registered local package manifests into the lock.\n"
        "  build            Compile modules, an optional executable, and package docs.\n"
        "  run [args...]    Run the executable produced by the last build.\n"
        "  test             Run slang-test on the workspace tests directory.\n"
        "  docs             Print the location of generated documentation (build/docs).\n"
        "  validate         Validate package structure and the locked dependency closure.\n"
        "  edit <name>      Make a dependency checkout editable in place.\n"
        "  unedit <name>    Return an unchanged checkout to tool ownership.\n"
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
    const Manifest& manifest,
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
        if (isPathOnlyLockedPackage(package))
        {
            for (const auto& exportPath : package.exports)
                searchPaths << Path::combine(package.path, exportPath) << "\n";
            continue;
        }
        if (isLocalOverrideLockedPackage(package))
        {
            outError = String("Locked local override '") + package.name +
                       "' is not registered in slang-workspace.json.";
            return SLANG_FAIL;
        }

        String packageRoot = Path::combine(getWorkspaceDepsDirectory(manifest), package.name);
        for (const auto& exportPath : package.exports)
            searchPaths << Path::combine(packageRoot, exportPath) << "\n";
    }

    String buildDirectory = Path::combine(projectRoot, getWorkspaceBuildDirectory(manifest));
    if (!Path::createDirectoryRecursive(buildDirectory) ||
        SLANG_FAILED(
            File::writeAllText(Path::combine(buildDirectory, "search-paths"), searchPaths)))
    {
        outError = "Cannot write the workspace build/search-paths file.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _clearSearchPaths(
    const String& projectRoot,
    const Manifest& manifest,
    String& outError)
{
    String buildDirectory = Path::combine(projectRoot, getWorkspaceBuildDirectory(manifest));
    if (!Path::createDirectoryRecursive(buildDirectory) ||
        SLANG_FAILED(File::writeAllText(Path::combine(buildDirectory, "search-paths"), "")))
    {
        outError = "Cannot clear build/search-paths before materializing packages.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _materialize(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const LockFile* previousLock,
    const List<LocalPackage>& localPackages,
    bool allowClean,
    String& outError)
{
    String depsRoot = Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest));
    if (!Path::createDirectoryRecursive(depsRoot))
    {
        outError = String("Cannot create dependency directory: ") + depsRoot;
        return SLANG_FAIL;
    }

    for (const auto& package : lock.packages)
    {
        if (findLocalPackageIndex(localPackages, package.name) >= 0)
            continue;
        if (isPathOnlyLockedPackage(package))
            continue;
        if (isLocalOverrideLockedPackage(package))
        {
            outError = String("Locked local override '") + package.name +
                       "' is not registered in slang-workspace.json.";
            return SLANG_FAIL;
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

        String currentCommit;
        if (previousLock)
        {
            Index previousIndex = findLockedPackageIndex(*previousLock, package.name);
            if (previousIndex >= 0)
            {
                const LockedPackage& previousPackage = previousLock->packages[previousIndex];
                if (previousPackage.git == package.git && !previousPackage.path.getLength())
                    currentCommit = previousPackage.commit;
            }
        }
        String destination = Path::combine(depsRoot, package.name);
        SLANG_RETURN_ON_FAIL(materializeLockedRevision(
            projectRoot,
            package.git,
            currentCommit,
            package.commit,
            destination,
            allowClean,
            outError));
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
                       "' does not match slang-workspace.json. Run "
                       "'slang package update --from-local'.";
            return SLANG_FAIL;
        }
        Manifest manifest;
        SLANG_RETURN_ON_FAIL(
            readLocalPackageManifest(projectRoot, localPackage, manifest, outError));
        if (SLANG_FAILED(validateLockedPackageManifest(*package, manifest, outError)))
        {
            outError =
                isEditedLocalPackage(localPackage)
                    ? outError +
                          " An edit retains its published Git pin; publish a new release tag and "
                          "run 'slang package update', or use an override for local manifest "
                          "changes."
                    : outError +
                          " Align the local manifest with the selected upstream graph, or run "
                          "'slang package update --from-local' to record local manifest changes.";
            return SLANG_FAIL;
        }
    }
    for (const auto& package : lock.packages)
    {
        if (isLocalOverrideLockedPackage(package) &&
            findLocalPackageIndex(localPackages, package.name) < 0)
        {
            outError = String("Locked local package '") + package.name +
                       "' is not registered in slang-workspace.json. Run "
                       "'slang package update' to restore a published pin.";
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

static SlangResult _validateMaterializedManifests(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
    bool allowLocalManifestChanges = false)
{
    List<bool> trusted;
    trusted.setCount(lock.packages.getCount());
    for (Index i = 0; i < lock.packages.getCount(); ++i)
        trusted[i] = false;
    List<Index> pending;

    for (const auto& dependency : rootManifest.dependencies)
    {
        Index packageIndex = findLockedPackageIndex(lock, dependency.name);
        SLANG_RELEASE_ASSERT(packageIndex >= 0);
        SLANG_RETURN_ON_FAIL(validateLockedPathDependency(
            projectRoot,
            projectRoot,
            rootManifest.name,
            dependency,
            lock.packages[packageIndex],
            outError));
        if (isTrustedLockSelection(dependency, lock.packages[packageIndex]) &&
            !trusted[packageIndex])
        {
            trusted[packageIndex] = true;
            pending.add(packageIndex);
        }
    }

    for (Index pendingIndex = 0; pendingIndex < pending.getCount(); ++pendingIndex)
    {
        Index packageIndex = pending[pendingIndex];
        const LockedPackage& package = lock.packages[packageIndex];
        String packageRoot;
        SLANG_RETURN_ON_FAIL(getLockedPackageRoot(
            projectRoot,
            getWorkspaceDepsDirectory(rootManifest),
            package,
            localPackages,
            packageRoot,
            outError));
        Index localIndex = findLocalPackageIndex(localPackages, package.name);

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
            SLANG_RETURN_ON_FAIL(validateLockedPathDependency(
                projectRoot,
                packageRoot,
                manifest.name,
                dependency,
                lock.packages[dependencyIndex],
                outError));
            if (isTrustedLockSelection(dependency, lock.packages[dependencyIndex]) &&
                !trusted[dependencyIndex])
            {
                trusted[dependencyIndex] = true;
                pending.add(dependencyIndex);
            }
        }
    }
    return requireAllLockPackagesTrusted(lock, trusted, outError);
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
    return _writeSearchPaths(projectRoot, manifest, lock, localPackages, outError);
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

    static const char* const kDirectories[] = {"src", "tests", "docs", "deps", "build"};
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
    manifest.workspace.depsDirectory = "deps";
    manifest.workspace.buildDirectory = "build";
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
    static const char* const kIgnoredWorkspacePaths[] = {
        ".slang/",
        "deps/",
        "build/",
        "slang-workspace.json",
    };
    StringBuilder updatedIgnore;
    updatedIgnore << gitIgnore;
    for (auto ignoredPath : kIgnoredWorkspacePaths)
    {
        bool found = false;
        for (auto line : LineParser(gitIgnore.getUnownedSlice()))
        {
            found = found || line.trim() == ignoredPath;
        }
        if (found)
            continue;
        if (updatedIgnore.getLength() &&
            updatedIgnore.getBuffer()[updatedIgnore.getLength() - 1] != '\n')
            updatedIgnore << "\n";
        updatedIgnore << ignoredPath << "\n";
    }
    if (SLANG_FAILED(File::writeAllText(gitIgnorePath, updatedIgnore)))
    {
        outError = String("Cannot add workspace state to .gitignore: ") + gitIgnorePath;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(writeManifest(manifestPath, manifest, outError));
    fprintf(stdout, "Initialized package '%s'.\n", manifest.name.getBuffer());
    return SLANG_OK;
}

static SlangResult _fetch(const String& projectRoot, bool allowClean, String& outError)
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
    SLANG_RETURN_ON_FAIL(_clearSearchPaths(projectRoot, manifest, outError));
    SLANG_RETURN_ON_FAIL(
        _materialize(projectRoot, manifest, lock, &lock, localPackages, allowClean, outError));
    SLANG_RETURN_ON_FAIL(
        _validateMaterializedManifests(projectRoot, manifest, lock, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_writeSearchPaths(projectRoot, manifest, lock, localPackages, outError));
    fprintf(stdout, "Fetched %lld package(s).\n", (long long)lock.packages.getCount());
    return SLANG_OK;
}

static SlangResult _update(
    const String& projectRoot,
    bool fromLocal,
    bool allowClean,
    String& outError)
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

    LockFile previousLock;
    LockFile* previousLockPtr = nullptr;
    String lockPath = Path::combine(projectRoot, kLockName);
    if (File::exists(lockPath))
    {
        SLANG_RETURN_ON_FAIL(readLockFile(lockPath, previousLock, outError));
        previousLockPtr = &previousLock;
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
    SLANG_RETURN_ON_FAIL(_clearSearchPaths(projectRoot, manifest, outError));
    SLANG_RETURN_ON_FAIL(_materialize(
        projectRoot,
        manifest,
        lock,
        previousLockPtr,
        localPackages,
        allowClean,
        outError));
    SLANG_RETURN_ON_FAIL(
        _validateMaterializedManifests(projectRoot, manifest, lock, localPackages, outError));
    SLANG_RETURN_ON_FAIL(writeLockFile(lockPath, lock, outError));
    SLANG_RETURN_ON_FAIL(_writeSearchPaths(projectRoot, manifest, lock, localPackages, outError));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    if (fromLocal)
    {
        fprintf(
            stdout,
            "The workspace contains local package state and requires slang-workspace.json.\n");
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

/// Return the absolute export roots used to compile workspace modules from source.
static SlangResult _collectCompilationSearchPaths(
    const String& projectRoot,
    const Manifest& manifest,
    List<String>& outSearchPaths,
    String& outError)
{
    outSearchPaths.clear();
    for (const auto& exportPath : manifest.exports)
        outSearchPaths.add(Path::combine(projectRoot, exportPath));

    String lockPath = Path::combine(projectRoot, kLockName);
    if (!File::exists(lockPath))
        return SLANG_OK;

    LockFile lock;
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    for (const auto& package : lock.packages)
    {
        String packageRoot;
        SLANG_RETURN_ON_FAIL(getLockedPackageRoot(
            projectRoot,
            getWorkspaceDepsDirectory(manifest),
            package,
            localPackages,
            packageRoot,
            outError));
        for (const auto& exportPath : package.exports)
            outSearchPaths.add(Path::combine(packageRoot, exportPath));
    }
    return SLANG_OK;
}

/// Locate an installed tool beside `slang-package`, matching the layout produced by Slang builds
/// and release packages.
static SlangResult _findSiblingTool(
    const char* toolName,
    String& outExecutablePath,
    String& outError)
{
    StringBuilder fileName;
    fileName << toolName << Process::getExecutableSuffix();
    outExecutablePath = Path::combine(
        Path::getParentDirectory(Path::getExecutablePath()),
        fileName.produceString());
    if (!File::exists(outExecutablePath))
    {
        outError =
            String("Cannot find required '") + toolName + "' executable beside slang-package.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

/// Execute a sibling Slang tool and preserve its output for an interactive package command.
static SlangResult _runSiblingTool(
    const String& executablePath,
    const List<String>& arguments,
    String& outError)
{
    CommandLine commandLine;
    commandLine.setExecutableLocation(
        ExecutableLocation(ExecutableLocation::Type::Path, executablePath));
    for (const auto& argument : arguments)
        commandLine.addArg(argument);

    ExecuteResult result;
    if (SLANG_FAILED(ProcessUtil::execute(commandLine, result)))
    {
        outError = String("Cannot execute: ") + commandLine.toString();
        return SLANG_FAIL;
    }
    if (result.standardOutput.getLength())
        fprintf(stdout, "%s", result.standardOutput.getBuffer());
    if (result.standardError.getLength())
        fprintf(stderr, "%s", result.standardError.getBuffer());
    if (result.resultCode != 0)
    {
        outError = result.standardError.trim();
        if (!outError.getLength())
            outError = String("Command failed: ") + commandLine.toString();
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

/// Execute a sibling tool while forwarding its output as it is produced. This is used for
/// long-running or user-visible commands; compiler subprocesses use `_runSiblingTool` so their
/// diagnostics can be attached directly to the package error.
static SlangResult _runStreamingSiblingTool(
    const String& executablePath,
    const List<String>& arguments,
    String& outError)
{
    CommandLine commandLine;
    commandLine.setExecutableLocation(
        ExecutableLocation(ExecutableLocation::Type::Path, executablePath));
    for (const auto& argument : arguments)
        commandLine.addArg(argument);

    RefPtr<Process> process;
    if (SLANG_FAILED(
            Process::create(commandLine, Process::Flag::DisableStdErrRedirection, process)))
    {
        outError = String("Cannot execute: ") + commandLine.toString();
        return SLANG_FAIL;
    }
    if (Stream* standardInput = process->getStream(StdStreamType::In))
        standardInput->close();

    Stream* standardOutput = process->getStream(StdStreamType::Out);
    while (!process->isTerminated())
    {
        List<Byte> output;
        SLANG_RETURN_ON_FAIL(StreamUtil::readOrDiscard(standardOutput, 0, &output));
        if (output.getCount())
        {
            fwrite(output.getBuffer(), 1, output.getCount(), stdout);
            fflush(stdout);
        }
        else
        {
            Process::sleepCurrentThread(0);
        }
    }
    for (;;)
    {
        List<Byte> output;
        SLANG_RETURN_ON_FAIL(StreamUtil::readOrDiscard(standardOutput, 0, &output));
        if (!output.getCount())
            break;
        fwrite(output.getBuffer(), 1, output.getCount(), stdout);
    }
    fflush(stdout);

    if (process->getReturnValue() != 0 || process->getTerminationSignal() != 0)
    {
        outError = String("Command failed: ") + commandLine.toString();
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static String _getExecutableOutputPath(const String& projectRoot, const Manifest& manifest)
{
    return Path::combine(
        projectRoot,
        getWorkspaceBuildDirectory(manifest),
        manifest.executable.name + Process::getExecutableSuffix());
}

/// Copy the Slang runtime beside a generated host executable so the executable's loader-relative
/// runtime path remains valid outside the compiler installation.
static SlangResult _deployExecutableRuntime(
    const String& slangcPath,
    const String& buildRoot,
    String& outError)
{
    String binDirectory = Path::getParentDirectory(slangcPath);
    String installRoot = Path::getParentDirectory(binDirectory);
    List<String> searchDirectories;
    searchDirectories.add(binDirectory);
    searchDirectories.add(Path::combine(installRoot, "lib"));

    for (const auto& directory : searchDirectories)
    {
        String runtimePath =
            SharedLibrary::calcPlatformPath(Path::combine(directory, "slang-rt").getUnownedSlice());
        if (!File::exists(runtimePath))
            continue;

        String canonicalRuntimePath;
        if (SLANG_FAILED(Path::getCanonical(runtimePath, canonicalRuntimePath)))
        {
            outError = String("Cannot canonicalize the Slang runtime library: ") + runtimePath;
            return SLANG_FAIL;
        }
        List<String> sourcePaths;
        sourcePaths.add(canonicalRuntimePath);
        if (Path::getFileName(runtimePath) != Path::getFileName(canonicalRuntimePath))
            sourcePaths.add(runtimePath);
        for (const auto& sourcePath : sourcePaths)
        {
            List<unsigned char> contents;
            String destinationPath = Path::combine(buildRoot, Path::getFileName(sourcePath));
            if (SLANG_FAILED(File::readAllBytes(sourcePath, contents)) ||
                SLANG_FAILED(File::writeAllBytes(
                    destinationPath,
                    contents.getBuffer(),
                    contents.getCount())))
            {
                outError = String("Cannot copy the Slang runtime library to: ") + destinationPath;
                return SLANG_FAIL;
            }
        }
        return SLANG_OK;
    }

    outError = String("Cannot find the Slang runtime library beside: ") + slangcPath;
    return SLANG_FAIL;
}

/// Compile each workspace primary to a front-end `.slang-module`, preserving its import-relative
/// path below the workspace `build/modules` directory. When requested by the manifest, also compile
/// the `main` primary to a native executable at the build root.
static SlangResult _build(const String& projectRoot, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    List<PrimaryModule> primaryModules;
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(validateProject(
        projectRoot,
        outError,
        &warnings,
        &primaryModules,
        ProjectValidationMode::SourceAndDependencies));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());

    List<String> searchPaths;
    SLANG_RETURN_ON_FAIL(
        _collectCompilationSearchPaths(projectRoot, manifest, searchPaths, outError));
    String slangcPath;
    SLANG_RETURN_ON_FAIL(_findSiblingTool("slangc", slangcPath, outError));

    String mainSourcePath;
    for (const auto& module : primaryModules)
    {
        if (module.importPath == "main")
            mainSourcePath = module.sourcePath;
    }
    if (manifest.executable.name.getLength() && !mainSourcePath.getLength())
    {
        outError =
            "The workspace configures an executable but does not export a primary module with "
            "import path 'main'.";
        return SLANG_FAIL;
    }

    String buildRoot = Path::combine(projectRoot, getWorkspaceBuildDirectory(manifest));
    String modulesRoot = Path::combine(buildRoot, "modules");
    for (const auto& module : primaryModules)
    {
        String outputPath = Path::combine(modulesRoot, module.importPath + ".slang-module");
        if (!Path::createDirectoryRecursive(Path::getParentDirectory(outputPath)))
        {
            outError = String("Cannot create module output directory for: ") + outputPath;
            return SLANG_FAIL;
        }

        List<String> arguments;
        arguments.add(module.sourcePath);
        for (const auto& searchPath : searchPaths)
        {
            arguments.add("-I");
            arguments.add(searchPath);
        }
        arguments.add("-o");
        arguments.add(outputPath);
        SLANG_RETURN_ON_FAIL(_runSiblingTool(slangcPath, arguments, outError));
        if (!File::exists(outputPath))
        {
            outError = String("slangc did not produce the expected module: ") + outputPath;
            return SLANG_FAIL;
        }
    }
    if (manifest.executable.name.getLength())
    {
        String executablePath = _getExecutableOutputPath(projectRoot, manifest);
        if (!Path::createDirectoryRecursive(Path::getParentDirectory(executablePath)))
        {
            outError = String("Cannot create executable output directory for: ") + executablePath;
            return SLANG_FAIL;
        }
        List<String> arguments;
        arguments.add(mainSourcePath);
        for (const auto& searchPath : searchPaths)
        {
            arguments.add("-I");
            arguments.add(searchPath);
        }
        arguments.add("-target");
        arguments.add("exe");
        arguments.add("-o");
        arguments.add(executablePath);
        SLANG_RETURN_ON_FAIL(_runSiblingTool(slangcPath, arguments, outError));
        if (!File::exists(executablePath))
        {
            outError = String("slangc did not produce the expected executable: ") + executablePath;
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(_deployExecutableRuntime(slangcPath, buildRoot, outError));
    }
    fprintf(stdout, "Built %lld module(s).\n", (long long)primaryModules.getCount());
    SLANG_RETURN_ON_FAIL(buildDocumentation(projectRoot, outError));
    return SLANG_OK;
}

/// Run the existing native executable configured by the workspace manifest.
static SlangResult _run(
    const String& projectRoot,
    int argumentCount,
    const char* const* arguments,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    if (!manifest.executable.name.getLength())
    {
        outError = "The workspace does not configure an executable. Add 'executable.name' to "
                   "slang-package.json and run 'slang package build'.";
        return SLANG_FAIL;
    }
    String executablePath = _getExecutableOutputPath(projectRoot, manifest);
    if (!File::exists(executablePath))
    {
        outError = String("The configured executable has not been built: ") + executablePath +
                   ". Run 'slang package build'.";
        return SLANG_FAIL;
    }
    List<String> executableArguments;
    for (Index i = 0; i < argumentCount; ++i)
        executableArguments.add(arguments[i]);
    return _runStreamingSiblingTool(executablePath, executableArguments, outError);
}

/// Validate the materialized workspace and run its `tests` tree through the sibling Slang test
/// infrastructure.
static SlangResult _test(const String& projectRoot, String& outError)
{
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(validateProject(
        projectRoot,
        outError,
        &warnings,
        nullptr,
        ProjectValidationMode::SourceAndDependencies));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());

    String testsPath = Path::combine(projectRoot, "tests");
    SlangPathType testsType;
    if (SLANG_FAILED(Path::getPathType(testsPath, &testsType)) ||
        testsType != SLANG_PATH_TYPE_DIRECTORY)
    {
        outError = String("Workspace tests directory does not exist: ") + testsPath;
        return SLANG_FAIL;
    }

    String slangTestPath;
    SLANG_RETURN_ON_FAIL(_findSiblingTool("slang-test", slangTestPath, outError));
    String relativeTestsPath = Path::getRelativePath(Path::getCurrentPath(), testsPath);
    if (Path::isAbsolute(relativeTestsPath))
    {
        outError = "Workspace tests must be on the same filesystem as the current directory.";
        return SLANG_FAIL;
    }
    List<String> testArguments;
    testArguments.add("-test-dir");
    testArguments.add(relativeTestsPath);
    // A prefix keeps slang-test from selecting its linked unit-test registry when this command is
    // itself exercised by a unit test. The trailing separator still matches every file below the
    // directory, while the directory entry that slang-test adds for an existing prefix is ignored
    // because it has no test-file extension.
    testArguments.add(relativeTestsPath + "/");
    return _runStreamingSiblingTool(slangTestPath, testArguments, outError);
}

/// Print the workspace documentation directory so the user can open `build/docs/index.md`.
static SlangResult _printDocumentationLocation(const String& projectRoot, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    String docsDirectory = Path::combine(projectRoot, getWorkspaceBuildDirectory(manifest), "docs");
    fprintf(stdout, "Open the generated documentation in '%s'.\n", docsDirectory.getBuffer());
    return SLANG_OK;
}

static SlangResult _registerLocalPackage(
    const String& projectRoot,
    const String& name,
    const String& path,
    LocalPackageKind kind,
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
    package.kind = kind;
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(readLocalPackageManifest(projectRoot, package, manifest, outError));
    ioPackages.add(package);
    ioPackages.sort([](const LocalPackage& left, const LocalPackage& right)
                    { return left.name < right.name; });
    return writeProjectLocalPackages(projectRoot, ioPackages, outError);
}

static SlangResult _edit(const String& projectRoot, const String& name, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
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
        outError =
            isPathOnlyLockedPackage(*package)
                ? String("Manifest path dependency is already editable in place: ") + package->path
                : String("Package already uses a local override at: ") + package->path;
        return SLANG_FAIL;
    }
    String destination = Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest), name);
    SlangPathType type;
    if (SLANG_FAILED(Path::getPathType(destination, &type)) || type != SLANG_PATH_TYPE_DIRECTORY)
    {
        outError = String("Dependency checkout is not materialized; run 'slang package fetch': ") +
                   destination;
        return SLANG_FAIL;
    }
    bool isSafe = false;
    SLANG_RETURN_ON_FAIL(isWorkingTreeSafeToRemove(destination, package->commit, isSafe, outError));
    if (!isSafe)
    {
        outError = String("Dependency checkout already has changed files, commits, or stashes: ") +
                   destination;
        return SLANG_FAIL;
    }
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_registerLocalPackage(
        projectRoot,
        name,
        destination,
        LocalPackageKind::Edit,
        localPackages,
        outError));
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    fprintf(stdout, "Package '%s' is now editable.\n", name.getBuffer());
    return SLANG_OK;
}

static SlangResult _unedit(const String& projectRoot, const String& name, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
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
    if (localIndex < 0 || !isEditedLocalPackage(localPackages[localIndex]))
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
    String canonicalExpected;
    if (SLANG_FAILED(Path::getCanonical(
            Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest), name),
            canonicalExpected)) ||
        destination != canonicalExpected)
    {
        outError = String("Editable package is not at its workspace dependency path: ") + name;
        return SLANG_FAIL;
    }
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
            String("Editable checkout has local changes, commits, or stashes; refusing to return "
                   "it to package-tool ownership: ") +
            destination;
        return SLANG_FAIL;
    }
    localPackages.removeAt(localIndex);
    SLANG_RETURN_ON_FAIL(writeProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    fprintf(
        stdout,
        "Package '%s' is no longer editable; its checkout remains at '%s'.\n",
        name.getBuffer(),
        Path::getRelativePath(projectRoot, destination).getBuffer());
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
    if (lockedPackage && isPathOnlyLockedPackage(*lockedPackage))
    {
        outError = String("Manifest path dependency cannot be overridden: ") + name;
        return SLANG_FAIL;
    }

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_registerLocalPackage(
        projectRoot,
        name,
        path,
        LocalPackageKind::Override,
        localPackages,
        outError));
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
    if (isEditedLocalPackage(localPackages[localIndex]))
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
    if (command == "fetch" && (argc == 2 || (argc == 3 && String(argv[2]) == "--clean")))
        return _fetch(projectRoot, argc == 3, outError);
    if (command == "update" && argc == 2)
        return _update(projectRoot, false, false, outError);
    if (command == "update" && argc == 3 && String(argv[2]) == "--clean")
        return _update(projectRoot, false, true, outError);
    if (command == "update" && argc == 3 && String(argv[2]) == "--from-local")
        return _update(projectRoot, true, false, outError);
    if (command == "update" && argc == 4 && String(argv[2]) == "--from-local" &&
        String(argv[3]) == "--clean")
        return _update(projectRoot, true, true, outError);
    if (command == "validate" && argc == 2)
        return _validate(projectRoot, outError);
    if (command == "build" && argc == 2)
        return _build(projectRoot, outError);
    if (command == "run")
        return _run(projectRoot, argc - 2, argv + 2, outError);
    if (command == "test" && argc == 2)
        return _test(projectRoot, outError);
    if (command == "docs" && argc == 2)
        return _printDocumentationLocation(projectRoot, outError);
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
