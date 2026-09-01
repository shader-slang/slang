// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-tool.h"

#include "core/slang-io.h"
#include "core/slang-platform.h"
#include "core/slang-process-util.h"
#include "core/slang-string-util.h"
#include "core/slang-writer.h"
#include "package-bundle.h"
#include "package-docs.h"
#include "package-git.h"
#include "package-json.h"
#include "package-local.h"
#include "package-lock.h"
#include "package-path.h"
#include "package-report.h"
#include "package-resolver.h"
#include "package-validate.h"

#include <stdio.h>

namespace Slang
{
namespace PackageTool
{

static const char* const kManifestName = "slang-package.json";
static const char* const kLockName = "slang-package-lock.json";

static void _printHelp(bool experimental = false)
{
    fprintf(
        stdout,
        "Usage: slang-package [--experimental] <command>\n"
        "\n"
        "Commands:\n"
        "  init             Create a package manifest and standard directories.\n"
        "  fetch [--clean] [--yes] [--skip-validate]\n"
        "                   Materialize dependencies from the lock file.\n"
        "  update [--from-local] [--clean] [--dry-run] [--minimal] [--yes]\n"
        "         [--skip-validate]\n"
        "                   Re-resolve dependencies and update the lock file.\n"
        "                   --from-local uses registered local package manifests.\n"
        "                   --dry-run reports the selected graph without writing the lock.\n"
        "                   --minimal prints one-line package changes without rationale.\n"
        "                   --yes applies without an interactive confirmation.\n"
        "                   --skip-validate skips source, license, and module-layout checks.\n"
        "  build [--skip-validate]\n"
        "                   Build the distributable source bundle and docs.\n"
        "  test             Reserved. Package testing is not implemented yet.\n"
        "  docs             Print the location of generated documentation (build/docs).\n"
        "  status           Check lock, local state, materialized packages, and checkouts.\n"
        "  validate         Validate package structure and the locked dependency closure.\n"
        "  tree             Print the selected dependency graph.\n"
        "  why <name>       Print every graph path that requires a package.\n"
        "  dependency add <name> --git <url> --version <range>\n"
        "  dependency add <name> --git <url> --ref <ref> --as <version>\n"
        "  dependency add <name> --path <path> --as <version>\n"
        "  dependency remove <name> | dependency list\n"
        "                   Manage direct dependencies in slang-package.json.\n"
        "  override add <name> <path> [as]\n"
        "  override enable|disable|remove <name> | override list\n"
        "                   Manage retained local dependency overrides.\n"
        "  edit <name>      Make a dependency checkout editable in place.\n"
        "  unedit <name>    Return an unchanged checkout to tool ownership.\n"
        "  override <name> <path> [as]\n"
        "                   Use a local package as an exact semantic version.\n"
        "  unoverride <name>\n"
        "                   Stop using an existing local package directory.\n");
    if (experimental)
    {
        fprintf(
            stdout,
            "\nExperimental commands and build features:\n"
            "  build            Also generate enabled modules and host executables.\n"
            "  run [name] [args...]  Run an experimental host executable produced by the last "
            "build.\n");
    }
    fprintf(
        stdout,
        "  help             Show this help text.\n"
        "\n"
        "Global options:\n"
        "  --experimental   Enable experimental commands and build features.\n");
}

bool isAffirmativeConfirmationAnswer(const UnownedStringSlice& answer)
{
    // `fgets` keeps the terminating newline, and `UnownedStringSlice::trim` removes only horizontal
    // whitespace, so the line ending has to come off separately. Without that, a bare "y" arrives
    // here as "y\n" and reads as a decline.
    UnownedStringSlice trimmed = StringUtil::trimEndOfLine(answer).trim();
    return trimmed.caseInsensitiveEquals(UnownedStringSlice("y")) ||
           trimmed.caseInsensitiveEquals(UnownedStringSlice("yes"));
}

/// Ask the user to approve an operation after its effects have been printed.
///
/// Non-interactive callers must pass `--yes`; treating end-of-file as approval would let CI or a
/// redirected stdin accidentally apply a graph that nobody reviewed.
static SlangResult _confirmApply(bool assumeYes, const char* prompt, String& outError)
{
    if (assumeYes)
        return SLANG_OK;
    if (!FileWriter::isFileConsole(stdin))
    {
        outError = String(prompt) + " requires confirmation in a terminal. Re-run with --yes.";
        return SLANG_FAIL;
    }

    fprintf(stdout, "%s [y/N] ", prompt);
    fflush(stdout);
    char response[16] = {};
    if (!fgets(response, sizeof(response), stdin))
    {
        outError = "Confirmation was not received; no changes were applied.";
        return SLANG_FAIL;
    }
    if (isAffirmativeConfirmationAnswer(UnownedStringSlice(response)))
        return SLANG_OK;
    outError = "Operation cancelled; no changes were applied.";
    return SLANG_FAIL;
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
        Index localIndex = findActiveLocalPackageIndex(localPackages, package.name);
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
        Index localIndex = findActiveLocalPackageIndex(localPackages, package.name);
        if (localIndex >= 0)
        {
            fprintf(
                stdout,
                "Using local %s '%s' at %s.\n",
                isEditedLocalPackage(localPackages[localIndex]) ? "edit" : "override",
                package.name.getBuffer(),
                localPackages[localIndex].path.getBuffer());
            continue;
        }
        if (isPathOnlyLockedPackage(package))
        {
            fprintf(
                stdout,
                "Using path package '%s' at %s.\n",
                package.name.getBuffer(),
                package.path.getBuffer());
            continue;
        }
        if (isLocalOverrideLockedPackage(package))
        {
            outError = String("Locked local override '") + package.name +
                       "' is not registered in slang-workspace.json.";
            return SLANG_FAIL;
        }

        SemanticVersion releaseVersion;
        if (SLANG_SUCCEEDED(parseReleaseTag(package.ref, releaseVersion)))
        {
            TagCandidate candidate;
            SLANG_RETURN_ON_FAIL(resolveReference(package.git, package.ref, candidate, outError));
            if (candidate.commit != package.commit)
            {
                outError =
                    String(
                        "Locked release tag no longer identifies the locked commit for package '") +
                    package.name + "'.";
                return SLANG_FAIL;
            }
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
        // A local-override lock intentionally carries no Git commit. When the retained published
        // checkout already equals the newly selected commit, it is still safe to reclaim without
        // --clean. A different HEAD remains unowned and materialization refuses to replace it.
        if (!currentCommit.getLength() && previousLock)
        {
            Index previousIndex = findLockedPackageIndex(*previousLock, package.name);
            if (previousIndex >= 0)
            {
                const auto& previousPackage = previousLock->packages[previousIndex];
                if (isLocalOverrideLockedPackage(previousPackage) &&
                    previousPackage.git == package.git)
                {
                    String origin;
                    String headCommit;
                    if (SLANG_SUCCEEDED(getRepositoryOrigin(destination, origin, outError)) &&
                        origin == package.git &&
                        SLANG_SUCCEEDED(
                            getRepositoryHeadCommit(destination, headCommit, outError)) &&
                        headCommit == package.commit)
                    {
                        currentCommit = headCommit;
                    }
                    outError = String();
                }
            }
        }
        fprintf(
            stdout,
            "Checking out '%s' at %s (%s).\n",
            package.name.getBuffer(),
            package.ref.getBuffer(),
            package.commit.getBuffer());
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

/// Collect existing tool-owned paths that `--clean` authorizes materialization to replace.
///
/// This is a confirmation preflight, not the source of truth for deletion. Materialization repeats
/// every ownership and dirty-tree check immediately before it changes a path, so a checkout that
/// changes after this inventory still fails safely.
static SlangResult _collectCheckoutsRequiringClean(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const LockFile* previousLock,
    const List<LocalPackage>& localPackages,
    List<String>& outPackageNames,
    String& outError)
{
    outPackageNames.clear();
    String depsRoot = Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest));
    for (const auto& package : lock.packages)
    {
        if (findActiveLocalPackageIndex(localPackages, package.name) >= 0 ||
            isPathOnlyLockedPackage(package))
        {
            continue;
        }

        String destination = Path::combine(depsRoot, package.name);
        SlangPathType pathType;
        if (SLANG_FAILED(Path::getPathType(destination, &pathType)))
            continue;
        if (pathType != SLANG_PATH_TYPE_DIRECTORY)
        {
            outPackageNames.add(package.name);
            continue;
        }

        String expectedCommit;
        if (previousLock)
        {
            Index previousIndex = findLockedPackageIndex(*previousLock, package.name);
            if (previousIndex >= 0)
            {
                const auto& previousPackage = previousLock->packages[previousIndex];
                if (previousPackage.git == package.git && !previousPackage.path.getLength())
                    expectedCommit = previousPackage.commit;
            }
        }
        if (!expectedCommit.getLength())
        {
            String origin;
            String headCommit;
            if (previousLock &&
                SLANG_SUCCEEDED(getRepositoryOrigin(destination, origin, outError)) &&
                origin == package.git &&
                SLANG_SUCCEEDED(getRepositoryHeadCommit(destination, headCommit, outError)) &&
                headCommit == package.commit)
            {
                outError = String();
                continue;
            }
            outError = String();
            outPackageNames.add(package.name);
            continue;
        }

        String origin;
        SLANG_RETURN_ON_FAIL(getRepositoryOrigin(destination, origin, outError));
        if (origin != package.git)
        {
            outPackageNames.add(package.name);
            continue;
        }
        bool isSafe = false;
        SLANG_RETURN_ON_FAIL(
            isWorkingTreeSafeToRemove(destination, expectedCommit, isSafe, outError));
        if (!isSafe)
            outPackageNames.add(package.name);
    }
    return SLANG_OK;
}

static void _printCleanReplacementWarning(const List<String>& packageNames)
{
    if (!packageNames.getCount())
        return;
    fprintf(stdout, "--clean will discard local state from:\n");
    for (const auto& packageName : packageNames)
        fprintf(stdout, "  %s\n", packageName.getBuffer());
}

static void _appendIncompleteMaterializationAdvice(String& ioError, bool previousLockExists)
{
    appendErrorAdvice(
        ioError,
        previousLockExists
            ? "The previous lock remains authoritative, but deps/ may be partially changed and "
              "build/search-paths may be empty. Run 'slang package fetch' to restore it; add "
              "'--clean' only if replacement is intended."
            : "No lock was written, but deps/ may be partial and build/search-paths may be empty. "
              "Fix the reported error and run 'slang package fetch' again.");
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
static SlangResult _validateLockExclusions(
    const Manifest& manifest,
    const LockFile& lock,
    String& outError)
{
    for (const auto& exclusion : manifest.workspace.exclusions)
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

static SlangResult _validateLockAgainstManifest(
    const Manifest& manifest,
    const LockFile& lock,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(_validateLockExclusions(manifest, lock, outError));
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
        if (!isActiveLocalPackage(localPackage))
            continue;
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
                       "'slang package update'.";
            return SLANG_FAIL;
        }
        if (!isEditedLocalPackage(localPackage) && localPackage.as.getLength() &&
            package->version != localPackage.as)
        {
            outError = String("Locked version for local override '") + package->name +
                       "' does not match slang-workspace.json. Run "
                       "'slang package update'.";
            return SLANG_FAIL;
        }
        Manifest manifest;
        SLANG_RETURN_ON_FAIL(
            readLocalPackageManifest(projectRoot, localPackage, manifest, outError));
        if (SLANG_FAILED(validateLockedPackageManifest(*package, manifest, outError)))
        {
            appendErrorAdvice(
                outError,
                isEditedLocalPackage(localPackage)
                    ? "An edit retains its published Git pin; publish a new release tag and "
                      "run 'slang package update', or use an override for local manifest "
                      "changes."
                    : "Align the local manifest with the selected upstream graph, or run "
                      "'slang package update' to record local manifest changes.");
            return SLANG_FAIL;
        }
    }
    for (const auto& package : lock.packages)
    {
        if (isLocalOverrideLockedPackage(package) &&
            findActiveLocalPackageIndex(localPackages, package.name) < 0)
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
    bool allowLocalManifestChanges = false,
    List<String>* outWarnings = nullptr)
{
    List<bool> trusted;
    trusted.setCount(lock.packages.getCount());
    for (Index i = 0; i < lock.packages.getCount(); ++i)
        trusted[i] = false;
    List<Index> pending;
    List<ToolchainConstraint> toolchainConstraints;
    addSlangToolchainConstraint(rootManifest, toolchainConstraints);

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
        Index localIndex = findActiveLocalPackageIndex(localPackages, package.name);

        Manifest manifest;
        if (SLANG_FAILED(
                readManifest(Path::combine(packageRoot, kManifestName), manifest, outError)))
        {
            outError = String("Cannot read the dependency manifest of locked package '") +
                       package.name + "'. " + outError;
            return SLANG_FAIL;
        }
        if (!(allowLocalManifestChanges && localIndex >= 0))
            SLANG_RETURN_ON_FAIL(validateLockedPackageManifest(package, manifest, outError));
        addSlangToolchainConstraint(manifest, toolchainConstraints);
        addUnadoptedWorkspaceExclusionWarnings(rootManifest, package.name, manifest, outWarnings);
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
    SLANG_RETURN_ON_FAIL(requireAllLockPackagesTrusted(lock, trusted, outError));
    return selectSlangToolchain(toolchainConstraints, outError);
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
    SemanticVersion installedToolchain;
    String installedToolchainText;
    String toolchainError;
    if (SLANG_SUCCEEDED(getInstalledSlangToolchainVersion(
            installedToolchain,
            installedToolchainText,
            toolchainError)))
    {
        manifest.slangToolchainConstraint = String(">=") + installedToolchainText;
    }
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

static Index _findDependencyIndex(const Manifest& manifest, const String& name)
{
    for (Index i = 0; i < manifest.dependencies.getCount(); ++i)
        if (manifest.dependencies[i].name == name)
            return i;
    return -1;
}

/// Validate generated manifest JSON before replacing the workspace manifest.
static SlangResult _writeValidatedProjectManifest(
    const String& projectRoot,
    const Manifest& manifest,
    String& outError)
{
    String temporaryPath = Path::combine(projectRoot, ".slang-package.json.validate.tmp");
    if (SLANG_FAILED(writeManifest(temporaryPath, manifest, outError)))
        return SLANG_FAIL;
    Manifest validatedManifest;
    SlangResult result = readManifest(temporaryPath, validatedManifest, outError);
    File::remove(temporaryPath);
    if (SLANG_FAILED(result))
        return result;
    return writeManifest(Path::combine(projectRoot, kManifestName), manifest, outError);
}

static SlangResult _dependencyAdd(
    const String& projectRoot,
    const Dependency& dependency,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    Index existingIndex = _findDependencyIndex(manifest, dependency.name);
    bool replacing = existingIndex >= 0;
    if (replacing)
        manifest.dependencies[existingIndex] = dependency;
    else
        manifest.dependencies.add(dependency);
    manifest.dependencies.sort([](const Dependency& left, const Dependency& right)
                               { return left.name < right.name; });
    SLANG_RETURN_ON_FAIL(_writeValidatedProjectManifest(projectRoot, manifest, outError));
    fprintf(
        stdout,
        "%s dependency '%s' in slang-package.json. Run 'slang package status', then "
        "'slang package update'.\n",
        replacing ? "Updated" : "Added",
        dependency.name.getBuffer());
    return SLANG_OK;
}

static SlangResult _dependencyRemove(
    const String& projectRoot,
    const String& name,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    Index index = _findDependencyIndex(manifest, name);
    if (index < 0)
    {
        outError = String("Manifest does not declare dependency: ") + name;
        return SLANG_FAIL;
    }
    manifest.dependencies.removeAt(index);
    SLANG_RETURN_ON_FAIL(_writeValidatedProjectManifest(projectRoot, manifest, outError));
    fprintf(
        stdout,
        "Removed dependency '%s' from slang-package.json. Run 'slang package status', then "
        "'slang package update'.\n",
        name.getBuffer());
    return SLANG_OK;
}

static SlangResult _dependencyList(const String& projectRoot, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    if (!manifest.dependencies.getCount())
    {
        fprintf(stdout, "Direct dependencies: none.\n");
        return SLANG_OK;
    }
    fprintf(stdout, "Direct dependencies:\n");
    for (const auto& dependency : manifest.dependencies)
    {
        if (dependency.path.getLength())
        {
            fprintf(
                stdout,
                "  %s: path %s as %s\n",
                dependency.name.getBuffer(),
                dependency.path.getBuffer(),
                dependency.as.getBuffer());
        }
        else if (dependency.version.getLength())
        {
            fprintf(
                stdout,
                "  %s: %s version %s\n",
                dependency.name.getBuffer(),
                dependency.git.getBuffer(),
                dependency.version.getBuffer());
        }
        else
        {
            fprintf(
                stdout,
                "  %s: %s ref %s as %s\n",
                dependency.name.getBuffer(),
                dependency.git.getBuffer(),
                dependency.ref.getBuffer(),
                dependency.as.getBuffer());
        }
    }
    return SLANG_OK;
}

static void _warnSkippedSourceValidation()
{
    fprintf(
        stderr,
        "slang-package: warning: skipped source, license, and module-layout validation "
        "(--skip-validate).\n");
}

static SlangResult _update(
    const String& projectRoot,
    bool fromLocal,
    bool allowClean,
    bool dryRun,
    bool minimal,
    bool assumeYes,
    bool skipValidate,
    String& outError);

static SlangResult _fetch(
    const String& projectRoot,
    bool allowClean,
    bool assumeYes,
    bool skipValidate,
    String& outError)
{
    SLANG_UNUSED(assumeYes);
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    String lockPath = Path::combine(projectRoot, kLockName);
    if (!File::exists(lockPath))
    {
        if (!manifest.dependencies.getCount())
        {
            outError = "fetch requires slang-package-lock.json when there is no dependency graph "
                       "to resolve. Run 'slang package update' to create an empty lock.";
            return SLANG_FAIL;
        }
        fprintf(stdout, "No lock file exists; resolving the initial dependency graph.\n");
        return _update(
            projectRoot,
            false,
            allowClean,
            false,
            false,
            assumeYes,
            skipValidate,
            outError);
    }

    LockFile lock;
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_validateLockAgainstManifest(manifest, lock, outError));
    SLANG_RETURN_ON_FAIL(_validateLocalPackages(projectRoot, lock, localPackages, outError));
    if (!skipValidate)
        SLANG_RETURN_ON_FAIL(validatePackageTree(projectRoot, manifest, outError));
    List<String> cleanReplacements;
    if (allowClean)
    {
        SLANG_RETURN_ON_FAIL(_collectCheckoutsRequiringClean(
            projectRoot,
            manifest,
            lock,
            &lock,
            localPackages,
            cleanReplacements,
            outError));
        _printCleanReplacementWarning(cleanReplacements);
        if (cleanReplacements.getCount())
        {
            SLANG_RETURN_ON_FAIL(
                _confirmApply(assumeYes, "Discard this local checkout state and fetch?", outError));
        }
    }
    SLANG_RETURN_ON_FAIL(_clearSearchPaths(projectRoot, manifest, outError));
    if (SLANG_FAILED(
            _materialize(projectRoot, manifest, lock, &lock, localPackages, allowClean, outError)))
    {
        _appendIncompleteMaterializationAdvice(outError, true);
        return SLANG_FAIL;
    }
    List<String> warnings;
    if (skipValidate)
    {
        if (SLANG_FAILED(_validateMaterializedManifests(
                projectRoot,
                manifest,
                lock,
                localPackages,
                outError,
                false,
                &warnings)))
        {
            _appendIncompleteMaterializationAdvice(outError, true);
            return SLANG_FAIL;
        }
        _warnSkippedSourceValidation();
    }
    else
    {
        if (SLANG_FAILED(validateResolvedProject(
                projectRoot,
                manifest,
                lock,
                localPackages,
                outError,
                &warnings)))
        {
            _appendIncompleteMaterializationAdvice(outError, true);
            return SLANG_FAIL;
        }
    }
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    SLANG_RETURN_ON_FAIL(_writeSearchPaths(projectRoot, manifest, lock, localPackages, outError));
    fprintf(stdout, "Fetched %lld package(s).\n", (long long)lock.packages.getCount());
    return SLANG_OK;
}

static SlangResult _update(
    const String& projectRoot,
    bool fromLocal,
    bool allowClean,
    bool dryRun,
    bool minimal,
    bool assumeYes,
    bool skipValidate,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    bool useLocalResolver = fromLocal;
    for (const auto& localPackage : localPackages)
        useLocalResolver =
            useLocalResolver || (!isEditedLocalPackage(localPackage) && localPackage.enabled);
    if (fromLocal && localPackages.getCount() == 0)
    {
        outError = "update --from-local requires a registered local package.";
        return SLANG_FAIL;
    }
    if (!skipValidate)
        SLANG_RETURN_ON_FAIL(validatePackageTree(projectRoot, manifest, outError));

    LockFile previousLock;
    LockFile* previousLockPtr = nullptr;
    String lockPath = Path::combine(projectRoot, kLockName);
    if (File::exists(lockPath))
    {
        SLANG_RETURN_ON_FAIL(readLockFile(lockPath, previousLock, outError));
        previousLockPtr = &previousLock;
    }
    if (useLocalResolver)
    {
        for (auto& localPackage : localPackages)
        {
            if (!isActiveLocalPackage(localPackage) || isEditedLocalPackage(localPackage) ||
                localPackage.as.getLength())
                continue;
            Index lockedIndex =
                previousLockPtr ? findLockedPackageIndex(*previousLockPtr, localPackage.name) : -1;
            if (lockedIndex < 0)
            {
                outError = String("Override for package '") + localPackage.name +
                           "' requires an 'as' version because no previous lock version exists.";
                return SLANG_FAIL;
            }
            localPackage.as = previousLock.packages[lockedIndex].version;
        }
    }

    LockFile lock;
    List<String> warnings;
    ResolveReport report;
    if (useLocalResolver)
    {
        SLANG_RETURN_ON_FAIL(resolveDependenciesFromLocalPackages(
            projectRoot,
            manifest,
            localPackages,
            lock,
            outError,
            &warnings,
            &report));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(
            resolveDependencies(projectRoot, manifest, lock, outError, &warnings, &report));
    }
    SLANG_RETURN_ON_FAIL(_validateLocalPackages(projectRoot, lock, localPackages, outError));
    String reportText =
        formatResolveReport(manifest, previousLockPtr, lock, report, dryRun, minimal);
    if (fromLocal)
    {
        fprintf(
            stderr,
            "slang-package: warning: --from-local is deprecated; enabled overrides now "
            "participate in plain update.\n");
    }
    if (dryRun)
    {
        for (const auto& warning : warnings)
            fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
        if (skipValidate)
            _warnSkippedSourceValidation();
        fprintf(stdout, "%s", reportText.getBuffer());
        fprintf(stdout, "Dry run: lock and dependency checkouts were not modified.\n");
        return SLANG_OK;
    }
    List<String> cleanReplacements;
    if (allowClean)
    {
        SLANG_RETURN_ON_FAIL(_collectCheckoutsRequiringClean(
            projectRoot,
            manifest,
            lock,
            previousLockPtr,
            localPackages,
            cleanReplacements,
            outError));
        _printCleanReplacementWarning(cleanReplacements);
    }
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    warnings.clear();
    if (skipValidate)
        _warnSkippedSourceValidation();
    fprintf(stdout, "%s", reportText.getBuffer());
    SLANG_RETURN_ON_FAIL(_confirmApply(assumeYes, "Apply this update?", outError));
    SLANG_RETURN_ON_FAIL(_clearSearchPaths(projectRoot, manifest, outError));
    if (SLANG_FAILED(_materialize(
            projectRoot,
            manifest,
            lock,
            previousLockPtr,
            localPackages,
            allowClean,
            outError)))
    {
        _appendIncompleteMaterializationAdvice(outError, previousLockPtr != nullptr);
        return SLANG_FAIL;
    }
    if (skipValidate)
    {
        if (SLANG_FAILED(_validateMaterializedManifests(
                projectRoot,
                manifest,
                lock,
                localPackages,
                outError,
                false,
                &warnings)))
        {
            _appendIncompleteMaterializationAdvice(outError, previousLockPtr != nullptr);
            return SLANG_FAIL;
        }
    }
    else
    {
        if (SLANG_FAILED(validateResolvedProject(
                projectRoot,
                manifest,
                lock,
                localPackages,
                outError,
                &warnings)))
        {
            _appendIncompleteMaterializationAdvice(outError, previousLockPtr != nullptr);
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(writeLockFile(lockPath, lock, outError));
    SLANG_RETURN_ON_FAIL(_writeSearchPaths(projectRoot, manifest, lock, localPackages, outError));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    if (useLocalResolver)
    {
        fprintf(
            stdout,
            "The workspace contains local package state and requires slang-workspace.json.\n");
    }
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

/// Report whether committed resolution, local package state, and materialized checkouts agree.
static SlangResult _status(const String& projectRoot, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    String lockPath = Path::combine(projectRoot, kLockName);
    if (!File::exists(lockPath))
    {
        if (manifest.dependencies.getCount())
        {
            outError = "Workspace has dependencies but no slang-package-lock.json. Run "
                       "'slang package fetch' to select the initial graph.";
            return SLANG_FAIL;
        }
        List<LocalPackage> localPackages;
        SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
        if (localPackages.getCount())
        {
            outError =
                "Workspace has local package registrations but no dependency lock to attach them "
                "to.";
            return SLANG_FAIL;
        }
        fprintf(
            stdout,
            "Package '%s': no dependency lock is required; workspace is clean and portable.\n",
            manifest.name.getBuffer());
        return SLANG_OK;
    }
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    StringBuilder issues;
    Index issueCount = 0;
    auto addIssue = [&](const String& issue)
    {
        ++issueCount;
        issues << "  - " << issue << "\n";
    };

    String issue;
    if (SLANG_FAILED(_validateLockAgainstManifest(manifest, lock, issue)))
        addIssue(issue);
    issue = String();
    if (SLANG_FAILED(_validateLocalPackages(projectRoot, lock, localPackages, issue)))
        addIssue(issue);

    // Find the tool-owned checkouts that are absent before inspecting anything inside them.
    // Reading a dependency's own `slang-package.json` and asking Git about its checkout both fail
    // for an absent directory, and those failures would only restate the absence -- one as a
    // missing JSON file, the other as Git refusing to run in a directory that does not exist.
    List<String> toolOwnedNames;
    List<String> unmaterializedNames;
    for (const auto& package : lock.packages)
    {
        if (findActiveLocalPackageIndex(localPackages, package.name) >= 0 ||
            package.path.getLength())
            continue;
        toolOwnedNames.add(package.name);
        String packageRoot =
            Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest), package.name);
        SlangPathType pathType;
        if (SLANG_FAILED(Path::getPathType(packageRoot, &pathType)) ||
            pathType != SLANG_PATH_TYPE_DIRECTORY)
            unmaterializedNames.add(package.name);
    }

    Index cleanCheckoutCount = 0;
    if (unmaterializedNames.getCount())
    {
        StringBuilder detail;
        detail << unmaterializedNames.getCount()
               << " locked package(s) are not materialized under '"
               << getWorkspaceDepsDirectory(manifest) << "/': ";
        for (Index i = 0; i < unmaterializedNames.getCount(); ++i)
            detail << (i ? ", " : "") << unmaterializedNames[i];
        detail << ". Run 'slang package fetch' to materialize them.";
        addIssue(detail);
    }
    else
    {
        // Validating the manifest closure requires walking every reachable dependency manifest, so
        // it can only run once all of them are present.
        List<String> warnings;
        issue = String();
        if (SLANG_FAILED(_validateMaterializedManifests(
                projectRoot,
                manifest,
                lock,
                localPackages,
                issue,
                false,
                &warnings)))
        {
            appendErrorAdvice(
                issue,
                "Run 'slang package update' if a path or override package manifest changed.");
            addIssue(issue);
        }
        for (const auto& warning : warnings)
            fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    }

    // Inspect each checkout that is present, even when a sibling is absent. A present checkout can
    // still carry the wrong origin or uncommitted work, which is separate information rather than a
    // restatement of the absence reported above.
    for (const auto& package : lock.packages)
    {
        if (findActiveLocalPackageIndex(localPackages, package.name) >= 0 ||
            package.path.getLength() || unmaterializedNames.indexOf(package.name) >= 0)
            continue;
        String packageRoot =
            Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest), package.name);
        String origin;
        issue = String();
        if (SLANG_FAILED(getRepositoryOrigin(packageRoot, origin, issue)))
        {
            addIssue(
                String("Package checkout '") + package.name +
                "' is not a Git repository with an 'origin' remote. Run 'slang package fetch "
                "--clean' to replace it.");
            continue;
        }
        if (origin != package.git)
        {
            addIssue(
                String("Package checkout '") + package.name +
                "' has a different Git origin. Run 'slang package fetch --clean' to restore it.");
            continue;
        }

        GitWorkingTreeStatus gitStatus;
        issue = String();
        if (SLANG_FAILED(getWorkingTreeStatus(packageRoot, package.commit, gitStatus, issue)))
        {
            addIssue(issue);
            continue;
        }
        if (gitStatus.changedFileCount || gitStatus.commitsAhead || gitStatus.commitsBehind ||
            gitStatus.stashCount || gitStatus.headCommit != package.commit)
        {
            StringBuilder detail;
            detail << "Package checkout '" << package.name << "' is not clean ("
                   << gitStatus.changedFileCount << " changed/untracked file(s), "
                   << gitStatus.commitsAhead << " commit(s) ahead, " << gitStatus.commitsBehind
                   << " commit(s) behind, " << gitStatus.stashCount
                   << " stash(es)). Run 'slang package edit " << package.name
                   << "' to keep the work, or 'slang package fetch --clean' to discard it.";
            addIssue(detail);
            continue;
        }
        ++cleanCheckoutCount;
    }

    fprintf(
        stdout,
        "Package '%s': lock is current with %lld package(s).\n",
        manifest.name.getBuffer(),
        (long long)lock.packages.getCount());
    if (localPackages.getCount() == 0)
    {
        fprintf(stdout, "Local package state: none.\n");
    }
    else
    {
        fprintf(stdout, "Local package state:\n");
        for (const auto& package : localPackages)
        {
            if (isEditedLocalPackage(package))
            {
                Index lockedIndex = findLockedPackageIndex(lock, package.name);
                GitWorkingTreeStatus gitStatus;
                String gitError;
                bool haveGitStatus =
                    lockedIndex >= 0 && SLANG_SUCCEEDED(getWorkingTreeStatus(
                                            Path::combine(projectRoot, package.path),
                                            lock.packages[lockedIndex].commit,
                                            gitStatus,
                                            gitError));
                if (haveGitStatus)
                {
                    fprintf(
                        stdout,
                        "  %s: edit at %s (%lld changed/untracked file(s), %lld commit(s) ahead, "
                        "%lld commit(s) behind, %lld stash(es))\n",
                        package.name.getBuffer(),
                        package.path.getBuffer(),
                        (long long)gitStatus.changedFileCount,
                        (long long)gitStatus.commitsAhead,
                        (long long)gitStatus.commitsBehind,
                        (long long)gitStatus.stashCount);
                }
                else
                {
                    fprintf(
                        stdout,
                        "  %s: edit at %s (Git state unavailable)\n",
                        package.name.getBuffer(),
                        package.path.getBuffer());
                }
                addIssue(
                    String("Package '") + package.name +
                    "' is in edit mode; the workspace is not portable.");
            }
            else
            {
                String effectiveVersion = package.as;
                if (!effectiveVersion.getLength())
                {
                    Index lockedIndex = findLockedPackageIndex(lock, package.name);
                    if (lockedIndex >= 0)
                        effectiveVersion = lock.packages[lockedIndex].version;
                }
                fprintf(
                    stdout,
                    "  %s: override %s at %s as %s\n",
                    package.name.getBuffer(),
                    package.enabled ? "enabled" : "disabled",
                    package.path.getBuffer(),
                    effectiveVersion.getBuffer());
                if (package.enabled)
                {
                    addIssue(
                        String("Package '") + package.name +
                        "' has an enabled override; the workspace is not portable.");
                }
            }
        }
    }
    if (toolOwnedNames.getCount())
    {
        fprintf(
            stdout,
            "%lld of %lld tool-owned Git checkout(s) are clean.\n",
            (long long)cleanCheckoutCount,
            (long long)toolOwnedNames.getCount());
    }
    if (issueCount)
    {
        outError = String("Workspace is not clean or portable:\n") + issues;
        return SLANG_FAIL;
    }
    fprintf(stdout, "Workspace is clean, portable, and consistent.\n");
    return SLANG_OK;
}

static String _describeDependencyRequirement(const Dependency& dependency)
{
    if (dependency.path.getLength())
        return String("path ") + dependency.path + " as " + dependency.as;
    if (dependency.version.getLength())
        return String("version ") + dependency.version;
    return String("ref ") + dependency.ref + " as " + dependency.as;
}

static void _getSortedDependencies(
    const List<Dependency>& dependencies,
    List<const Dependency*>& outDependencies)
{
    outDependencies.clear();
    for (const auto& dependency : dependencies)
        outDependencies.add(&dependency);
    outDependencies.sort([](const Dependency* left, const Dependency* right)
                         { return left->name < right->name; });
}

static void _printDependencyTree(
    const LockFile& lock,
    const Dependency& dependency,
    const String& prefix,
    List<String>& ioExpanded)
{
    Index packageIndex = findLockedPackageIndex(lock, dependency.name);
    if (packageIndex < 0)
    {
        fprintf(
            stdout,
            "%s%s (missing from lock)\n",
            prefix.getBuffer(),
            dependency.name.getBuffer());
        return;
    }
    const auto& package = lock.packages[packageIndex];
    bool repeated = ioExpanded.contains(package.name);
    fprintf(
        stdout,
        "%s%s@%s [%s]%s\n",
        prefix.getBuffer(),
        package.name.getBuffer(),
        package.version.getBuffer(),
        _describeDependencyRequirement(dependency).getBuffer(),
        repeated ? " (*)" : "");
    if (repeated)
        return;
    ioExpanded.add(package.name);

    List<const Dependency*> children;
    _getSortedDependencies(package.dependencies, children);
    for (const auto child : children)
        _printDependencyTree(lock, *child, prefix + "  ", ioExpanded);
}

static SlangResult _tree(const String& projectRoot, String& outError)
{
    Manifest manifest;
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    SLANG_RETURN_ON_FAIL(_validateLockAgainstManifest(manifest, lock, outError));

    fprintf(stdout, "%s\n", manifest.name.getBuffer());
    List<const Dependency*> dependencies;
    _getSortedDependencies(manifest.dependencies, dependencies);
    List<String> expanded;
    for (const auto dependency : dependencies)
        _printDependencyTree(lock, *dependency, "  ", expanded);
    if (!dependencies.getCount())
        fprintf(stdout, "  (no dependencies)\n");
    fprintf(stdout, "(*) dependency subtree already shown\n");
    return SLANG_OK;
}

static void _printWhyPaths(
    const LockFile& lock,
    const List<Dependency>& dependencies,
    const String& targetName,
    const String& path,
    List<String>& ioStack,
    Index& ioPathCount)
{
    List<const Dependency*> sortedDependencies;
    _getSortedDependencies(dependencies, sortedDependencies);
    for (const auto dependency : sortedDependencies)
    {
        Index packageIndex = findLockedPackageIndex(lock, dependency->name);
        if (packageIndex < 0 || ioStack.contains(dependency->name))
            continue;
        const auto& package = lock.packages[packageIndex];
        String nextPath = path + " -> " + package.name + "@" + package.version + " [" +
                          _describeDependencyRequirement(*dependency) + "]";
        if (package.name == targetName)
        {
            fprintf(stdout, "%s\n", nextPath.getBuffer());
            ++ioPathCount;
            continue;
        }
        ioStack.add(package.name);
        _printWhyPaths(lock, package.dependencies, targetName, nextPath, ioStack, ioPathCount);
        ioStack.removeLast();
    }
}

static SlangResult _why(const String& projectRoot, const String& name, String& outError)
{
    Manifest manifest;
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    SLANG_RETURN_ON_FAIL(_validateLockAgainstManifest(manifest, lock, outError));
    if (findLockedPackageIndex(lock, name) < 0)
    {
        outError = String("Package is not present in the lock: ") + name;
        return SLANG_FAIL;
    }

    fprintf(stdout, "Dependency paths to '%s':\n", name.getBuffer());
    List<String> stack;
    Index pathCount = 0;
    _printWhyPaths(lock, manifest.dependencies, name, manifest.name, stack, pathCount);
    if (!pathCount)
    {
        outError = String("No dependency path from the workspace reaches package: ") + name;
        return SLANG_FAIL;
    }
    fprintf(
        stdout,
        "%lld path(s). This explains graph presence, not candidates rejected by resolution.\n",
        (long long)pathCount);
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

static String _getExecutableOutputPath(
    const String& projectRoot,
    const Manifest& manifest,
    const String& executableName)
{
    return Path::combine(
        Path::combine(projectRoot, getWorkspaceBuildDirectory(manifest)),
        "host",
        executableName + Process::getExecutableSuffix());
}

/// Locate the workspace primary whose source filename stem matches a host executable name.
static SlangResult _findHostExecutableSource(
    const Manifest& manifest,
    const List<PrimaryModule>& primaryModules,
    const String& executableName,
    String& outSourcePath,
    String& outError)
{
    const PrimaryModule* match = nullptr;
    for (const auto& module : primaryModules)
    {
        if (module.packageName != manifest.name)
            continue;
        if (Path::getFileNameWithoutExt(module.sourcePath) != executableName)
            continue;
        if (match)
        {
            outError = String("Host executable '") + executableName +
                       "' matches more than one workspace primary.";
            return SLANG_FAIL;
        }
        match = &module;
    }
    if (!match)
    {
        outError = String("The workspace configures host executable '") + executableName +
                   "' but does not export a primary whose filename is '" + executableName +
                   ".slang'.";
        return SLANG_FAIL;
    }
    outSourcePath = match->sourcePath;
    return SLANG_OK;
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

/// Copy exported source under `build/bundle/source`. With the experimental opt-in, also compile
/// enabled `.slang-module` output and host executables into explicitly marked directories.
static SlangResult _build(
    const String& projectRoot,
    bool experimental,
    bool skipValidate,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    List<PrimaryModule> primaryModules;
    List<ExportedSourceFile> sourceFiles;
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(validateProject(
        projectRoot,
        outError,
        &warnings,
        &primaryModules,
        &sourceFiles,
        skipValidate));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    if (skipValidate)
        _warnSkippedSourceValidation();

    const bool buildModules = experimental && manifest.workspace.bundle.modules;
    const bool buildHost = experimental && hasHostExecutables(manifest);

    List<String> executableSources;
    if (buildHost)
    {
        for (const auto& executableName : manifest.host.executables)
        {
            String sourcePath;
            SLANG_RETURN_ON_FAIL(_findHostExecutableSource(
                manifest,
                primaryModules,
                executableName,
                sourcePath,
                outError));
            executableSources.add(sourcePath);
        }
    }

    String buildRoot = Path::combine(projectRoot, getWorkspaceBuildDirectory(manifest));
    String bundleRoot = Path::combine(buildRoot, "bundle");
    String modulesRoot = Path::combine(bundleRoot, "modules");
    String sourceRoot = Path::combine(bundleRoot, "source");
    String hostRoot = Path::combine(buildRoot, "host");
    if (manifest.workspace.bundle.source)
    {
        SLANG_RETURN_ON_FAIL(copyBundleSource(sourceRoot, sourceFiles, outError));
        fprintf(stdout, "Copied %lld source file(s).\n", (long long)sourceFiles.getCount());
    }
    else
    {
        Path::removeNonEmpty(sourceRoot);
    }
    String slangcPath;
    List<String> searchPaths;
    if (buildModules || buildHost)
    {
        SLANG_RETURN_ON_FAIL(
            _collectCompilationSearchPaths(projectRoot, manifest, searchPaths, outError));
        SLANG_RETURN_ON_FAIL(_findSiblingTool("slangc", slangcPath, outError));
    }
    if (buildModules)
    {
        fprintf(
            stderr,
            "slang-package: warning: Generating experimental .slang-module files; their binary "
            "format is not stable. See %s.\n",
            Path::combine(modulesRoot, "provenance.json").getBuffer());
        SLANG_RETURN_ON_FAIL(resetDirectory(modulesRoot, outError));
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
        SLANG_RETURN_ON_FAIL(writeModuleProvenance(modulesRoot, slangcPath, outError));
        fprintf(stdout, "Built %lld module(s).\n", (long long)primaryModules.getCount());
    }
    else
    {
        Path::removeNonEmpty(modulesRoot);
    }
    if (buildHost)
    {
        SLANG_RETURN_ON_FAIL(resetDirectory(hostRoot, outError));
        SLANG_RETURN_ON_FAIL(writeExperimentalHostMarker(hostRoot, outError));
        for (Index i = 0; i < manifest.host.executables.getCount(); ++i)
        {
            String executablePath =
                _getExecutableOutputPath(projectRoot, manifest, manifest.host.executables[i]);
            if (!Path::createDirectoryRecursive(Path::getParentDirectory(executablePath)))
            {
                outError =
                    String("Cannot create executable output directory for: ") + executablePath;
                return SLANG_FAIL;
            }
            List<String> arguments;
            arguments.add(executableSources[i]);
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
                outError =
                    String("slangc did not produce the expected executable: ") + executablePath;
                return SLANG_FAIL;
            }
        }
        SLANG_RETURN_ON_FAIL(_deployExecutableRuntime(slangcPath, hostRoot, outError));
    }
    else
    {
        Path::removeNonEmpty(hostRoot);
    }
    SLANG_RETURN_ON_FAIL(buildDocumentation(projectRoot, outError));
    return SLANG_OK;
}

/// Run an existing native executable configured by the workspace `host` section.
static SlangResult _run(
    const String& projectRoot,
    int argumentCount,
    const char* const* arguments,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    if (!hasHostExecutables(manifest))
    {
        outError = "The workspace does not configure a host executable. Add 'host.executables' to "
                   "slang-package.json and run 'slang package --experimental build'.";
        return SLANG_FAIL;
    }

    String executableName = manifest.host.defaultExecutable;
    int argumentIndex = 0;
    if (argumentCount > 0 && isHostExecutableName(manifest, arguments[0]))
    {
        executableName = arguments[0];
        argumentIndex = 1;
    }

    String executablePath = _getExecutableOutputPath(projectRoot, manifest, executableName);
    if (!File::exists(executablePath))
    {
        outError = String("The configured executable has not been built: ") + executablePath +
                   ". Run 'slang package --experimental build'.";
        return SLANG_FAIL;
    }
    List<String> executableArguments;
    for (int i = argumentIndex; i < argumentCount; ++i)
        executableArguments.add(arguments[i]);
    return _runStreamingSiblingTool(executablePath, executableArguments, outError);
}

/// Reserve `slang package test` without invoking `slang-test`. Package testing is not a
/// slang-test prefix yet: slang-test is an internal compiler harness with extra licenses, and the
/// package-owned test model is still undecided.
static SlangResult _test(const String& projectRoot, String& outError)
{
    SLANG_UNUSED(projectRoot);
    outError = "slang package test is not implemented yet.";
    appendErrorAdvice(
        outError,
        "The command is reserved until package testing has a dedicated model; it does not run "
        "slang-test.");
    return SLANG_FAIL;
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
    const String& as,
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
    package.as = as;
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
        String(),
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
    const String& as,
    String& outError)
{
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    LockedPackage* lockedPackage = _findLockedPackage(lock, name);
    if (!lockedPackage && !as.getLength())
    {
        outError = String("Override for package '") + name +
                   "' requires an 'as' version because it is not present in the lock.";
        return SLANG_FAIL;
    }
    if (lockedPackage && isPathOnlyLockedPackage(*lockedPackage))
    {
        outError = String("Manifest path dependency cannot be overridden: ") + name;
        return SLANG_FAIL;
    }
    String providedVersion = as.getLength() ? as : lockedPackage->version;
    SemanticVersion ignoredVersion;
    SLANG_RETURN_ON_FAIL(parseExactVersion(providedVersion, ignoredVersion, outError));

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_registerLocalPackage(
        projectRoot,
        name,
        path,
        providedVersion,
        LocalPackageKind::Override,
        localPackages,
        outError));
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    fprintf(
        stdout,
        "Package '%s' now uses '%s'. Run 'slang package update' to adopt its manifest.\n",
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

static SlangResult _setOverrideEnabled(
    const String& projectRoot,
    const String& name,
    bool enabled,
    String& outError)
{
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    Index localIndex = findLocalPackageIndex(localPackages, name);
    if (localIndex < 0 || isEditedLocalPackage(localPackages[localIndex]))
    {
        outError = String("Package has no local override: ") + name;
        return SLANG_FAIL;
    }
    if (localPackages[localIndex].enabled == enabled)
    {
        fprintf(
            stdout,
            "Override '%s' is already %s.\n",
            name.getBuffer(),
            enabled ? "enabled" : "disabled");
        return SLANG_OK;
    }

    localPackages[localIndex].enabled = enabled;
    SLANG_RETURN_ON_FAIL(writeProjectLocalPackages(projectRoot, localPackages, outError));

    Index lockedIndex = findLockedPackageIndex(lock, name);
    bool lockUsesOverride =
        lockedIndex >= 0 && isLocalOverrideLockedPackage(lock.packages[lockedIndex]);
    if (enabled || !lockUsesOverride)
    {
        SLANG_RETURN_ON_FAIL(
            _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    }
    fprintf(
        stdout,
        "Override '%s' is now %s. Run 'slang package update' to select the %s graph.\n",
        name.getBuffer(),
        enabled ? "enabled" : "disabled",
        enabled ? "local" : "published");
    return SLANG_OK;
}

static SlangResult _listOverrides(const String& projectRoot, String& outError)
{
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    Index count = 0;
    for (const auto& package : localPackages)
    {
        if (isEditedLocalPackage(package))
            continue;
        if (count++ == 0)
            fprintf(stdout, "Overrides:\n");
        fprintf(
            stdout,
            "  %s: %s at %s%s%s\n",
            package.name.getBuffer(),
            package.enabled ? "enabled" : "disabled",
            package.path.getBuffer(),
            package.as.getLength() ? " as " : "",
            package.as.getBuffer());
    }
    if (!count)
        fprintf(stdout, "Overrides: none.\n");
    return SLANG_OK;
}

SlangResult executeInDirectory(
    const String& projectRoot,
    int argc,
    const char* const* argv,
    String& outError)
{
    bool experimental = false;
    List<const char*> normalizedArguments;
    if (argc >= 2 && String(argv[1]) == "--experimental")
    {
        experimental = true;
        normalizedArguments.add(argv[0]);
        for (int i = 2; i < argc; ++i)
            normalizedArguments.add(argv[i]);
        argc = int(normalizedArguments.getCount());
        argv = normalizedArguments.getBuffer();
    }
    if (argc < 2 || String(argv[1]) == "help" || String(argv[1]) == "-help" ||
        String(argv[1]) == "--help")
    {
        _printHelp(experimental);
        return SLANG_OK;
    }

    String command = argv[1];
    if (command == "init" && argc == 2)
        return _init(projectRoot, outError);
    if (command == "fetch")
    {
        bool allowClean = false;
        bool assumeYes = false;
        bool skipValidate = false;
        for (int i = 2; i < argc; ++i)
        {
            String flag = argv[i];
            if (flag == "--clean")
                allowClean = true;
            else if (flag == "--yes")
                assumeYes = true;
            else if (flag == "--skip-validate")
                skipValidate = true;
            else
            {
                outError = String("Unknown fetch option: ") + flag;
                return SLANG_FAIL;
            }
        }
        return _fetch(projectRoot, allowClean, assumeYes, skipValidate, outError);
    }
    if (command == "update")
    {
        bool fromLocal = false;
        bool allowClean = false;
        bool dryRun = false;
        bool minimal = false;
        bool assumeYes = false;
        bool skipValidate = false;
        for (int i = 2; i < argc; ++i)
        {
            String flag = argv[i];
            if (flag == "--from-local")
                fromLocal = true;
            else if (flag == "--clean")
                allowClean = true;
            else if (flag == "--dry-run")
                dryRun = true;
            else if (flag == "--minimal")
                minimal = true;
            else if (flag == "--yes")
                assumeYes = true;
            else if (flag == "--skip-validate")
                skipValidate = true;
            else
            {
                outError = String("Unknown update option: ") + flag;
                return SLANG_FAIL;
            }
        }
        if (dryRun && allowClean)
        {
            outError = "update --dry-run cannot be combined with --clean.";
            return SLANG_FAIL;
        }
        return _update(
            projectRoot,
            fromLocal,
            allowClean,
            dryRun,
            minimal,
            assumeYes,
            skipValidate,
            outError);
    }
    if (command == "validate" && argc == 2)
        return _validate(projectRoot, outError);
    if (command == "build")
    {
        bool skipValidate = false;
        for (int i = 2; i < argc; ++i)
        {
            String flag = argv[i];
            if (flag == "--skip-validate")
                skipValidate = true;
            else
            {
                outError = String("Unknown build option: ") + flag;
                return SLANG_FAIL;
            }
        }
        return _build(projectRoot, experimental, skipValidate, outError);
    }
    if (command == "run")
    {
        if (!experimental)
        {
            outError =
                "Host executable run is experimental. Re-run as 'slang package --experimental "
                "run'.";
            return SLANG_FAIL;
        }
        return _run(projectRoot, argc - 2, argv + 2, outError);
    }
    if (command == "test" && argc == 2)
        return _test(projectRoot, outError);
    if (command == "docs" && argc == 2)
        return _printDocumentationLocation(projectRoot, outError);
    if (command == "status" && argc == 2)
        return _status(projectRoot, outError);
    if (command == "tree" && argc == 2)
        return _tree(projectRoot, outError);
    if (command == "why" && argc == 3)
        return _why(projectRoot, argv[2], outError);
    if (command == "dependency")
    {
        if (argc == 3 && String(argv[2]) == "list")
            return _dependencyList(projectRoot, outError);
        if (argc == 4 && String(argv[2]) == "remove")
            return _dependencyRemove(projectRoot, argv[3], outError);
        if (argc >= 4 && String(argv[2]) == "add")
        {
            Dependency dependency;
            dependency.name = argv[3];
            if (!isValidPackageName(dependency.name))
            {
                outError = String("Invalid dependency name: ") + dependency.name;
                return SLANG_FAIL;
            }
            for (int i = 4; i < argc; i += 2)
            {
                if (i + 1 >= argc)
                {
                    outError = String("Missing value for dependency option: ") + argv[i];
                    return SLANG_FAIL;
                }
                String option = argv[i];
                String value = argv[i + 1];
                if (option == "--git")
                    dependency.git = value;
                else if (option == "--path")
                    dependency.path = value;
                else if (option == "--version")
                    dependency.version = value;
                else if (option == "--ref")
                    dependency.ref = value;
                else if (option == "--as")
                    dependency.as = value;
                else
                {
                    outError = String("Unknown dependency add option: ") + option;
                    return SLANG_FAIL;
                }
            }
            bool validPath = dependency.path.getLength() && dependency.as.getLength() &&
                             !dependency.git.getLength() && !dependency.version.getLength() &&
                             !dependency.ref.getLength();
            bool validGitVersion = dependency.git.getLength() && dependency.version.getLength() &&
                                   !dependency.path.getLength() && !dependency.ref.getLength() &&
                                   !dependency.as.getLength();
            bool validGitRef = dependency.git.getLength() && dependency.ref.getLength() &&
                               dependency.as.getLength() && !dependency.path.getLength() &&
                               !dependency.version.getLength();
            if (!(validPath || validGitVersion || validGitRef))
            {
                outError = "Dependency add requires exactly one of: --git URL --version RANGE, "
                           "--git URL --ref REF --as VERSION, or --path PATH --as VERSION.";
                return SLANG_FAIL;
            }
            return _dependencyAdd(projectRoot, dependency, outError);
        }
        outError = "Invalid dependency command. Use 'dependency add', 'dependency remove', or "
                   "'dependency list'.";
        return SLANG_FAIL;
    }
    if (command == "edit" && argc == 3)
        return _edit(projectRoot, argv[2], outError);
    if (command == "unedit" && argc == 3)
        return _unedit(projectRoot, argv[2], outError);
    if (command == "override" && argc == 3 && String(argv[2]) == "list")
        return _listOverrides(projectRoot, outError);
    if (command == "override" && argc == 4 && String(argv[2]) == "enable")
        return _setOverrideEnabled(projectRoot, argv[3], true, outError);
    if (command == "override" && argc == 4 && String(argv[2]) == "disable")
        return _setOverrideEnabled(projectRoot, argv[3], false, outError);
    if (command == "override" && argc == 4 && String(argv[2]) == "remove")
        return _unoverride(projectRoot, argv[3], outError);
    if (command == "override" && (argc == 5 || argc == 6) && String(argv[2]) == "add")
        return _override(
            projectRoot,
            argv[3],
            argv[4],
            argc == 6 ? String(argv[5]) : String(),
            outError);
    if (command == "override" && (argc == 4 || argc == 5))
        return _override(
            projectRoot,
            argv[2],
            argv[3],
            argc == 5 ? String(argv[4]) : String(),
            outError);
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
