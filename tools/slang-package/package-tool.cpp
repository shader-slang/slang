// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-tool.h"

#include "core/slang-io.h"
#include "core/slang-platform.h"
#include "core/slang-process-util.h"
#include "core/slang-process.h"
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

static void _printHelp(bool experimental = false)
{
    fprintf(
        stdout,
        "Usage: slang-package [--experimental] <command>\n"
        "\n"
        "Manifest (slang-package.json):\n"
        "  init              Create a package in the current directory.\n"
        "  dependency add <name> --git <url> (--version <range> | --ref <ref> [--as <ver>])\n"
        "  dependency add <name> --path <path> --as <version>\n"
        "  dependency pin <name> [--to <version> | --commit]\n"
        "  dependency remove <name> | list\n"
        "\n"
        "Overlay (slang-package-overlay.json):\n"
        "  override add <name> <path> [as]\n"
        "  override enable|disable|remove <name> | list\n"
        "  edit <name>       Make a Git checkout editable in place.\n"
        "  unedit <name> [--clean | --adopt [--ref <ref>] [--as <version>]] [--yes]\n"
        "                    --clean restores the lock; --adopt pins HEAD into the manifest.\n"
        "\n"
        "Lock (slang-package-lock.json):\n"
        "  fetch [--clean] [--yes] [--skip-validate]\n"
        "                    Install the lock. Missing lock runs update. --clean discards "
        "checkout state.\n"
        "  update [--ignore-overrides] [--clean] [--dry-run] [--minimal] [--offline] [--yes]\n"
        "         [--skip-validate]\n"
        "                    Re-resolve and rewrite the lock. --offline uses .slang/cache only.\n"
        "  status            Lock and graph readiness (details only when dirty).\n"
        "  validate [name] [--all]\n"
        "                    Check this package for sharing; NAME or --all checks locked trees.\n"
        "  tree              Print the selected dependency graph.\n"
        "  why <name>        Print every graph path that requires a package.\n"
        "\n"
        "Build:\n"
        "  build [--skip-validate]   Source bundle and docs; fetches if needed.\n"
        "  run [name] [args...]      Invoke the interpreter on the source bundle.\n"
        "  docs [--print]            Open build/docs/index.md (--print writes the path).\n");
    if (experimental)
    {
        fprintf(
            stdout,
            "\n"
            "Experimental:\n"
            "  build            Also generate enabled modules and host executables.\n"
            "  run --binary [name] [args...]\n"
            "                   Run a native host executable from the last experimental build.\n");
    }
    else
    {
        fprintf(stdout, "\n--experimental enables experimental options.\n");
    }
    fprintf(stdout, "\nCommands start from the nearest slang-package.json.\n");
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

/// Ask the user to approve an operation after its effects have been printed, reporting the answer
/// in `outApproved`.
///
/// Declining is a normal outcome rather than a failure: the user reviewed the proposed changes and
/// chose to keep the workspace as it is, so the command reports that nothing was applied and
/// succeeds. Failure is reserved for the cases where no answer can be obtained at all.
/// Non-interactive callers must pass `--yes`; treating end-of-file as approval would let CI or a
/// redirected stdin accidentally apply a graph that nobody reviewed.
static SlangResult _confirmApply(
    bool assumeYes,
    const char* prompt,
    bool& outApproved,
    String& outError)
{
    outApproved = false;
    if (assumeYes)
    {
        outApproved = true;
        return SLANG_OK;
    }
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
    outApproved = isAffirmativeConfirmationAnswer(UnownedStringSlice(response));
    if (!outApproved)
        fprintf(stdout, "Cancelled; no changes were applied.\n");
    return SLANG_OK;
}

SlangResult discoverPackageRoot(const String& startDirectory, String& outRoot, String& outError)
{
    String current;
    if (SLANG_FAILED(Path::getCanonical(startDirectory, current)))
    {
        outError = String("Cannot determine the package directory: ") + startDirectory;
        return SLANG_FAIL;
    }

    String directory = current;
    for (;;)
    {
        if (File::exists(Path::combine(directory, kPackageFileName)))
        {
            outRoot = directory;
            return SLANG_OK;
        }
        String parent = Path::getParentDirectory(directory);
        if (parent.getLength() == 0 || parent == directory)
            break;
        directory = parent;
    }

    outError = String("Cannot find slang-package.json from ") + current +
               ". Run this command from a package directory or a subdirectory of a package.";
    return SLANG_FAIL;
}

static int _commandArgumentIndex(int argc, const char* const* argv)
{
    int index = 1;
    if (index < argc && String(argv[index]) == "--experimental")
        ++index;
    return index;
}

static bool _isHelpCommand(int argc, const char* const* argv)
{
    int index = _commandArgumentIndex(argc, argv);
    if (index >= argc)
        return true;
    String command = argv[index];
    return command == "help" || command == "-help" || command == "--help";
}

static bool _isInitCommand(int argc, const char* const* argv)
{
    int index = _commandArgumentIndex(argc, argv);
    return index < argc && String(argv[index]) == "init";
}

static bool _commandRequiresPackageRoot(int argc, const char* const* argv)
{
    if (_isHelpCommand(argc, argv) || _isInitCommand(argc, argv))
        return false;
    int index = _commandArgumentIndex(argc, argv);
    if (index >= argc)
        return false;
    String command = argv[index];
    return command == "fetch" || command == "update" || command == "validate" ||
           command == "build" || command == "run" || command == "test" || command == "docs" ||
           command == "status" || command == "tree" || command == "why" ||
           command == "dependency" || command == "override" || command == "edit" ||
           command == "unedit";
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

/// Write `slang-package-includes.txt` with the same export roots `build` passes to `slangc`.
///
/// Those roots come from `getLockedPackageRoot`, so Git checkouts, path dependencies, and local
/// overrides are all workspace-rooted. A later `slangc -I` can use a line from this file even when
/// the compiler is invoked from a subdirectory. Fetch and update regenerate it; it is not a build
/// output.
static SlangResult _writeSearchPaths(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError)
{
    StringBuilder searchPaths;
    String depsDirectory = getWorkspaceDepsDirectory(manifest);
    for (const auto& package : lock.packages)
    {
        String packageRoot;
        SLANG_RETURN_ON_FAIL(getLockedPackageRoot(
            projectRoot,
            depsDirectory,
            package,
            localPackages,
            packageRoot,
            outError));
        Manifest packageManifest;
        SLANG_RETURN_ON_FAIL(loadLockedPackageGraphManifest(
            projectRoot,
            manifest,
            package,
            localPackages,
            packageManifest,
            outError));
        for (const auto& exportPath : packageManifest.exports)
            searchPaths << Path::combine(packageRoot, exportPath) << "\n";
    }

    if (SLANG_FAILED(
            File::writeAllText(Path::combine(projectRoot, kIncludesFileName), searchPaths)))
    {
        outError = String("Cannot write ") + kIncludesFileName + ".";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _clearSearchPaths(const String& projectRoot, String& outError)
{
    if (SLANG_FAILED(File::writeAllText(Path::combine(projectRoot, kIncludesFileName), "")))
    {
        outError = String("Cannot clear ") + kIncludesFileName + " before materializing packages.";
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
    const List<String>& movingRefPackages,
    List<String>* outChangedPackageNames,
    String& outError)
{
    if (outChangedPackageNames)
        outChangedPackageNames->clear();
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
                isInPlaceLocalPackage(manifest, localPackages[localIndex]) ? "edit" : "override",
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
                       "' is not registered in slang-package-overlay.json.";
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
        bool didMaterialize = false;
        String cachePath =
            Path::combine(Path::combine(projectRoot, ".slang", "cache"), package.name);
        SLANG_RETURN_ON_FAIL(materializeLockedRevision(
            package.git,
            currentCommit,
            package.commit,
            destination,
            allowClean,
            movingRefPackages.indexOf(package.name) >= 0,
            didMaterialize,
            outError,
            cachePath));
        if (didMaterialize)
        {
            if (outChangedPackageNames)
                outChangedPackageNames->add(package.name);
            fprintf(
                stdout,
                "Checked out '%s' at %s (%s).\n",
                package.name.getBuffer(),
                package.ref.getBuffer(),
                package.commit.getBuffer());
        }
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

/// Inventory named refs that cache staging would move in existing dependency repositories.
///
/// Objects and new refs are additive. An existing tag or origin-tracking branch has a meaning the
/// user may rely on, so changing its target is disclosed before any dependency repository is
/// mutated. A checkout already scheduled for `--clean` replacement is omitted because its entire
/// repository is covered by the stronger replacement warning.
static SlangResult _collectMovingPackageRefs(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    const List<String>& cleanReplacements,
    List<String>& outPackageNames,
    List<String>& outFacts,
    String& outError)
{
    outPackageNames.clear();
    outFacts.clear();
    String depsRoot = Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest));
    String cacheRoot = Path::combine(projectRoot, ".slang", "cache");
    for (const auto& package : lock.packages)
    {
        if (!isGitBackedLockedPackage(package) || package.path.getLength() ||
            findActiveLocalPackageIndex(localPackages, package.name) >= 0 ||
            cleanReplacements.indexOf(package.name) >= 0)
        {
            continue;
        }

        String destination = Path::combine(depsRoot, package.name);
        List<String> refs;
        SLANG_RETURN_ON_FAIL(collectMovingCachedRefs(
            Path::combine(cacheRoot, package.name),
            destination,
            refs,
            outError));
        if (!refs.getCount())
            continue;

        StringBuilder fact;
        fact << package.name << ": ";
        for (Index i = 0; i < refs.getCount(); ++i)
            fact << (i ? ", " : "") << refs[i];
        outPackageNames.add(package.name);
        outFacts.add(fact.produceString());
    }
    return SLANG_OK;
}

static void _printMovingRefWarning(const List<String>& facts)
{
    if (!facts.getCount())
        return;
    fprintf(stdout, "Staging from cache will move existing refs in:\n");
    for (const auto& fact : facts)
        fprintf(stdout, "  %s\n", fact.getBuffer());
}

static void _appendIncompleteMaterializationAdvice(String& ioError, bool previousLockExists)
{
    appendErrorAdvice(
        ioError,
        previousLockExists
            ? "The previous lock remains authoritative, but deps/ may be partially changed and "
              "slang-package-includes.txt may be empty. Run 'slang package fetch' to restore it; "
              "add "
              "'--clean' only if replacement is intended."
            : "No lock was written, but deps/ may be partial and slang-package-includes.txt may be "
              "empty. "
              "Fix the reported error and run 'slang package fetch' again.");
}

static SlangResult _readProjectManifest(
    const String& projectRoot,
    Manifest& outManifest,
    String& outError)
{
    return readManifest(Path::combine(projectRoot, kPackageFileName), outManifest, outError);
}

static SlangResult _readProjectLock(const String& projectRoot, LockFile& outLock, String& outError)
{
    return readLockFile(Path::combine(projectRoot, kLockFileName), outLock, outError);
}

/// Check that the current dependency graph, including overrides, still selects this lock.
///
/// Declared edges come from overlay working trees and from each locked version's manifest. This
/// does not search for newer Git tags.
static SlangResult _validateLockAgainstManifest(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError)
{
    return validateWorkspaceResolvedProject(
        projectRoot,
        manifest,
        lock,
        localPackages,
        outError,
        nullptr);
}

/// Verify that every registered local tree matches its locked slot and every path lock is
/// registered.
static SlangResult _validateLocalPackages(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
    List<String>* outWarnings = nullptr)
{
    for (const auto& localPackage : localPackages)
    {
        if (!isActiveLocalPackage(localPackage))
            continue;
        Index packageIndex = findLockedPackageIndex(lock, localPackage.name);
        if (packageIndex < 0)
        {
            if (isInPlaceLocalPackage(manifest, localPackage))
            {
                if (outWarnings)
                {
                    outWarnings->add(
                        String("In-place override '") + localPackage.name +
                        "' is not in this graph; leaving its checkout and registration in "
                        "place.");
                }
                continue;
            }
            outError =
                String("Registered local package is not present in the lock: ") + localPackage.name;
            return SLANG_FAIL;
        }
        const LockedPackage* package = &lock.packages[packageIndex];
        if (package->path.getLength() && package->path != localPackage.path)
        {
            outError = String("Locked path for package '") + package->name +
                       "' does not match slang-package-overlay.json. Run "
                       "'slang package update'.";
            return SLANG_FAIL;
        }
        if (localPackage.as.getLength() && package->version != localPackage.as)
        {
            outError = String("Locked version for local override '") + package->name +
                       "' does not match slang-package-overlay.json. Run "
                       "'slang package update'.";
            return SLANG_FAIL;
        }
        Manifest localManifest;
        SLANG_RETURN_ON_FAIL(
            readLocalPackageManifest(projectRoot, localPackage, localManifest, outError));
        if (SLANG_FAILED(validateLockedPackageManifest(*package, localManifest, outError)))
        {
            appendErrorAdvice(
                outError,
                "Align the local manifest with the selected upstream graph, or run "
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
                       "' is not registered in slang-package-overlay.json. Run "
                       "'slang package update' to restore a published pin.";
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

/// Validate enough of the locked graph to regenerate search paths after a local registration
/// changes.
///
/// An overlay may declare dependencies the lock does not yet contain. Those edges wait for
/// `update`. Existing lock rows must still be reachable, and overlay edges that already have a lock
/// row must still select that pin.
static SlangResult _validateGraphAfterLocalRegistrationChange(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
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
                readManifest(Path::combine(packageRoot, kPackageFileName), manifest, outError)))
        {
            outError = String("Cannot read the dependency manifest of locked package '") +
                       package.name + "'. " + outError;
            return SLANG_FAIL;
        }
        if (localIndex < 0)
            SLANG_RETURN_ON_FAIL(validateLockedPackageManifest(package, manifest, outError));
        addSlangToolchainConstraint(manifest, toolchainConstraints);
        addUnadoptedWorkspaceExclusionWarnings(rootManifest, package.name, manifest, outWarnings);
        for (const auto& dependency : manifest.dependencies)
        {
            Index dependencyIndex = findLockedPackageIndex(lock, dependency.name);
            if (dependencyIndex < 0)
                continue;
            SLANG_RETURN_ON_FAIL(
                validateLockedDependency(dependency, lock, dependencyIndex, outError));
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
    SLANG_RETURN_ON_FAIL(_validateGraphAfterLocalRegistrationChange(
        projectRoot,
        manifest,
        lock,
        localPackages,
        outError));
    return _writeSearchPaths(projectRoot, manifest, lock, localPackages, outError);
}

/// Return whether fetch/update should run the shareable-package checks on this lock row.
///
/// Git packages that were rematerialized are handled separately through `changedPackageNames`, so
/// this covers enabled local overrides instead. Those trees are a
/// library the developer is preparing to share, which is what lets `update` right after a promote
/// certify the package before any remote tag exists.
///
/// A vendored path row is deliberately excluded. Consider an application that keeps a package in
/// `vendor/math` and depends on it with `path: vendor/math`. That tree is how the application
/// consumes an in-repo package, and it is allowed to do things a published library may not, such
/// as declaring a sibling `path: ../shared-math` dependency that a clone of the application repo
/// alone would not reproduce. Asking whether such a tree could be published is a deliberate
/// question, so it belongs to `validate NAME` and `validate --all` rather than to every fetch.
static bool _shouldPublishCheckChangedLocalPackage(
    const LockFile* previousLock,
    const LockedPackage& package,
    const List<LocalPackage>& localPackages)
{
    if (findActiveLocalPackageIndex(localPackages, package.name) < 0)
        return false;
    if (!previousLock)
        return true;
    Index previousIndex = findLockedPackageIndex(*previousLock, package.name);
    if (previousIndex < 0)
        return true;
    return !lockedPackagesEqual(previousLock->packages[previousIndex], package);
}

/// Run the shareable-package checks on one locked tree and verify its declared edges against
/// this workspace lock, not against a nested lock under that package.
static SlangResult _validateLockedPackagePublishable(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    const LockedPackage& package,
    String& outError)
{
    String packageRoot;
    SLANG_RETURN_ON_FAIL(getLockedPackageRoot(
        projectRoot,
        getWorkspaceDepsDirectory(rootManifest),
        package,
        localPackages,
        packageRoot,
        outError));
    Manifest manifest;
    if (SLANG_FAILED(
            readManifest(Path::combine(packageRoot, kPackageFileName), manifest, outError)))
    {
        outError = String("Cannot validate package '") + package.name + "': " + outError;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(validatePublishablePackage(packageRoot, manifest, outError));
    for (const auto& dependency : manifest.dependencies)
    {
        Index dependencyIndex = -1;
        SLANG_RETURN_ON_FAIL(validateLockedDependency(dependency, lock, dependencyIndex, outError));
        SLANG_RETURN_ON_FAIL(validateLockedPathDependency(
            projectRoot,
            packageRoot,
            manifest.name,
            dependency,
            lock.packages[dependencyIndex],
            outError));
    }
    return SLANG_OK;
}

/// Validate packages whose Git checkout changed during materialization, plus any changed local
/// registration, as newly accepted releases.
///
/// An unchanged package cannot develop a new license or source-layout defect, while the separate
/// buildable-closure check still catches interactions such as an import collision between a
/// changed package and an unchanged one.
static SlangResult _validateChangedPublishablePackages(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const LockFile* previousLock,
    const List<LocalPackage>& localPackages,
    const List<String>& changedPackageNames,
    String& outError)
{
    List<String> names;
    for (const auto& packageName : changedPackageNames)
    {
        if (names.indexOf(packageName) < 0)
            names.add(packageName);
    }
    for (const auto& package : lock.packages)
    {
        if (!_shouldPublishCheckChangedLocalPackage(previousLock, package, localPackages))
            continue;
        if (names.indexOf(package.name) < 0)
            names.add(package.name);
    }
    for (const auto& packageName : names)
    {
        Index packageIndex = findLockedPackageIndex(lock, packageName);
        SLANG_RELEASE_ASSERT(packageIndex >= 0);
        SLANG_RETURN_ON_FAIL(_validateLockedPackagePublishable(
            projectRoot,
            rootManifest,
            lock,
            localPackages,
            lock.packages[packageIndex],
            outError));
    }
    return SLANG_OK;
}

static SlangResult _init(const String& projectRoot, String& outError)
{
    String manifestPath = Path::combine(projectRoot, kPackageFileName);
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
        "slang-package-overlay.json",
        "slang-package-includes.txt",
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
    return writeManifest(Path::combine(projectRoot, kPackageFileName), manifest, outError);
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

/// Write a Git pin into the workspace manifest from the current lock selection.
///
/// Default copies the locked exact version as `git` plus `version`, so a later update cannot float
/// to a newer compatible tag. `--to` writes that exact version instead of the locked one.
/// `--commit` writes `git` plus `ref` plus `as` using the locked SHA, so a moved tag is not
/// reselected. A transitive package is promoted to a direct edge using the lock's Git URL. Path
/// dependencies are already pins. An overlay stays in slang-package-overlay.json; this command only
/// edits committed Git intent. Because an overlay row does not have a locked SHA, `--commit`
/// requires a Git-only lock row. Its effective version is also local intent, so default pin
/// requires a Git-only row while `--to` makes the version explicit. This command does not rewrite
/// the lock; inspect status and run update afterward, the same as `dependency add`.
static SlangResult _dependencyPin(
    const String& projectRoot,
    const String& name,
    const String& requestedVersion,
    bool hasRequestedVersion,
    bool pinCommit,
    String& outError)
{
    if (hasRequestedVersion && pinCommit)
    {
        outError = "dependency pin --to cannot be combined with --commit.";
        return SLANG_FAIL;
    }
    if (hasRequestedVersion)
    {
        SemanticVersion ignored;
        SLANG_RETURN_ON_FAIL(parseExactVersion(requestedVersion, ignored, outError));
    }

    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    Index existingIndex = _findDependencyIndex(manifest, name);
    if (existingIndex >= 0 && manifest.dependencies[existingIndex].path.getLength())
    {
        outError = String("Path dependency is already pinned: ") + name;
        return SLANG_FAIL;
    }

    String lockPath = Path::combine(projectRoot, kLockFileName);
    if (!File::exists(lockPath))
    {
        outError = "dependency pin requires slang-package-lock.json. Run 'slang package update'.";
        return SLANG_FAIL;
    }
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    Index lockedIndex = findLockedPackageIndex(lock, name);
    if (lockedIndex < 0)
    {
        outError = String("Lock file does not contain package '") + name + "'.";
        return SLANG_FAIL;
    }
    const LockedPackage& locked = lock.packages[lockedIndex];
    if (!isGitBackedLockedPackage(locked))
    {
        outError = String("Cannot pin a path-only package: ") + name;
        return SLANG_FAIL;
    }
    if (existingIndex >= 0 && manifest.dependencies[existingIndex].git != locked.git)
    {
        outError = String("Lock file uses a different Git URL for dependency '") + name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }
    if (locked.path.getLength() && pinCommit)
    {
        outError = String("Cannot pin overlaid package '") + name +
                   "' by commit because its lock row selects a local path. Disable the override "
                   "and run 'slang package update', use --to with a published version, or use "
                   "'slang package unedit " +
                   name + " --adopt' to keep the working-tree commit.";
        return SLANG_FAIL;
    }
    if (locked.path.getLength() && !hasRequestedVersion)
    {
        outError = String("Cannot infer a published Git version for overlaid package '") + name +
                   "' because its lock row selects a local path. Pass --to VERSION.";
        return SLANG_FAIL;
    }
    if (pinCommit)
        SLANG_RELEASE_ASSERT(locked.commit.getLength());
    String pinVersion = hasRequestedVersion ? requestedVersion : locked.version;
    if (!pinCommit && !pinVersion.getLength())
    {
        outError = String("Lock file does not record a version for package '") + name + "'.";
        return SLANG_FAIL;
    }

    Dependency dependency;
    dependency.name = name;
    dependency.git = locked.git;
    if (pinCommit)
    {
        dependency.ref = locked.commit;
        dependency.as = locked.version;
    }
    else
        dependency.version = pinVersion;
    return _dependencyAdd(projectRoot, dependency, outError);
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
                "  %s: %s ref %s%s%s\n",
                dependency.name.getBuffer(),
                dependency.git.getBuffer(),
                dependency.ref.getBuffer(),
                dependency.as.getLength() ? " as " : "",
                dependency.as.getBuffer());
        }
    }
    return SLANG_OK;
}

static void _warnSkippedSourceValidation()
{
    fprintf(
        stderr,
        "slang-package: warning: skipped source-layout and new-release publish validation "
        "(--skip-validate).\n");
}

/// Tell the user that this command is invoking another, and why.
///
/// Build may run fetch, and fetch with no lock may run update. Those hand-offs are easy to miss
/// in a wall of resolver output, so each one prints a one-line notice first.
static void _announceSubcommand(const char* because, const char* command)
{
    fprintf(stdout, "slang-package: %s; running '%s'.\n", because, command);
}

/// Join nonzero Git checkout facts, omitting zero ahead/behind/stash counts.
///
/// Consider this example: a lock pins `color` at commit A, and `deps/color` has one untracked
/// file but is still at A. Status should say `color: 1 changed`, not four zero counters. If HEAD
/// is not A and `rev-list` reports no ahead/behind (for example a detached other commit), the
/// remaining fact is `not at locked commit`.
///
/// An empty result therefore means the checkout is clean and sitting on the locked commit, which
/// is exactly the condition under which materialization is allowed to replace it. Both `status`
/// and the fetch/update preflight describe drift through this function so that the facts a user
/// reads from `status` are the same facts that stop a command.
static String _describeDirtyCheckout(
    const GitWorkingTreeStatus& gitStatus,
    const String& expectedCommit)
{
    List<String> facts;
    if (gitStatus.changedFileCount)
        facts.add(String(gitStatus.changedFileCount) + " changed");
    if (gitStatus.commitsAhead)
        facts.add(String(gitStatus.commitsAhead) + " ahead");
    if (gitStatus.commitsBehind)
        facts.add(String(gitStatus.commitsBehind) + " behind");
    if (gitStatus.stashCount)
    {
        facts.add(
            String(gitStatus.stashCount) + (gitStatus.stashCount == 1 ? " stash" : " stashes"));
    }
    if (gitStatus.headCommit != expectedCommit && !gitStatus.commitsAhead &&
        !gitStatus.commitsBehind)
        facts.add("not at locked commit");
    StringBuilder detail;
    for (Index i = 0; i < facts.getCount(); ++i)
        detail << (i ? ", " : "") << facts[i];
    return detail.produceString();
}

/// Fail when a checkout the lock owns holds local work that this command would have to discard.
///
/// Consider this example: `deps/color-encoding` was materialized from the lock, and then a file
/// in it is edited without running `slang package edit color-encoding`. Materialization already
/// refuses to overwrite that tree without `--clean`, but it only reaches that decision after the
/// graph has been resolved, the plan has been printed, the user has confirmed it, and
/// `slang-package-includes.txt` has been cleared. The user is then told the command failed after
/// reading a report that described the new graph as if it had been installed.
///
/// Never discarding local work without `--clean` is the overriding rule here, so the trees the
/// current lock owns are inspected before anything else happens: no solve, no report, no prompt,
/// no cleared search paths. Enabled overrides are skipped because those trees belong to the user
/// and materialization does not touch them.
static SlangResult _refuseIfOwnedCheckoutsAreDirty(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError)
{
    String depsRoot = Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest));
    List<String> dirtyNames;
    List<String> dirtyFacts;
    for (const auto& package : lock.packages)
    {
        if (findActiveLocalPackageIndex(localPackages, package.name) >= 0 ||
            !isGitBackedLockedPackage(package) || package.path.getLength())
        {
            continue;
        }

        String destination = Path::combine(depsRoot, package.name);
        SlangPathType pathType;
        // An absent checkout is the normal case for fetch and update, and holds nothing to keep.
        if (SLANG_FAILED(Path::getPathType(destination, &pathType)) ||
            pathType != SLANG_PATH_TYPE_DIRECTORY)
        {
            continue;
        }

        auto addDirty = [&](const String& description)
        {
            dirtyNames.add(package.name);
            dirtyFacts.add(package.name + ": " + description);
        };

        // A path that is not the repository the lock names is reported rather than inspected: Git
        // cannot describe its drift against a commit it does not contain, and materialization
        // would still have to delete the whole directory to install the locked package there.
        String origin;
        String issue;
        if (SLANG_FAILED(getRepositoryOrigin(destination, origin, issue)))
        {
            addDirty("not a git checkout");
            continue;
        }
        if (origin != package.git)
        {
            addDirty("different origin");
            continue;
        }

        GitWorkingTreeStatus gitStatus;
        SLANG_RETURN_ON_FAIL(
            getWorkingTreeStatus(destination, package.commit, gitStatus, outError));
        String description = _describeDirtyCheckout(gitStatus, package.commit);
        if (!description.getLength())
            continue;
        addDirty(description);
    }
    if (!dirtyNames.getCount())
        return SLANG_OK;

    StringBuilder message;
    message << "Refusing to replace a dependency checkout under '"
            << getWorkspaceDepsDirectory(manifest) << "/' that has local state without --clean:";
    for (const auto& fact : dirtyFacts)
        message << "\n  " << fact;
    outError = message.produceString();
    String editTarget = dirtyNames.getCount() == 1 ? dirtyNames[0] : String("<name>");
    appendErrorAdvice(
        outError,
        String("Fetch and update replace the checkouts the lock owns, so they stop before "
               "touching any tree when one of them holds local state. Commit or discard the "
               "changes, run 'slang package edit ") +
            editTarget +
            "' to keep working in that checkout, or re-run with '--clean' to discard local "
            "checkout state and restore the locked commit.");
    return SLANG_FAIL;
}

static SlangResult _update(
    const String& projectRoot,
    bool ignoreOverrides,
    bool allowClean,
    bool dryRun,
    bool minimal,
    bool assumeYes,
    bool skipValidate,
    bool offline,
    String& outError);

static SlangResult _fetch(
    const String& projectRoot,
    bool allowClean,
    bool assumeYes,
    bool skipValidate,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    String lockPath = Path::combine(projectRoot, kLockFileName);
    if (!File::exists(lockPath))
    {
        if (!manifest.dependencies.getCount())
        {
            outError = "fetch requires slang-package-lock.json when there is no dependency graph "
                       "to resolve. Run 'slang package update' to create an empty lock.";
            return SLANG_FAIL;
        }
        _announceSubcommand(
            "slang-package-lock.json is missing",
            assumeYes ? "slang package update --yes" : "slang package update");
        return _update(
            projectRoot,
            false,
            allowClean,
            false,
            false,
            assumeYes,
            skipValidate,
            false,
            outError);
    }

    LockFile lock;
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    if (!allowClean)
    {
        SLANG_RETURN_ON_FAIL(
            _refuseIfOwnedCheckoutsAreDirty(projectRoot, manifest, lock, localPackages, outError));
    }
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(
        _validateLocalPackages(projectRoot, manifest, lock, localPackages, outError, &warnings));
    SLANG_RETURN_ON_FAIL(validateLockedWorkspaceExclusions(manifest, lock, outError));
    SLANG_RETURN_ON_FAIL(refreshAndValidateUpstreamResolvedProject(projectRoot, lock, outError));
    SLANG_RETURN_ON_FAIL(validateWorkspaceResolvedProject(
        projectRoot,
        manifest,
        lock,
        localPackages,
        outError,
        &warnings));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    warnings.clear();
    if (skipValidate)
        _warnSkippedSourceValidation();
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
    }
    List<String> movingRefPackages;
    List<String> movingRefFacts;
    SLANG_RETURN_ON_FAIL(_collectMovingPackageRefs(
        projectRoot,
        manifest,
        lock,
        localPackages,
        cleanReplacements,
        movingRefPackages,
        movingRefFacts,
        outError));
    _printMovingRefWarning(movingRefFacts);
    if (cleanReplacements.getCount() || movingRefFacts.getCount())
    {
        bool approved = false;
        SLANG_RETURN_ON_FAIL(_confirmApply(
            assumeYes,
            "Apply these dependency repository changes?",
            approved,
            outError));
        if (!approved)
            return SLANG_OK;
    }
    SLANG_RETURN_ON_FAIL(_clearSearchPaths(projectRoot, outError));
    List<String> changedPackageNames;
    if (SLANG_FAILED(_materialize(
            projectRoot,
            manifest,
            lock,
            &lock,
            localPackages,
            allowClean,
            movingRefPackages,
            &changedPackageNames,
            outError)))
    {
        _appendIncompleteMaterializationAdvice(outError, true);
        return SLANG_FAIL;
    }
    if (!skipValidate)
    {
        if (SLANG_FAILED(validateBuildableResolvedProject(
                projectRoot,
                manifest,
                lock,
                localPackages,
                outError,
                &warnings,
                nullptr,
                nullptr,
                false,
                true)))
        {
            _appendIncompleteMaterializationAdvice(outError, true);
            return SLANG_FAIL;
        }
        if (SLANG_FAILED(_validateChangedPublishablePackages(
                projectRoot,
                manifest,
                lock,
                &lock,
                localPackages,
                changedPackageNames,
                outError)))
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

static bool _hasEnabledOutOfTreeOverride(
    const Manifest& manifest,
    const List<LocalPackage>& localPackages)
{
    for (const auto& localPackage : localPackages)
    {
        if (localPackage.enabled && !isInPlaceLocalPackage(manifest, localPackage))
            return true;
    }
    return false;
}

/// Copy workspace registrations for one `update`, optionally disabling out-of-tree overrides.
///
/// An in-place override continues to own `deps/NAME` under `--ignore-overrides`; treating it as
/// inactive could let materialization replace the checkout the user entered through `edit`.
static void _localPackagesForUpdate(
    const Manifest& manifest,
    const List<LocalPackage>& localPackages,
    bool ignoreOverrides,
    List<LocalPackage>& outEffective)
{
    outEffective.clear();
    for (const auto& localPackage : localPackages)
    {
        LocalPackage copy = localPackage;
        if (ignoreOverrides && !isInPlaceLocalPackage(manifest, copy))
            copy.enabled = false;
        outEffective.add(copy);
    }
}

static SlangResult _update(
    const String& projectRoot,
    bool ignoreOverrides,
    bool allowClean,
    bool dryRun,
    bool minimal,
    bool assumeYes,
    bool skipValidate,
    bool offline,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    const bool ignoredEnabledOverrides =
        ignoreOverrides && _hasEnabledOutOfTreeOverride(manifest, localPackages);
    List<LocalPackage> effectiveLocalPackages;
    _localPackagesForUpdate(manifest, localPackages, ignoreOverrides, effectiveLocalPackages);
    bool useLocalResolver = false;
    for (const auto& localPackage : effectiveLocalPackages)
        useLocalResolver = useLocalResolver || localPackage.enabled;

    LockFile previousLock;
    LockFile* previousLockPtr = nullptr;
    String lockPath = Path::combine(projectRoot, kLockFileName);
    if (File::exists(lockPath))
    {
        SLANG_RETURN_ON_FAIL(readLockFile(lockPath, previousLock, outError));
        previousLockPtr = &previousLock;
    }
    // Inspect the checkouts the existing lock owns before resolving. A dirty tool-owned tree means
    // this update cannot be applied at all, and the user should learn that instead of reading a
    // plan for a graph that will never be installed. A dry run installs nothing, so it is free to
    // report the plan regardless of what the checkouts look like.
    if (previousLockPtr && !dryRun && !allowClean)
    {
        SLANG_RETURN_ON_FAIL(_refuseIfOwnedCheckoutsAreDirty(
            projectRoot,
            manifest,
            previousLock,
            effectiveLocalPackages,
            outError));
    }
    if (useLocalResolver)
    {
        for (auto& localPackage : effectiveLocalPackages)
        {
            if (!isActiveLocalPackage(localPackage))
                continue;
            if (!localPackage.as.getLength())
            {
                String checkout;
                String headCommit;
                SemanticVersion inferredVersion;
                String tag;
                bool foundTag = false;
                String inferError;
                if (SLANG_SUCCEEDED(
                        getLocalPackageRoot(projectRoot, localPackage, checkout, inferError)) &&
                    SLANG_SUCCEEDED(getRepositoryHeadCommit(checkout, headCommit, inferError)) &&
                    SLANG_SUCCEEDED(findNearestReleaseTag(
                        checkout,
                        headCommit,
                        tag,
                        inferredVersion,
                        foundTag,
                        inferError)) &&
                    foundTag)
                {
                    localPackage.as = formatExactVersion(inferredVersion);
                }
            }
            if (localPackage.as.getLength())
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
            effectiveLocalPackages,
            lock,
            outError,
            &warnings,
            &report,
            offline));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(resolveDependencies(
            projectRoot,
            manifest,
            lock,
            outError,
            &warnings,
            &report,
            offline));
    }
    SLANG_RETURN_ON_FAIL(_validateLocalPackages(
        projectRoot,
        manifest,
        lock,
        effectiveLocalPackages,
        outError,
        &warnings));
    SLANG_RETURN_ON_FAIL(validateWorkspaceResolvedProject(
        projectRoot,
        manifest,
        lock,
        effectiveLocalPackages,
        outError,
        &warnings));
    SLANG_RETURN_ON_FAIL(validateCachedResolvedProject(projectRoot, lock, outError));
    // The report is printed before anything is materialized, so it always describes a plan. Only
    // the summary printed after the lock and the checkouts have been written may claim the work
    // happened.
    String reportText =
        formatResolveReport(manifest, previousLockPtr, lock, report, /* planned */ true, minimal);
    if (ignoredEnabledOverrides)
    {
        fprintf(
            stderr,
            "slang-package: warning: ignoring enabled overrides for this update; they remain in "
            "slang-package-overlay.json.\n");
    }
    List<String> cleanReplacements;
    if (allowClean)
    {
        SLANG_RETURN_ON_FAIL(_collectCheckoutsRequiringClean(
            projectRoot,
            manifest,
            lock,
            previousLockPtr,
            effectiveLocalPackages,
            cleanReplacements,
            outError));
        _printCleanReplacementWarning(cleanReplacements);
    }
    List<String> movingRefPackages;
    List<String> movingRefFacts;
    SLANG_RETURN_ON_FAIL(_collectMovingPackageRefs(
        projectRoot,
        manifest,
        lock,
        effectiveLocalPackages,
        cleanReplacements,
        movingRefPackages,
        movingRefFacts,
        outError));
    _printMovingRefWarning(movingRefFacts);
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
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    warnings.clear();
    if (skipValidate)
        _warnSkippedSourceValidation();
    fprintf(stdout, "%s", reportText.getBuffer());
    // Only ask when there is a decision to make. A different committed lock is a change the user
    // should review, and `--clean` discards local checkout state, but re-running `update` on an
    // already-current graph only checks and validates what the lock already says.
    const bool lockChanges = !previousLockPtr || !lockFilesEqual(*previousLockPtr, lock);
    if (lockChanges || cleanReplacements.getCount() || movingRefFacts.getCount())
    {
        bool approved = false;
        SLANG_RETURN_ON_FAIL(_confirmApply(assumeYes, "Apply this update?", approved, outError));
        if (!approved)
            return SLANG_OK;
    }
    SLANG_RETURN_ON_FAIL(_clearSearchPaths(projectRoot, outError));
    List<String> changedPackageNames;
    if (SLANG_FAILED(_materialize(
            projectRoot,
            manifest,
            lock,
            previousLockPtr,
            effectiveLocalPackages,
            allowClean,
            movingRefPackages,
            &changedPackageNames,
            outError)))
    {
        _appendIncompleteMaterializationAdvice(outError, previousLockPtr != nullptr);
        return SLANG_FAIL;
    }
    if (!skipValidate)
    {
        if (SLANG_FAILED(validateBuildableResolvedProject(
                projectRoot,
                manifest,
                lock,
                effectiveLocalPackages,
                outError,
                &warnings,
                nullptr,
                nullptr,
                false,
                true)))
        {
            _appendIncompleteMaterializationAdvice(outError, previousLockPtr != nullptr);
            return SLANG_FAIL;
        }
        if (SLANG_FAILED(_validateChangedPublishablePackages(
                projectRoot,
                manifest,
                lock,
                previousLockPtr,
                effectiveLocalPackages,
                changedPackageNames,
                outError)))
        {
            _appendIncompleteMaterializationAdvice(outError, previousLockPtr != nullptr);
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(writeLockFile(lockPath, lock, outError));
    SLANG_RETURN_ON_FAIL(
        _writeSearchPaths(projectRoot, manifest, lock, effectiveLocalPackages, outError));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    if (lockChanges)
    {
        String summary = formatResolveSummary(manifest, previousLockPtr, lock, report);
        fprintf(stdout, "%s", summary.getBuffer());
    }
    if (useLocalResolver)
    {
        fprintf(
            stdout,
            "The workspace contains local package state and requires "
            "slang-package-overlay.json.\n");
    }
    return SLANG_OK;
}

static SlangResult _validate(const String& projectRoot, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    SLANG_RETURN_ON_FAIL(validatePublishablePackage(projectRoot, manifest, outError));

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    for (const auto& localPackage : localPackages)
    {
        if (!isActiveLocalPackage(localPackage))
            continue;
        outError =
            String("Package cannot be published while local package '") + localPackage.name +
            "' is in " +
            (isInPlaceLocalPackage(manifest, localPackage) ? "edit mode." : "override mode.");
        return SLANG_FAIL;
    }

    String lockPath = Path::combine(projectRoot, kLockFileName);
    LockFile lock;
    if (File::exists(lockPath))
    {
        SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
    }
    else if (manifest.dependencies.getCount())
    {
        outError = "Package dependencies require slang-package-lock.json before publishing.";
        return SLANG_FAIL;
    }
    for (const auto& package : lock.packages)
    {
        if (!isLocalOverrideLockedPackage(package))
            continue;
        outError = String("Package lock requires local override '") + package.name +
                   "' and is not portable.";
        return SLANG_FAIL;
    }

    List<String> warnings;
    SLANG_RETURN_ON_FAIL(validateLockedWorkspaceExclusions(manifest, lock, outError));
    SLANG_RETURN_ON_FAIL(refreshAndValidateUpstreamResolvedProject(projectRoot, lock, outError));
    SLANG_RETURN_ON_FAIL(validateWorkspaceResolvedProject(
        projectRoot,
        manifest,
        lock,
        localPackages,
        outError,
        &warnings));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    fprintf(stdout, "Package is valid and suitable for sharing.\n");
    return SLANG_OK;
}

static SlangResult _validateNamedPackage(
    const String& projectRoot,
    const String& name,
    String& outError)
{
    Manifest rootManifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, rootManifest, outError));
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    Index packageIndex = findLockedPackageIndex(lock, name);
    if (packageIndex < 0)
    {
        outError = String("Package is not present in the lock file: ") + name;
        return SLANG_FAIL;
    }
    LockFile packageLock;
    packageLock.packages.add(lock.packages[packageIndex]);
    SLANG_RETURN_ON_FAIL(
        refreshAndValidateUpstreamResolvedProject(projectRoot, packageLock, outError));
    SLANG_RETURN_ON_FAIL(_validateLockedPackagePublishable(
        projectRoot,
        rootManifest,
        lock,
        localPackages,
        lock.packages[packageIndex],
        outError));
    fprintf(stdout, "Package '%s' is valid and suitable for sharing.\n", name.getBuffer());
    return SLANG_OK;
}

static SlangResult _validateAllLockedPackages(const String& projectRoot, String& outError)
{
    Manifest rootManifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, rootManifest, outError));
    LockFile lock;
    String lockPath = Path::combine(projectRoot, kLockFileName);
    if (!File::exists(lockPath))
    {
        if (rootManifest.dependencies.getCount())
        {
            outError = "Package dependencies require slang-package-lock.json.";
            return SLANG_FAIL;
        }
        fprintf(stdout, "No locked packages to validate.\n");
        return SLANG_OK;
    }
    SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
    SLANG_RETURN_ON_FAIL(refreshAndValidateUpstreamResolvedProject(projectRoot, lock, outError));
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    List<String> failures;
    for (const auto& package : lock.packages)
    {
        String packageError;
        if (SLANG_SUCCEEDED(_validateLockedPackagePublishable(
                projectRoot,
                rootManifest,
                lock,
                localPackages,
                package,
                packageError)))
        {
            continue;
        }
        failures.add(package.name + ": " + packageError);
    }
    if (failures.getCount())
    {
        StringBuilder message;
        message << "Locked packages failed sharing checks:";
        for (const auto& failure : failures)
            message << "\n  " << failure;
        outError = message.produceString();
        return SLANG_FAIL;
    }
    fprintf(stdout, "Validated %lld locked package(s).\n", (long long)lock.packages.getCount());
    return SLANG_OK;
}

/// Report lock, checkout, and graph readiness, with extra lines only when something is dirty.
///
/// Like `git status`, reportable drift is data rather than command failure. This function fails
/// only when the root manifest, an existing lock, or workspace-local JSON cannot be read and
/// parsed well enough to produce a report. The header uses `incomplete` when the lock or Git pins
/// are missing, and `not buildable` only after those trees are present and the source check fails.
SlangResult getWorkspaceStatusReport(const String& projectRoot, String& outReport, String& outError)
{
    outReport = String();
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));

    String lockPath = Path::combine(projectRoot, kLockFileName);
    LockFile lock;
    bool hasLock = File::exists(lockPath);
    if (hasLock)
        SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));

    StringBuilder observations;
    List<String> observationFacts;
    auto addFact = [&](const String& fact,
                       const String& useCommand = String(),
                       const String& useTail = String())
    {
        observationFacts.add(fact);
        observations << "  " << fact << "\n";
        if (useCommand.getLength() == 0)
            return;
        observations << "    use '" << useCommand << "'";
        if (useTail.getLength())
            observations << useTail;
        observations << "\n";
    };

    String issue;
    bool lockMatchesManifest = false;
    bool reportedLockDrift = false;
    if (!hasLock)
    {
        if (manifest.dependencies.getCount())
        {
            addFact("no slang-package-lock.json", "slang package fetch");
            reportedLockDrift = true;
        }
        if (localPackages.getCount())
        {
            addFact("slang-package-overlay.json has no lock");
            reportedLockDrift = true;
        }
    }
    else
    {
        lockMatchesManifest = SLANG_SUCCEEDED(
            _validateLockAgainstManifest(projectRoot, manifest, lock, localPackages, issue));
        if (!lockMatchesManifest)
        {
            addFact(issue);
            reportedLockDrift = true;
        }
        issue = String();
        if (SLANG_FAILED(_validateLocalPackages(projectRoot, manifest, lock, localPackages, issue)))
        {
            addFact(issue);
            reportedLockDrift = true;
        }
        issue = String();
        if (SLANG_FAILED(validateCachedResolvedProject(projectRoot, lock, issue)))
        {
            addFact(issue, "slang package fetch");
        }
    }

    // Find the tool-owned checkouts that are absent before inspecting anything inside them.
    // Reading a dependency's own `slang-package.json` and asking Git about its checkout both
    // fail for an absent directory, and those failures would only restate the absence -- one as a
    // missing JSON file, the other as Git refusing to run in a directory that does not exist.
    List<String> unmaterializedNames;
    for (const auto& package : lock.packages)
    {
        if (findActiveLocalPackageIndex(localPackages, package.name) >= 0 ||
            package.path.getLength())
            continue;
        String packageRoot =
            Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest), package.name);
        SlangPathType pathType;
        if (SLANG_FAILED(Path::getPathType(packageRoot, &pathType)) ||
            pathType != SLANG_PATH_TYPE_DIRECTORY)
            unmaterializedNames.add(package.name);
    }

    if (unmaterializedNames.getCount())
    {
        StringBuilder detail;
        detail << "missing under '" << getWorkspaceDepsDirectory(manifest) << "/': ";
        for (Index i = 0; i < unmaterializedNames.getCount(); ++i)
            detail << (i ? ", " : "") << unmaterializedNames[i];
        addFact(detail.produceString(), "slang package fetch");
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
            addFact(package.name + ": not a git checkout", "slang package fetch --clean");
            continue;
        }
        if (origin != package.git)
        {
            addFact(package.name + ": different origin", "slang package fetch --clean");
            continue;
        }

        GitWorkingTreeStatus gitStatus;
        issue = String();
        if (SLANG_FAILED(getWorkingTreeStatus(packageRoot, package.commit, gitStatus, issue)))
        {
            addFact(package.name + ": " + issue.trim());
            continue;
        }
        if (gitStatus.changedFileCount || gitStatus.commitsAhead || gitStatus.commitsBehind ||
            gitStatus.stashCount || gitStatus.headCommit != package.commit)
        {
            addFact(
                package.name + ": " + _describeDirtyCheckout(gitStatus, package.commit),
                String("slang package edit ") + package.name,
                " to keep, or 'slang package fetch --clean' to reset");
        }
    }

    for (const auto& package : localPackages)
    {
        if (isInPlaceLocalPackage(manifest, package))
        {
            if (package.enabled)
                addFact(package.name + ": edited");
            else
                addFact(
                    package.name + ": in-place override disabled",
                    String("slang package override enable ") + package.name);
        }
        else if (package.enabled)
            addFact(package.name + ": override at " + package.path);
    }

    List<String> buildWarnings;
    issue = String();
    const bool needsLock = manifest.dependencies.getCount() != 0 || localPackages.getCount() != 0;
    const bool canCheckBuildability =
        unmaterializedNames.getCount() == 0 && (hasLock || !needsLock);
    const bool isBuildable =
        canCheckBuildability &&
        SLANG_SUCCEEDED(validateBuildableProject(projectRoot, issue, &buildWarnings));
    if (canCheckBuildability && !isBuildable)
    {
        bool alreadyReported = false;
        for (const auto& fact : observationFacts)
        {
            if (issue.getUnownedSlice().indexOf(fact.getUnownedSlice()) >= 0 ||
                fact.getUnownedSlice().indexOf(issue.getUnownedSlice()) >= 0)
            {
                alreadyReported = true;
                break;
            }
        }
        if (!alreadyReported &&
            !(reportedLockDrift &&
              (issue.getUnownedSlice().indexOf(UnownedStringSlice("lock")) >= 0 ||
               issue.getUnownedSlice().indexOf(UnownedStringSlice("Lock")) >= 0)))
            addFact(issue);
    }
    for (const auto& warning : buildWarnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());

    const char* readiness = "not buildable";
    if (isBuildable)
        readiness = "buildable";
    else if (!canCheckBuildability)
        readiness = "incomplete";

    StringBuilder report;
    report << "Package '" << manifest.name << "': ";
    if (!hasLock)
    {
        if (!needsLock)
            report << "no lock required";
        else
            report << "lock absent";
    }
    else if (lockMatchesManifest)
    {
        report << "lock current, " << lock.packages.getCount()
               << (lock.packages.getCount() == 1 ? " package" : " packages");
    }
    else
    {
        report << "lock stale";
    }
    report << ", " << readiness << ".\n";

    if (observationFacts.getCount())
        report << observations;
    outReport = report.produceString();
    return SLANG_OK;
}

static SlangResult _status(const String& projectRoot, String& outError)
{
    String report;
    SLANG_RETURN_ON_FAIL(getWorkspaceStatusReport(projectRoot, report, outError));
    fprintf(stdout, "%s", report.getBuffer());
    return SLANG_OK;
}

static String _describeDependencyRequirement(const Dependency& dependency)
{
    if (dependency.path.getLength())
        return String("path ") + dependency.path + " as " + dependency.as;
    if (dependency.version.getLength())
        return String("version ") + dependency.version;
    return String("ref ") + dependency.ref +
           (dependency.as.getLength() ? String(" as ") + dependency.as : String());
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
    const List<Manifest>& graphManifests,
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
    _getSortedDependencies(graphManifests[packageIndex].dependencies, children);
    for (const auto child : children)
        _printDependencyTree(lock, graphManifests, *child, prefix + "  ", ioExpanded);
}

static SlangResult _loadLockGraphManifests(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    List<Manifest>& outManifests,
    String& outError)
{
    outManifests.clear();
    outManifests.setCount(lock.packages.getCount());
    for (Index i = 0; i < lock.packages.getCount(); ++i)
    {
        SLANG_RETURN_ON_FAIL(loadLockedPackageGraphManifest(
            projectRoot,
            manifest,
            lock.packages[i],
            localPackages,
            outManifests[i],
            outError));
    }
    return SLANG_OK;
}

static SlangResult _tree(const String& projectRoot, String& outError)
{
    Manifest manifest;
    LockFile lock;
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _validateLockAgainstManifest(projectRoot, manifest, lock, localPackages, outError));
    List<Manifest> graphManifests;
    SLANG_RETURN_ON_FAIL(_loadLockGraphManifests(
        projectRoot,
        manifest,
        lock,
        localPackages,
        graphManifests,
        outError));

    fprintf(stdout, "%s\n", manifest.name.getBuffer());
    List<const Dependency*> dependencies;
    _getSortedDependencies(manifest.dependencies, dependencies);
    List<String> expanded;
    for (const auto dependency : dependencies)
        _printDependencyTree(lock, graphManifests, *dependency, "  ", expanded);
    if (!dependencies.getCount())
        fprintf(stdout, "  (no dependencies)\n");
    fprintf(stdout, "(*) dependency subtree already shown\n");
    return SLANG_OK;
}

static void _printWhyPaths(
    const LockFile& lock,
    const List<Manifest>& graphManifests,
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
        _printWhyPaths(
            lock,
            graphManifests,
            graphManifests[packageIndex].dependencies,
            targetName,
            nextPath,
            ioStack,
            ioPathCount);
        ioStack.removeLast();
    }
}

static SlangResult _why(const String& projectRoot, const String& name, String& outError)
{
    Manifest manifest;
    LockFile lock;
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _validateLockAgainstManifest(projectRoot, manifest, lock, localPackages, outError));
    if (findLockedPackageIndex(lock, name) < 0)
    {
        outError = String("Package is not present in the lock: ") + name;
        return SLANG_FAIL;
    }
    List<Manifest> graphManifests;
    SLANG_RETURN_ON_FAIL(_loadLockGraphManifests(
        projectRoot,
        manifest,
        lock,
        localPackages,
        graphManifests,
        outError));

    fprintf(stdout, "Dependency paths to '%s':\n", name.getBuffer());
    List<String> stack;
    Index pathCount = 0;
    _printWhyPaths(
        lock,
        graphManifests,
        manifest.dependencies,
        name,
        manifest.name,
        stack,
        pathCount);
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

    String lockPath = Path::combine(projectRoot, kLockFileName);
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
        Manifest packageManifest;
        SLANG_RETURN_ON_FAIL(loadLockedPackageGraphManifest(
            projectRoot,
            manifest,
            package,
            localPackages,
            packageManifest,
            outError));
        for (const auto& exportPath : packageManifest.exports)
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

/// Return whether a tool-owned Git checkout directory is absent.
///
/// Path-only rows and active overrides are skipped: those trees are not fetch's to
/// create. A missing `deps/NAME` for a Git pin is the case where build can invoke fetch without
/// writing the lock. A directory that is present but not at the locked commit is left to fetch
/// or fetch --clean; build does not try to replace it.
static bool _lockedGitCheckoutIsMissing(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages)
{
    String depsRoot = Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest));
    for (const auto& package : lock.packages)
    {
        if (findActiveLocalPackageIndex(localPackages, package.name) >= 0 ||
            !isGitBackedLockedPackage(package) || package.path.getLength())
        {
            continue;
        }
        String destination = Path::combine(depsRoot, package.name);
        SlangPathType pathType;
        if (SLANG_FAILED(Path::getPathType(destination, &pathType)) ||
            pathType != SLANG_PATH_TYPE_DIRECTORY)
        {
            return true;
        }
    }
    return false;
}

/// Copy exported source under `build/bundle/source`. With the experimental opt-in, also compile
/// enabled `.slang-module` output and host executables into explicitly marked directories.
///
/// If a locked Git checkout is missing, this runs fetch first (without `--clean`). If there is no
/// lock and the manifest has dependencies, it also runs fetch, which runs update with `--yes` so
/// a first clone can `slang package build` without a prompt. An existing lock is never rewritten.
/// Each of those hand-offs prints why it is running the inner command.
static SlangResult _build(
    const String& projectRoot,
    bool experimental,
    bool skipValidate,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));

    String lockPath = Path::combine(projectRoot, kLockFileName);
    bool ranFetch = false;
    if (File::exists(lockPath))
    {
        LockFile lock;
        List<LocalPackage> localPackages;
        SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
        SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
        if (_lockedGitCheckoutIsMissing(projectRoot, manifest, lock, localPackages))
        {
            _announceSubcommand("a locked Git checkout is missing", "slang package fetch");
            SLANG_RETURN_ON_FAIL(_fetch(projectRoot, false, false, skipValidate, outError));
            ranFetch = true;
        }
    }
    else if (manifest.dependencies.getCount())
    {
        _announceSubcommand("slang-package-lock.json is missing", "slang package fetch");
        SLANG_RETURN_ON_FAIL(_fetch(projectRoot, false, true, skipValidate, outError));
        ranFetch = true;
    }

    List<PrimaryModule> primaryModules;
    List<ExportedSourceFile> sourceFiles;
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(validateBuildableProject(
        projectRoot,
        outError,
        &warnings,
        &primaryModules,
        &sourceFiles,
        skipValidate));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    if (skipValidate && !ranFetch)
        _warnSkippedSourceValidation();

    const bool buildModules = experimental && manifest.workspace.bundle.modules;
    const bool buildHost = experimental && hasHostExecutables(manifest);

    List<String> executableSources;
    if (buildHost)
    {
        for (const auto& executableName : manifest.build.host.executables)
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
        for (Index i = 0; i < manifest.build.host.executables.getCount(); ++i)
        {
            String executablePath =
                _getExecutableOutputPath(projectRoot, manifest, manifest.build.host.executables[i]);
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

/// Select the configured executable named by the first argument, or the workspace default.
static void _selectHostExecutable(
    const Manifest& manifest,
    int argumentCount,
    const char* const* arguments,
    String& outExecutableName,
    int& outArgumentIndex)
{
    outExecutableName = manifest.build.host.defaultExecutable;
    outArgumentIndex = 0;
    if (argumentCount > 0 && isHostExecutableName(manifest, arguments[0]))
    {
        outExecutableName = arguments[0];
        outArgumentIndex = 1;
    }
}

/// Run a configured workspace primary from the distributable source bundle with `slangi`.
static SlangResult _runSource(
    const String& projectRoot,
    int argumentCount,
    const char* const* arguments,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    if (!hasHostExecutables(manifest))
    {
        outError =
            "The workspace does not configure a host executable. Add 'build.host.executables' to "
            "slang-package.json and run 'slang package build'.";
        return SLANG_FAIL;
    }
    if (!manifest.workspace.bundle.source)
    {
        outError =
            "Source run requires 'workspace.bundle.source'. Enable it and run 'slang package "
            "build'.";
        return SLANG_FAIL;
    }

    String executableName;
    int argumentIndex;
    _selectHostExecutable(manifest, argumentCount, arguments, executableName, argumentIndex);

    String bundledSourcePath = Path::combine(
        Path::combine(
            Path::combine(projectRoot, getWorkspaceBuildDirectory(manifest)),
            "bundle",
            "source"),
        executableName + ".slang");
    if (!File::exists(bundledSourcePath))
    {
        outError = String("The configured source entry has not been built: ") + bundledSourcePath +
                   ". Source run requires that primary at an export root; move it there if needed, "
                   "then run 'slang package build'.";
        return SLANG_FAIL;
    }

    // `slangi` adds the input file's parent to its module search paths only when the argument has a
    // directory component. Preserve that signal even for a source bundle rooted in the cwd.
    if (!Path::getParentDirectory(bundledSourcePath).getLength())
        bundledSourcePath = Path::combine(".", bundledSourcePath);

    String slangiPath;
    SLANG_RETURN_ON_FAIL(_findSiblingTool("slangi", slangiPath, outError));
    List<String> interpreterArguments;
    interpreterArguments.add(bundledSourcePath);
    for (int i = argumentIndex; i < argumentCount; ++i)
        interpreterArguments.add(arguments[i]);
    return _runStreamingSiblingTool(slangiPath, interpreterArguments, outError);
}

/// Run an existing native executable configured by the workspace `build.host` section.
static SlangResult _runBinary(
    const String& projectRoot,
    int argumentCount,
    const char* const* arguments,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    if (!hasHostExecutables(manifest))
    {
        outError =
            "The workspace does not configure a host executable. Add 'build.host.executables' to "
            "slang-package.json and run 'slang package --experimental build'.";
        return SLANG_FAIL;
    }

    String executableName;
    int argumentIndex;
    _selectHostExecutable(manifest, argumentCount, arguments, executableName, argumentIndex);

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

void getRegisteredApplicationOpenCommand(const String& path, CommandLine& outCommand)
{
    outCommand = CommandLine();
#if SLANG_WINDOWS_FAMILY
    // `start` is a cmd built-in. The empty argument is the window title; without it a quoted path
    // is taken as the title and the file is never opened.
    outCommand.setExecutableLocation(ExecutableLocation(ExecutableLocation::Type::Name, "cmd.exe"));
    outCommand.addArg("/c");
    outCommand.addArg("start");
    outCommand.addArg("");
    outCommand.addArg(path);
#elif defined(__APPLE__)
    outCommand.setExecutableLocation(ExecutableLocation(ExecutableLocation::Type::Name, "open"));
    outCommand.addArg(path);
#else
    outCommand.setExecutableLocation(
        ExecutableLocation(ExecutableLocation::Type::Name, "xdg-open"));
    outCommand.addArg(path);
#endif
}

static SlangResult _openPathWithRegisteredApplication(const String& path, String& outError)
{
    CommandLine commandLine;
    getRegisteredApplicationOpenCommand(path, commandLine);
    RefPtr<Process> process;
    if (SLANG_FAILED(Process::create(
            commandLine,
            Process::Flag::DisableStdErrRedirection | Process::Flag::UnreadableStdin,
            process)))
    {
        outError = String("Cannot launch the registered application for '") + path + "'.";
        return SLANG_FAIL;
    }
    // Do not wait. `open` and `cmd /c start` return at once; some `xdg-open` implementations keep
    // running for as long as the viewer does, and blocking on that would stall `docs`.
    return SLANG_OK;
}

/// Open `build/docs/index.md` with the host's registered Markdown handler, or print its path.
static SlangResult _docs(const String& projectRoot, bool printOnly, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    String indexPath = Path::combine(
        Path::combine(projectRoot, getWorkspaceBuildDirectory(manifest), "docs"),
        "index.md");
    if (!File::exists(indexPath))
    {
        outError = String("Generated documentation index is missing: ") + indexPath +
                   ". Run 'slang package build' first.";
        return SLANG_FAIL;
    }
    String canonicalPath = indexPath;
    Path::getCanonical(indexPath, canonicalPath);
    if (printOnly)
    {
        fprintf(stdout, "%s\n", canonicalPath.getBuffer());
        return SLANG_OK;
    }
    SLANG_RETURN_ON_FAIL(_openPathWithRegisteredApplication(canonicalPath, outError));
    fprintf(stdout, "Opened '%s'.\n", canonicalPath.getBuffer());
    return SLANG_OK;
}

static SlangResult _registerLocalPackage(
    const String& projectRoot,
    const String& name,
    const String& path,
    const String& as,
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
    // Local file changes are not an obstacle here, they are the reason to run this command: the
    // user has work in `deps/NAME` and wants the tool to stop managing that tree. Fetch and update
    // refuse to replace a dirty checkout, so requiring a pristine tree would leave the one
    // command that preserves the work unavailable in exactly the state that needs it.
    //
    // What must still hold is that the tree is the repository the lock names. `edit` turns that
    // existing checkout into an in-place override; it is not a way to substitute an unrelated
    // repository under the workspace-owned dependency path.
    if (isGitBackedLockedPackage(*package))
    {
        String origin;
        if (SLANG_FAILED(getRepositoryOrigin(destination, origin, outError)))
        {
            outError = String("Dependency checkout is not a Git repository: ") + destination;
            appendErrorAdvice(
                outError,
                "Run 'slang package fetch --clean' to restore it from the lock, then edit it.");
            return SLANG_FAIL;
        }
        if (origin != package->git)
        {
            outError =
                String("Dependency checkout is not the repository the lock names: ") + destination;
            appendErrorAdvice(
                outError,
                String("The lock selects ") + package->git +
                    ". Run 'slang package fetch --clean' to restore that repository, or use an "
                    "override if this directory should participate in resolution.");
            return SLANG_FAIL;
        }
    }
    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(_registerLocalPackage(
        projectRoot,
        name,
        destination,
        package->version,
        localPackages,
        outError));
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    fprintf(stdout, "Package '%s' is now editable.\n", name.getBuffer());
    return SLANG_OK;
}

/// Replace an in-place override with a committed Git pin at its current `HEAD`.
///
/// The manifest records the pin so a later update cannot silently select another release; the lock
/// records the exact commit that fetch installs. Omit `requestedRef` to freeze `HEAD` as a commit
/// or as a unique release tag that points at `HEAD`. Pass `requestedRef` to keep following that
/// branch or tag. Omit `requestedVersion` to derive `as` from the nearest `vMAJOR.MINOR.PATCH` tag
/// reachable from `HEAD`.
static SlangResult _adoptInPlaceOverride(
    const String& projectRoot,
    const String& name,
    const String& requestedVersion,
    const String& requestedRef,
    bool assumeYes,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
    Index dependencyIndex = _findDependencyIndex(manifest, name);
    if (dependencyIndex < 0 || manifest.dependencies[dependencyIndex].path.getLength())
    {
        outError = String("unedit --adopt requires a direct Git dependency: ") + name;
        return SLANG_FAIL;
    }

    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readProjectLock(projectRoot, lock, outError));
    Index lockedIndex = findLockedPackageIndex(lock, name);
    if (lockedIndex < 0 || !isGitBackedLockedPackage(lock.packages[lockedIndex]))
    {
        outError = String("Package is not a locked Git dependency: ") + name;
        return SLANG_FAIL;
    }

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    Index localIndex = findLocalPackageIndex(localPackages, name);
    if (localIndex < 0 || !isInPlaceLocalPackage(manifest, localPackages[localIndex]))
    {
        outError = String("Package is not editable: ") + name;
        return SLANG_FAIL;
    }

    String checkout;
    SLANG_RETURN_ON_FAIL(
        getLocalPackageRoot(projectRoot, localPackages[localIndex], checkout, outError));
    String origin;
    SLANG_RETURN_ON_FAIL(getRepositoryOrigin(checkout, origin, outError));
    if (origin != lock.packages[lockedIndex].git)
    {
        outError = String("Editable checkout is not the repository the lock names: ") + name;
        return SLANG_FAIL;
    }

    String headCommit;
    SLANG_RETURN_ON_FAIL(getRepositoryHeadCommit(checkout, headCommit, outError));
    GitWorkingTreeStatus status;
    SLANG_RETURN_ON_FAIL(getWorkingTreeStatus(checkout, headCommit, status, outError));
    if (status.changedFileCount || status.stashCount)
    {
        outError = String("unedit --adopt requires committed files and no stashes: ") + checkout;
        return SLANG_FAIL;
    }

    Manifest localManifest;
    SLANG_RETURN_ON_FAIL(
        readLocalPackageManifest(projectRoot, localPackages[localIndex], localManifest, outError));
    SLANG_RETURN_ON_FAIL(
        validateLockedPackageManifest(lock.packages[lockedIndex], localManifest, outError));

    String ref = headCommit;
    String version = requestedVersion;
    if (requestedRef.getLength())
    {
        if (isGitObjectId(requestedRef))
        {
            outError = String("unedit --adopt --ref cannot be a commit ID: ") + requestedRef +
                       ". Omit --ref to freeze HEAD.";
            return SLANG_FAIL;
        }
        String resolvedCommit;
        SLANG_RETURN_ON_FAIL(
            resolveLocalRevision(checkout, requestedRef, resolvedCommit, outError));
        if (resolvedCommit != headCommit)
        {
            outError = String("unedit --adopt --ref '") + requestedRef +
                       "' does not point at HEAD. Check out that ref first.";
            return SLANG_FAIL;
        }
        ref = requestedRef;
    }
    if (version.getLength())
    {
        SemanticVersion ignoredVersion;
        SLANG_RETURN_ON_FAIL(parseExactVersion(version, ignoredVersion, outError));
    }
    else
    {
        SemanticVersion taggedVersion;
        bool foundTag = false;
        String headTag;
        SLANG_RETURN_ON_FAIL(
            findVersionTagAtHead(checkout, headTag, taggedVersion, foundTag, outError));
        if (foundTag)
        {
            version = formatExactVersion(taggedVersion);
            if (!requestedRef.getLength())
                ref = headTag;
        }
        else
        {
            String nearestTag;
            SLANG_RETURN_ON_FAIL(findNearestReleaseTag(
                checkout,
                headCommit,
                nearestTag,
                taggedVersion,
                foundTag,
                outError));
            if (!foundTag)
            {
                outError = String("Editable checkout HEAD has no semantic-version tag in its "
                                  "history: ") +
                           name + ". Pass --as VERSION to adopt this commit.";
                return SLANG_FAIL;
            }
            version = formatExactVersion(taggedVersion);
        }
    }

    Dependency& dependency = manifest.dependencies[dependencyIndex];
    dependency.version = String();
    dependency.ref = ref;
    dependency.as = version;

    LockedPackage& package = lock.packages[lockedIndex];
    package.path = String();
    package.ref = ref;
    package.version = version;
    package.commit = headCommit;

    localPackages.removeAt(localIndex);
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(validateWorkspaceResolvedProject(
        projectRoot,
        manifest,
        lock,
        localPackages,
        outError,
        &warnings));

    fprintf(
        stdout,
        "Will pin dependency '%s' to %s (%s) as %s.\n",
        name.getBuffer(),
        ref.getBuffer(),
        headCommit.getBuffer(),
        version.getBuffer());
    bool approved = false;
    SLANG_RETURN_ON_FAIL(_confirmApply(
        assumeYes,
        requestedRef.getLength() ? "Adopt this ref in the manifest and lock?"
                                 : "Adopt this commit in the manifest and lock?",
        approved,
        outError));
    if (!approved)
        return SLANG_OK;

    SLANG_RETURN_ON_FAIL(_writeValidatedProjectManifest(projectRoot, manifest, outError));
    SLANG_RETURN_ON_FAIL(writeLockFile(Path::combine(projectRoot, kLockFileName), lock, outError));
    SLANG_RETURN_ON_FAIL(writeProjectLocalPackages(projectRoot, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    fprintf(
        stdout,
        "Adopted '%s' at %s as %s.\n",
        name.getBuffer(),
        ref.getBuffer(),
        version.getBuffer());
    return SLANG_OK;
}

static SlangResult _unedit(
    const String& projectRoot,
    const String& name,
    bool allowClean,
    bool assumeYes,
    String& outError)
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
    if (localIndex < 0 || !isInPlaceLocalPackage(manifest, localPackages[localIndex]))
    {
        outError = String("Package is not editable: ") + name;
        return SLANG_FAIL;
    }
    if (package->path.getLength())
    {
        outError = String("The lock still records an in-place override for package: ") + name;
        appendErrorAdvice(
            outError,
            String("To keep the committed checkout, run 'slang package unedit ") + name +
                " --adopt' (and pass '--as VERSION' when no release tag is in history). To return "
                "to the "
                "published graph, run 'slang package override disable " +
                name + "', then 'slang package update' and 'slang package unedit " + name + "'.");
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
    String headCommit;
    SLANG_RETURN_ON_FAIL(getRepositoryHeadCommit(destination, headCommit, outError));
    GitWorkingTreeStatus status;
    SLANG_RETURN_ON_FAIL(getWorkingTreeStatus(destination, headCommit, status, outError));
    const bool hasLocalState = status.changedFileCount != 0 || status.stashCount != 0;
    const bool headMoved = status.headCommit != package->commit;
    if ((hasLocalState || headMoved) && !allowClean)
    {
        outError =
            hasLocalState
                ? String("Editable checkout has uncommitted files or stashes: ") + destination
                : String("Editable checkout HEAD differs from the locked commit: ") + destination;
        String advice =
            hasLocalState
                ? String("Commit the files you want to keep and apply any stashes, then run "
                         "'slang package unedit ") +
                      name +
                      " --adopt' (and pass '--as VERSION' when no release tag is in history). To "
                      "discard all "
                      "local state instead, run 'slang package unedit " +
                      name + " --clean'."
                : String("To keep these commits, run 'slang package unedit ") + name +
                      " --adopt' (and pass '--as VERSION' when no release tag is in history). To "
                      "discard them "
                      "and restore the locked commit, run 'slang package unedit " +
                      name + " --clean'.";
        appendErrorAdvice(outError, advice);
        return SLANG_FAIL;
    }
    if (allowClean)
    {
        const bool needsRestore = hasLocalState || headMoved;
        if (needsRestore)
        {
            bool approved = false;
            SLANG_RETURN_ON_FAIL(_confirmApply(
                assumeYes,
                "Discard this editable checkout state and restore the locked commit?",
                approved,
                outError));
            if (!approved)
                return SLANG_OK;

            bool didMaterialize = false;
            String cachePath =
                Path::combine(Path::combine(projectRoot, ".slang", "cache"), package->name);
            // The state that triggered this confirmation makes materialization replace the whole
            // repository, so no surviving ref can move. If the checkout becomes clean before the
            // mutation, keep ref moves disallowed because they were not listed in this prompt.
            SLANG_RETURN_ON_FAIL(materializeLockedRevision(
                package->git,
                package->commit,
                package->commit,
                destination,
                true,
                false,
                didMaterialize,
                outError,
                cachePath));
        }
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

/// Return whether `path` is the workspace checkout for `name`.
///
/// `edit color-encoding` and `override add color-encoding deps/color-encoding` name the same local
/// registration, so callers use this role check instead of storing a second edit representation.
static SlangResult _isWorkspaceDependencyCheckout(
    const String& projectRoot,
    const Manifest& manifest,
    const String& name,
    const String& path,
    bool& outIsCheckout,
    String& outRelativePath,
    String& outError)
{
    outIsCheckout = false;
    outRelativePath = String();
    String inputPath = Path::isAbsolute(path) ? path : Path::combine(projectRoot, path);
    String canonicalPath;
    SlangPathType type;
    if (SLANG_FAILED(Path::getPathType(inputPath, &type)) || type != SLANG_PATH_TYPE_DIRECTORY ||
        SLANG_FAILED(Path::getCanonical(inputPath, canonicalPath)))
    {
        outError = String("Local package directory does not exist: ") + path;
        return SLANG_FAIL;
    }
    String expected;
    if (SLANG_FAILED(Path::getCanonical(
            Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest), name),
            expected)))
    {
        outError = String("Cannot canonicalize dependency checkout: ") + name;
        return SLANG_FAIL;
    }
    outIsCheckout = canonicalPath == expected;
    if (outIsCheckout)
        outRelativePath = Path::getRelativePath(projectRoot, canonicalPath);
    return SLANG_OK;
}

static SlangResult _overrideAdd(
    const String& projectRoot,
    const String& name,
    const String& path,
    const String& as,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
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
    Index localIndex = findLocalPackageIndex(localPackages, name);
    if (localIndex >= 0)
    {
        bool isCheckout = false;
        String relativePath;
        SLANG_RETURN_ON_FAIL(_isWorkspaceDependencyCheckout(
            projectRoot,
            manifest,
            name,
            path,
            isCheckout,
            relativePath,
            outError));
        if (!isInPlaceLocalPackage(manifest, localPackages[localIndex]) || !isCheckout)
        {
            outError = String("Package already has a registered local tree: ") + name;
            return SLANG_FAIL;
        }
        // `edit` already registered this exact in-place override. Re-adding it may supply a new
        // effective version, but it does not create another local-package representation.
        localPackages[localIndex].path = relativePath;
        localPackages[localIndex].as = providedVersion;
        localPackages[localIndex].enabled = true;
        SLANG_RETURN_ON_FAIL(writeProjectLocalPackages(projectRoot, localPackages, outError));
        SLANG_RETURN_ON_FAIL(
            _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
        fprintf(
            stdout,
            "Package '%s' is now an override at '%s' as %s. Run 'slang package update' to adopt "
            "its manifest.\n",
            name.getBuffer(),
            relativePath.getBuffer(),
            providedVersion.getBuffer());
        return SLANG_OK;
    }

    SLANG_RETURN_ON_FAIL(
        _registerLocalPackage(projectRoot, name, path, providedVersion, localPackages, outError));
    SLANG_RETURN_ON_FAIL(
        _writeValidatedSearchPathsAfterLocalChange(projectRoot, lock, localPackages, outError));
    fprintf(
        stdout,
        "Package '%s' now uses '%s'. Run 'slang package update' to adopt its manifest.\n",
        name.getBuffer(),
        path.getBuffer());
    return SLANG_OK;
}

static SlangResult _overrideRemove(const String& projectRoot, const String& name, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProjectManifest(projectRoot, manifest, outError));
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
    if (isInPlaceLocalPackage(manifest, localPackages[localIndex]))
    {
        outError = String("Package is editable; use 'slang package unedit ") + name + "'.";
        return SLANG_FAIL;
    }
    if (packageIndex >= 0 && lock.packages[packageIndex].path.getLength())
    {
        outError = String("The lock still points at this local package. Run "
                          "'slang package update' before 'override remove'.");
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
    if (localIndex < 0)
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
        bool ignoreOverrides = false;
        bool allowClean = false;
        bool dryRun = false;
        bool minimal = false;
        bool assumeYes = false;
        bool skipValidate = false;
        bool offline = false;
        for (int i = 2; i < argc; ++i)
        {
            String flag = argv[i];
            if (flag == "--ignore-overrides")
                ignoreOverrides = true;
            else if (flag == "--clean")
                allowClean = true;
            else if (flag == "--dry-run")
                dryRun = true;
            else if (flag == "--minimal")
                minimal = true;
            else if (flag == "--offline")
                offline = true;
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
            ignoreOverrides,
            allowClean,
            dryRun,
            minimal,
            assumeYes,
            skipValidate,
            offline,
            outError);
    }
    if (command == "validate")
    {
        bool all = false;
        String name;
        for (int i = 2; i < argc; ++i)
        {
            String argument = argv[i];
            if (argument == "--all")
                all = true;
            else if (argument.startsWith("-"))
            {
                outError = String("Unknown validate option: ") + argument;
                return SLANG_FAIL;
            }
            else if (name.getLength())
            {
                outError = "validate accepts at most one package name.";
                return SLANG_FAIL;
            }
            else
                name = argument;
        }
        if (all && name.getLength())
        {
            outError = "validate --all cannot be combined with a package name.";
            return SLANG_FAIL;
        }
        if (all)
            return _validateAllLockedPackages(projectRoot, outError);
        if (name.getLength())
            return _validateNamedPackage(projectRoot, name, outError);
        return _validate(projectRoot, outError);
    }
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
        bool binary = false;
        int argumentIndex = 2;
        if (argc > argumentIndex && String(argv[argumentIndex]) == "--binary")
        {
            if (!experimental)
            {
                outError = "run --binary requires the global --experimental option.";
                return SLANG_FAIL;
            }
            binary = true;
            ++argumentIndex;
        }
        if (binary)
            return _runBinary(projectRoot, argc - argumentIndex, argv + argumentIndex, outError);
        return _runSource(projectRoot, argc - argumentIndex, argv + argumentIndex, outError);
    }
    if (command == "test" && argc == 2)
        return _test(projectRoot, outError);
    if (command == "docs")
    {
        bool printOnly = false;
        for (int i = 2; i < argc; ++i)
        {
            String flag = argv[i];
            if (flag == "--print")
                printOnly = true;
            else
            {
                outError = String("Unknown docs option: ") + flag;
                return SLANG_FAIL;
            }
        }
        return _docs(projectRoot, printOnly, outError);
    }
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
        if (argc == 3 && String(argv[2]) == "pin")
        {
            outError = "dependency pin requires a package name.";
            return SLANG_FAIL;
        }
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
                               !dependency.path.getLength() && !dependency.version.getLength();
            if (!(validPath || validGitVersion || validGitRef))
            {
                outError = "Dependency add requires exactly one of: --git URL --version RANGE, "
                           "--git URL --ref REF [--as VERSION], or --path PATH --as VERSION.";
                return SLANG_FAIL;
            }
            return _dependencyAdd(projectRoot, dependency, outError);
        }
        if (argc >= 4 && String(argv[2]) == "pin")
        {
            String name = argv[3];
            if (!isValidPackageName(name))
            {
                outError = String("Invalid dependency name: ") + name;
                return SLANG_FAIL;
            }
            String requestedVersion;
            bool hasRequestedVersion = false;
            bool pinCommit = false;
            for (int i = 4; i < argc; ++i)
            {
                String option = argv[i];
                if (option == "--to")
                {
                    if (hasRequestedVersion)
                    {
                        outError = "Duplicate dependency pin option: --to";
                        return SLANG_FAIL;
                    }
                    if (i + 1 >= argc)
                    {
                        outError = "Missing value for dependency option: --to";
                        return SLANG_FAIL;
                    }
                    requestedVersion = argv[++i];
                    hasRequestedVersion = true;
                    if (!requestedVersion.getLength() || requestedVersion.startsWith("-"))
                    {
                        outError = "dependency pin --to requires an exact version.";
                        return SLANG_FAIL;
                    }
                }
                else if (option == "--commit")
                {
                    if (pinCommit)
                    {
                        outError = "Duplicate dependency pin option: --commit";
                        return SLANG_FAIL;
                    }
                    pinCommit = true;
                }
                else
                {
                    outError = String("Unknown dependency pin option: ") + option;
                    return SLANG_FAIL;
                }
            }
            return _dependencyPin(
                projectRoot,
                name,
                requestedVersion,
                hasRequestedVersion,
                pinCommit,
                outError);
        }
        outError = "Invalid dependency command. Use 'dependency add', 'dependency pin', "
                   "'dependency remove', or 'dependency list'.";
        return SLANG_FAIL;
    }
    if (command == "edit" && argc == 3)
        return _edit(projectRoot, argv[2], outError);
    if (command == "unedit")
    {
        if (argc < 3)
        {
            outError = "unedit requires a package name.";
            return SLANG_FAIL;
        }
        bool allowClean = false;
        bool adopt = false;
        bool assumeYes = false;
        String as;
        String ref;
        for (int i = 3; i < argc; ++i)
        {
            String flag = argv[i];
            if (flag == "--clean")
                allowClean = true;
            else if (flag == "--adopt")
                adopt = true;
            else if (flag == "--as" && i + 1 < argc)
                as = argv[++i];
            else if (flag == "--ref" && i + 1 < argc)
                ref = argv[++i];
            else if (flag == "--yes")
                assumeYes = true;
            else
            {
                outError = String("Unknown unedit option: ") + flag;
                return SLANG_FAIL;
            }
        }
        if (allowClean && adopt)
        {
            outError = "unedit --clean cannot be combined with --adopt.";
            return SLANG_FAIL;
        }
        if (as.getLength() && !adopt)
        {
            outError = "unedit --as requires --adopt.";
            return SLANG_FAIL;
        }
        if (ref.getLength() && !adopt)
        {
            outError = "unedit --ref requires --adopt.";
            return SLANG_FAIL;
        }
        if (assumeYes && !allowClean && !adopt)
        {
            outError = "unedit --yes requires --clean or --adopt.";
            return SLANG_FAIL;
        }
        if (adopt)
            return _adoptInPlaceOverride(projectRoot, argv[2], as, ref, assumeYes, outError);
        return _unedit(projectRoot, argv[2], allowClean, assumeYes, outError);
    }
    if (command == "override" && argc == 3 && String(argv[2]) == "list")
        return _listOverrides(projectRoot, outError);
    if (command == "override" && argc == 4 && String(argv[2]) == "enable")
        return _setOverrideEnabled(projectRoot, argv[3], true, outError);
    if (command == "override" && argc == 4 && String(argv[2]) == "disable")
        return _setOverrideEnabled(projectRoot, argv[3], false, outError);
    if (command == "override" && argc == 4 && String(argv[2]) == "remove")
        return _overrideRemove(projectRoot, argv[3], outError);
    if (command == "override" && (argc == 5 || argc == 6) && String(argv[2]) == "add")
        return _overrideAdd(
            projectRoot,
            argv[3],
            argv[4],
            argc == 6 ? String(argv[5]) : String(),
            outError);

    outError = String("Invalid command or arguments. Run '") + argv[0] + " help'.";
    return SLANG_FAIL;
}

String formatCommandError(const String& error)
{
    return String("slang-package: error: ") + error + "\n";
}

int executeFromStartDirectory(const String& startDirectory, int argc, const char* const* argv)
{
    String error;
    String projectRoot;
    if (SLANG_FAILED(Path::getCanonical(startDirectory, projectRoot)))
    {
        error = String("Cannot determine the current directory: ") + startDirectory;
        String transcript = formatCommandError(error);
        fprintf(stderr, "%s", transcript.getBuffer());
        return 1;
    }
    if (_commandRequiresPackageRoot(argc, argv) &&
        SLANG_FAILED(discoverPackageRoot(projectRoot, projectRoot, error)))
    {
        String transcript = formatCommandError(error);
        fprintf(stderr, "%s", transcript.getBuffer());
        return 1;
    }
    if (SLANG_FAILED(executeInDirectory(projectRoot, argc, argv, error)))
    {
        String transcript = formatCommandError(error);
        fprintf(stderr, "%s", transcript.getBuffer());
        return 1;
    }
    return 0;
}

int execute(int argc, const char* const* argv)
{
    return executeFromStartDirectory(".", argc, argv);
}

} // namespace PackageTool
} // namespace Slang
