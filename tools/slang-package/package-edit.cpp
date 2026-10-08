// package-edit.cpp

#include "package-edit.h"

#include "core/slang-io.h"
#include "core/slang-writer.h"
#include "package-git.h"
#include "package-json.h"
#include "package-lock.h"
#include "package-resolver.h"
#include "package-tool.h"

#include <stdio.h>

namespace Slang
{
namespace PackageTool
{

static SlangResult _readProject(const String& projectRoot, Manifest& outManifest, String& outError)
{
    return readManifest(Path::combine(projectRoot, kPackageFileName), outManifest, outError);
}

static SlangResult _readLock(const String& projectRoot, LockFile& outLock, String& outError)
{
    String path = Path::combine(projectRoot, kLockFileName);
    if (!File::exists(path))
    {
        outError = "slang-package-lock.json is missing. Run 'slang package update'.";
        return SLANG_FAIL;
    }
    return readLockFile(path, outLock, outError);
}

static SlangResult _writeLock(const String& projectRoot, const LockFile& lock, String& outError)
{
    return writeLockFile(Path::combine(projectRoot, kLockFileName), lock, outError);
}

static String _checkoutPath(const String& projectRoot, const Manifest& manifest, const String& name)
{
    return Path::combine(Path::combine(projectRoot, getWorkspaceDepsDirectory(manifest)), name);
}

static SlangResult _requireCheckout(const String& checkout, const String& gitURL, String& outError)
{
    SlangPathType type;
    if (SLANG_FAILED(Path::getPathType(checkout, &type)) || type != SLANG_PATH_TYPE_DIRECTORY)
    {
        outError =
            String("Dependency checkout is missing: ") + checkout + ". Run 'slang package fetch'.";
        return SLANG_FAIL;
    }
    String origin;
    SLANG_RETURN_ON_FAIL(getRepositoryOrigin(checkout, origin, outError));
    if (origin != gitURL)
    {
        outError = String("Dependency checkout origin does not match the lock: ") + checkout;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _confirm(const char* prompt, bool& outApproved, String& outError)
{
    outApproved = false;
    if (!FileWriter::isFileConsole(stdin))
    {
        outError = String(prompt) + " requires confirmation in a terminal.";
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

/// Solve `lock` without moving any release other than `targetName`.
///
/// Every other release is held pinned for the solve, then its previous pin flag is written back.
/// `targetPinned` is the flag stored on `targetName` after the solve, so an unpinned `unedit` can
/// land on one version without the solver immediately replacing it.
static SlangResult _resolveFrozen(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const String& targetName,
    bool targetPinned,
    LockFile& outLock,
    String& outError)
{
    LockFile held = lock;
    for (auto& package : held.packages)
    {
        if (package.name == targetName || package.branch.getLength())
            continue;
        package.pinned = true;
    }
    List<RepositoryLocation> remapPackages;
    if (lock.remapIndex.getLength())
        SLANG_RETURN_ON_FAIL(readPackageIndex(lock.remapIndex, remapPackages, outError));
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(resolveDependencies(
        projectRoot,
        manifest,
        outLock,
        outError,
        &warnings,
        nullptr,
        false,
        &held,
        lock.remapIndex.getLength() ? &remapPackages : nullptr));
    outLock.remapIndex = lock.remapIndex;
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    for (auto& package : outLock.packages)
    {
        if (package.name == targetName)
        {
            package.pinned = targetPinned;
            continue;
        }
        Index original = findLockedPackageIndex(lock, package.name);
        if (original >= 0)
            package.pinned = lock.packages[original].pinned;
    }
    return SLANG_OK;
}

static SlangResult _readDependentManifest(
    const String& projectRoot,
    const Manifest& root,
    const LockedPackage& package,
    Manifest& outManifest,
    String& outError)
{
    if (package.branch.getLength())
    {
        String path =
            Path::combine(_checkoutPath(projectRoot, root, package.name), kPackageFileName);
        return readManifest(path, outManifest, outError);
    }
    String checkout = _checkoutPath(projectRoot, root, package.name);
    String cache = Path::combine(Path::combine(projectRoot, ".slang", "cache"), package.name);
    String text;
    String gitError;
    String source;
    if (SLANG_SUCCEEDED(
            readFileAtRevision(checkout, package.commit, kPackageFileName, text, gitError)))
    {
        source = checkout;
    }
    else if (SLANG_SUCCEEDED(
                 readFileAtRevision(cache, package.commit, kPackageFileName, text, gitError)))
    {
        source = cache;
    }
    else
    {
        outError = String("Cannot read manifest for '") + package.name +
                   "' to check constraints. " + gitError;
        return SLANG_FAIL;
    }
    return readManifestText(source + ":" + kPackageFileName, text, outManifest, outError);
}

static SlangResult _requireVersionAccepted(
    const String& projectRoot,
    const Manifest& root,
    const LockFile& lock,
    const String& packageName,
    const PackageVersion& version,
    String& outError)
{
    String versionText = formatExactVersion(version);
    auto checkList = [&](const String& owner, const List<Dependency>& dependencies) -> SlangResult
    {
        for (const auto& dependency : dependencies)
        {
            if (dependency.name != packageName)
                continue;
            if (dependency.version.getLength())
            {
                VersionConstraint constraint;
                SLANG_RETURN_ON_FAIL(parseDependencyConstraint(dependency, constraint, outError));
                if (!constraint.matches(version))
                {
                    outError = String("Version ") + versionText + " of '" + packageName +
                               "' does not satisfy " + dependency.version + " required by " +
                               owner + ".";
                    return SLANG_FAIL;
                }
            }
            if (dependency.as.getLength() && !sameExactRelease(dependency.as, versionText))
            {
                outError = String("Version ") + versionText + " of '" + packageName +
                           "' does not match 'as' version " + dependency.as + " required by " +
                           owner + ".";
                return SLANG_FAIL;
            }
        }
        return SLANG_OK;
    };

    SLANG_RETURN_ON_FAIL(checkList(root.name, root.dependencies));
    for (const auto& package : lock.packages)
    {
        if (package.name == packageName)
            continue;
        Manifest manifest;
        SLANG_RETURN_ON_FAIL(
            _readDependentManifest(projectRoot, root, package, manifest, outError));
        SLANG_RETURN_ON_FAIL(checkList(package.name, manifest.dependencies));
    }
    return SLANG_OK;
}

static SlangResult _findTagRepository(
    const String& projectRoot,
    const Manifest& manifest,
    const LockedPackage& package,
    String& outRepository,
    String& outError)
{
    String checkout = _checkoutPath(projectRoot, manifest, package.name);
    String origin;
    String originError;
    if (SLANG_SUCCEEDED(getRepositoryOrigin(checkout, origin, originError)) &&
        origin == package.git)
    {
        outRepository = checkout;
        return SLANG_OK;
    }
    String cache = Path::combine(Path::combine(projectRoot, ".slang", "cache"), package.name);
    if (SLANG_SUCCEEDED(requirePackageCache(package.git, cache, outError)))
    {
        outRepository = cache;
        return SLANG_OK;
    }
    outError = String("Cannot find a checkout or cache for '") + package.name +
               "' to resolve a release tag.";
    return SLANG_FAIL;
}

static SlangResult _findCanonicalTag(
    const String& repository,
    const PackageVersion& version,
    TagCandidate& outTag,
    String& outError)
{
    List<TagCandidate> tags;
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(listReleaseTagsFromRepository(repository, tags, outError, &warnings));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());
    for (const auto& tag : tags)
    {
        if (tag.version == version)
        {
            outTag = tag;
            return SLANG_OK;
        }
    }
    outError = String("No canonical release tag for ") + formatExactVersion(version) + ".";
    return SLANG_FAIL;
}

SlangResult pinLockedPackage(
    const String& projectRoot,
    const String& name,
    const String& versionText,
    bool hasVersion,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProject(projectRoot, manifest, outError));
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readLock(projectRoot, lock, outError));
    Index index = findLockedPackageIndex(lock, name);
    if (index < 0)
    {
        outError = String("Package is not present in the lock file: ") + name;
        return SLANG_FAIL;
    }
    LockedPackage& package = lock.packages[index];
    if (package.branch.getLength())
    {
        if (hasVersion)
        {
            outError = String("Cannot pass a version while '") + name +
                       "' is edited. The version moves with 'slang package edit " + name +
                       " --advance' or 'slang package unedit " + name + "'.";
            return SLANG_FAIL;
        }
        package.pinned = true;
        SLANG_RETURN_ON_FAIL(_writeLock(projectRoot, lock, outError));
        fprintf(
            stdout,
            "Pinned '%s' on branch '%s'.\n",
            name.getBuffer(),
            package.branch.getBuffer());
        return SLANG_OK;
    }
    if (!hasVersion)
    {
        outError = String("pin requires a version when '") + name + "' is not edited.";
        return SLANG_FAIL;
    }
    PackageVersion version;
    SLANG_RETURN_ON_FAIL(parseExactVersion(versionText, version, outError));
    SLANG_RETURN_ON_FAIL(
        _requireVersionAccepted(projectRoot, manifest, lock, name, version, outError));
    String repository;
    SLANG_RETURN_ON_FAIL(_findTagRepository(projectRoot, manifest, package, repository, outError));
    TagCandidate tag;
    SLANG_RETURN_ON_FAIL(_findCanonicalTag(repository, version, tag, outError));

    package.version = formatExactVersion(version);
    package.ref = tag.ref;
    package.commit = tag.commit;
    package.pinned = true;
    LockFile resolved;
    SLANG_RETURN_ON_FAIL(
        _resolveFrozen(projectRoot, manifest, lock, name, true, resolved, outError));
    SLANG_RETURN_ON_FAIL(_writeLock(projectRoot, resolved, outError));
    fprintf(
        stdout,
        "Pinned '%s' at %s (%s).\n",
        name.getBuffer(),
        package.version.getBuffer(),
        tag.commit.getBuffer());
    return SLANG_OK;
}

SlangResult unpinLockedPackage(const String& projectRoot, const String& name, String& outError)
{
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readLock(projectRoot, lock, outError));
    Index index = findLockedPackageIndex(lock, name);
    if (index < 0)
    {
        outError = String("Package is not present in the lock file: ") + name;
        return SLANG_FAIL;
    }
    lock.packages[index].pinned = false;
    SLANG_RETURN_ON_FAIL(_writeLock(projectRoot, lock, outError));
    fprintf(stdout, "Unpinned '%s'.\n", name.getBuffer());
    return SLANG_OK;
}

SlangResult beginPackageEdit(
    const String& projectRoot,
    const String& name,
    const String& branch,
    bool create,
    String& outError)
{
    if (!branch.getLength() || branch == "HEAD" || branch.startsWith("-"))
    {
        outError = "edit requires a branch name.";
        return SLANG_FAIL;
    }
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProject(projectRoot, manifest, outError));
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readLock(projectRoot, lock, outError));
    Index index = findLockedPackageIndex(lock, name);
    if (index < 0)
    {
        outError = String("Package is not present in the lock file: ") + name;
        return SLANG_FAIL;
    }
    LockedPackage& package = lock.packages[index];
    if (package.branch.getLength())
    {
        outError = String("Package is already edited on branch '") + package.branch + "': " + name;
        return SLANG_FAIL;
    }
    if (!package.commit.getLength())
    {
        outError = String("Package has no resolved commit to edit: ") + name;
        return SLANG_FAIL;
    }
    String checkout = _checkoutPath(projectRoot, manifest, name);
    SLANG_RETURN_ON_FAIL(_requireCheckout(checkout, package.git, outError));
    bool exists = false;
    SLANG_RETURN_ON_FAIL(localBranchExists(checkout, branch, exists, outError));
    if (!exists && !create)
    {
        outError = String("Branch does not exist: ") + branch + ". Pass --create to create it at " +
                   package.commit + ".";
        return SLANG_FAIL;
    }
    if (!exists)
        SLANG_RETURN_ON_FAIL(createLocalBranch(checkout, branch, package.commit, outError));
    String representedCommit = package.commit;
    if (SLANG_FAILED(checkoutLocalBranch(checkout, branch, outError)))
    {
        String restoreError;
        checkoutDetachedCommit(checkout, representedCommit, restoreError);
        return SLANG_FAIL;
    }

    bool pinned = package.pinned;
    String version = package.version;
    package.branch = branch;
    package.pinned = pinned;
    LockFile resolved;
    if (SLANG_FAILED(_resolveFrozen(projectRoot, manifest, lock, name, pinned, resolved, outError)))
    {
        String restoreError;
        checkoutDetachedCommit(checkout, representedCommit, restoreError);
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(_writeLock(projectRoot, resolved, outError));
    fprintf(
        stdout,
        "Editing '%s' on branch '%s' as %s.\n",
        name.getBuffer(),
        branch.getBuffer(),
        version.getBuffer());
    return SLANG_OK;
}

static SlangResult _loadEditLine(
    const String& checkout,
    const LockedPackage& package,
    String& outHead,
    List<EditLineTag>& outTags,
    bool& outReachedPin,
    String& outError)
{
    String branch;
    bool detached = false;
    SLANG_RETURN_ON_FAIL(getCheckedOutBranch(checkout, branch, detached, outError));
    if (detached || branch != package.branch)
    {
        outError = String("Edited package '") + package.name + "' is not checked out on branch '" +
                   package.branch + "'.";
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(getRepositoryHeadCommit(checkout, outHead, outError));
    return collectCanonicalTagsOnEditLine(
        checkout,
        outHead,
        package.commit,
        outTags,
        outReachedPin,
        outError);
}

/// Greatest canonical tag on `tags` that is newer than `baseline` and satisfies constraints.
///
/// `outRejected` names a newer tag that failed the constraints when no tag was usable. A lesser
/// tag that does satisfy them is still selected: the candidate is the greatest tag that matches.
static SlangResult _selectNewerTag(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const String& packageName,
    const PackageVersion& baseline,
    const List<EditLineTag>& tags,
    bool& outFound,
    EditLineTag& outTag,
    String& outRejected,
    String& outError)
{
    outFound = false;
    outRejected = String();
    EditLineTag const* best = nullptr;
    EditLineTag const* newestFailure = nullptr;
    String newestFailureReason;
    for (const auto& tag : tags)
    {
        if (!(tag.version > baseline))
            continue;
        String reason;
        if (SLANG_FAILED(_requireVersionAccepted(
                projectRoot,
                manifest,
                lock,
                packageName,
                tag.version,
                reason)))
        {
            if (!reason.startsWith("Version "))
            {
                outError = reason;
                return SLANG_FAIL;
            }
            if (!newestFailure || tag.version > newestFailure->version)
            {
                newestFailure = &tag;
                newestFailureReason = reason;
            }
            continue;
        }
        if (!best || tag.version > best->version)
            best = &tag;
    }
    if (!best)
    {
        if (newestFailure)
            outRejected = String("Tag ") + newestFailure->tag +
                          " is newer but cannot be selected. " + newestFailureReason;
        return SLANG_OK;
    }
    outFound = true;
    outTag = *best;
    return SLANG_OK;
}

SlangResult advancePackageEdit(const String& projectRoot, const String& name, String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProject(projectRoot, manifest, outError));
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readLock(projectRoot, lock, outError));
    Index index = findLockedPackageIndex(lock, name);
    if (index < 0)
    {
        outError = String("Package is not present in the lock file: ") + name;
        return SLANG_FAIL;
    }
    LockedPackage& package = lock.packages[index];
    if (!package.branch.getLength())
    {
        outError = String("Package is not edited: ") + name;
        return SLANG_FAIL;
    }
    String checkout = _checkoutPath(projectRoot, manifest, name);
    SLANG_RETURN_ON_FAIL(_requireCheckout(checkout, package.git, outError));
    String head;
    List<EditLineTag> tags;
    bool reachedPin = false;
    SLANG_RETURN_ON_FAIL(_loadEditLine(checkout, package, head, tags, reachedPin, outError));
    if (!reachedPin)
    {
        outError = String("The edit line of '") + name + "' does not reach commit " +
                   package.commit + ". The row was not changed.";
        return SLANG_FAIL;
    }
    PackageVersion frozen;
    SLANG_RETURN_ON_FAIL(parseExactVersion(package.version, frozen, outError));
    bool found = false;
    EditLineTag tag;
    String rejected;
    SLANG_RETURN_ON_FAIL(_selectNewerTag(
        projectRoot,
        manifest,
        lock,
        name,
        frozen,
        tags,
        found,
        tag,
        rejected,
        outError));
    if (!found)
    {
        outError = rejected.getLength()
                       ? rejected + " The row was not changed."
                       : String("No canonical tag on branch '") + package.branch +
                             "' is newer than " + package.version +
                             " and satisfies the constraints. The row was not changed.";
        return SLANG_FAIL;
    }
    bool pinned = package.pinned;
    package.version = formatExactVersion(tag.version);
    package.ref = tag.tag;
    package.commit = tag.commit;
    LockFile resolved;
    SLANG_RETURN_ON_FAIL(
        _resolveFrozen(projectRoot, manifest, lock, name, pinned, resolved, outError));
    SLANG_RETURN_ON_FAIL(_writeLock(projectRoot, resolved, outError));
    fprintf(
        stdout,
        "Advanced '%s' to %s. The checkout was not moved.\n",
        name.getBuffer(),
        package.version.getBuffer());
    return SLANG_OK;
}

static SlangResult _checkoutIsDirty(const String& checkout, bool& outDirty, String& outError)
{
    String head;
    SLANG_RETURN_ON_FAIL(getRepositoryHeadCommit(checkout, head, outError));
    GitWorkingTreeStatus status;
    SLANG_RETURN_ON_FAIL(getWorkingTreeStatus(checkout, head, status, outError));
    outDirty = status.changedFileCount != 0 || status.stashCount != 0;
    return SLANG_OK;
}

static SlangResult _releaseAlreadyTagged(
    const String& checkout,
    const PackageVersion& version,
    bool& outTagged,
    String& outError)
{
    outTagged = false;
    List<String> tags;
    SLANG_RETURN_ON_FAIL(listTagNames(checkout, tags, outError));
    for (const auto& tag : tags)
    {
        PackageVersion parsed;
        if (SLANG_FAILED(parseReleaseTag(tag.getUnownedSlice(), parsed)))
            continue;
        if (parsed == version)
        {
            outTagged = true;
            return SLANG_OK;
        }
    }
    return SLANG_OK;
}

static SlangResult _landRelease(
    const String& projectRoot,
    const Manifest& manifest,
    LockFile lock,
    Index index,
    const String& version,
    const String& tag,
    const String& commit,
    bool pinned,
    const String& branchToRestore,
    bool checkoutMoved,
    const String& checkout,
    String& outError)
{
    LockedPackage& package = lock.packages[index];
    package.version = version;
    package.ref = tag;
    package.commit = commit;
    package.branch = String();
    package.pinned = true;
    LockFile resolved;
    if (SLANG_FAILED(
            _resolveFrozen(projectRoot, manifest, lock, package.name, pinned, resolved, outError)))
    {
        if (checkoutMoved)
        {
            String restoreError;
            checkoutLocalBranch(checkout, branchToRestore, restoreError);
        }
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(_writeLock(projectRoot, resolved, outError));
    fprintf(
        stdout,
        "Package '%s' is a release at %s (%s).\n",
        package.name.getBuffer(),
        version.getBuffer(),
        commit.getBuffer());
    return SLANG_OK;
}

SlangResult endPackageEdit(
    const String& projectRoot,
    const String& name,
    UneditMode mode,
    const String& tagVersionText,
    bool clean,
    String& outError)
{
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(_readProject(projectRoot, manifest, outError));
    LockFile lock;
    SLANG_RETURN_ON_FAIL(_readLock(projectRoot, lock, outError));
    Index index = findLockedPackageIndex(lock, name);
    if (index < 0)
    {
        outError = String("Package is not present in the lock file: ") + name;
        return SLANG_FAIL;
    }
    LockedPackage package = lock.packages[index];
    if (!package.branch.getLength())
    {
        outError = String("Package is not edited: ") + name;
        return SLANG_FAIL;
    }
    String checkout = _checkoutPath(projectRoot, manifest, name);
    SLANG_RETURN_ON_FAIL(_requireCheckout(checkout, package.git, outError));
    String head;
    List<EditLineTag> tags;
    bool reachedPin = false;
    SLANG_RETURN_ON_FAIL(_loadEditLine(checkout, package, head, tags, reachedPin, outError));

    PackageVersion represented;
    SLANG_RETURN_ON_FAIL(parseExactVersion(package.version, represented, outError));
    bool found = false;
    EditLineTag newer;
    String rejected;
    if (reachedPin && mode != UneditMode::Tag)
    {
        SLANG_RETURN_ON_FAIL(_selectNewerTag(
            projectRoot,
            manifest,
            lock,
            name,
            represented,
            tags,
            found,
            newer,
            rejected,
            outError));
    }

    if (mode == UneditMode::Tag)
    {
        if (!tagVersionText.getLength())
        {
            outError = "unedit --tag requires a version.";
            return SLANG_FAIL;
        }
        PackageVersion requested;
        SLANG_RETURN_ON_FAIL(parseExactVersion(tagVersionText, requested, outError));
        bool already = false;
        SLANG_RETURN_ON_FAIL(_releaseAlreadyTagged(checkout, requested, already, outError));
        if (already)
        {
            outError = String("Release ") + formatExactVersion(requested) + " already has a tag.";
            return SLANG_FAIL;
        }
        if (!(requested > represented))
        {
            outError = String("Tag version must be greater than the edited version ") +
                       package.version + ".";
            return SLANG_FAIL;
        }
        for (const auto& tag : tags)
        {
            if (!(requested > tag.version))
            {
                outError = String("Tag version must be greater than every canonical tag on the "
                                  "edit line, including ") +
                           tag.tag + ".";
                return SLANG_FAIL;
            }
        }
        SLANG_RETURN_ON_FAIL(
            _requireVersionAccepted(projectRoot, manifest, lock, name, requested, outError));
        String tagName = String("v") + formatExactVersion(requested);
        SLANG_RETURN_ON_FAIL(createAnnotatedTag(checkout, tagName, outError));
        return _landRelease(
            projectRoot,
            manifest,
            lock,
            index,
            formatExactVersion(requested),
            tagName,
            head,
            package.pinned,
            package.branch,
            false,
            checkout,
            outError);
    }

    auto restore = [&](const String& preface) -> SlangResult
    {
        StringBuilder prompt;
        if (preface.getLength())
            prompt << preface << " ";
        prompt << "Return '" << name << "' to " << package.version << " at " << package.commit
               << "?";
        bool approved = false;
        SLANG_RETURN_ON_FAIL(_confirm(prompt.getBuffer(), approved, outError));
        if (!approved)
            return SLANG_OK;
        bool moves = head != package.commit;
        if (moves)
        {
            bool dirty = false;
            SLANG_RETURN_ON_FAIL(_checkoutIsDirty(checkout, dirty, outError));
            if (dirty && !clean)
            {
                outError = String("Checkout has uncommitted files or stashes: ") + checkout +
                           ". Commit them or re-run with --clean.";
                return SLANG_FAIL;
            }
            if (dirty)
                SLANG_RETURN_ON_FAIL(discardUncommittedState(checkout, outError));
            SLANG_RETURN_ON_FAIL(checkoutDetachedCommit(checkout, package.commit, outError));
        }
        return _landRelease(
            projectRoot,
            manifest,
            lock,
            index,
            package.version,
            package.ref,
            package.commit,
            package.pinned,
            package.branch,
            moves,
            checkout,
            outError);
    };

    if (mode == UneditMode::Restore)
    {
        String preface;
        if (found)
            preface = String("Tag ") + newer.tag +
                      " would satisfy the constraints and is not being selected.";
        else if (rejected.getLength())
            preface = rejected;
        return restore(preface);
    }

    if (mode == UneditMode::Advance && (!reachedPin || !found))
    {
        outError = !reachedPin ? String("The edit line of '") + name + "' does not reach commit " +
                                     package.commit + "."
                               : (rejected.getLength()
                                      ? rejected
                                      : String("No canonical tag on branch '") + package.branch +
                                            "' is newer than " + package.version +
                                            " and satisfies the constraints.");
        return SLANG_FAIL;
    }

    if (!reachedPin || !found)
    {
        String preface =
            !reachedPin
                ? String("The edit line does not reach commit ") + package.commit + "."
                : (rejected.getLength() ? rejected
                                        : String("No newer canonical tag on branch '") +
                                              package.branch + "' satisfies the constraints.");
        return restore(preface);
    }

    Index commitsAfter = 0;
    if (newer.commit != head)
        SLANG_RETURN_ON_FAIL(
            countCommitsAfter(checkout, newer.commit, head, commitsAfter, outError));
    bool moves = commitsAfter != 0;
    if (moves)
    {
        bool dirty = false;
        SLANG_RETURN_ON_FAIL(_checkoutIsDirty(checkout, dirty, outError));
        if (dirty && !clean)
        {
            outError = String("Checkout has uncommitted files or stashes: ") + checkout +
                       ". Commit them or re-run with --clean.";
            return SLANG_FAIL;
        }
        StringBuilder prompt;
        prompt << "Tag " << newer.tag << " is " << commitsAfter
               << (commitsAfter == 1 ? " commit" : " commits") << " behind the tip of "
               << package.branch << ". Check out " << newer.tag
               << " and leave those commits out of the workspace?";
        bool approved = false;
        SLANG_RETURN_ON_FAIL(_confirm(prompt.getBuffer(), approved, outError));
        if (!approved)
            return SLANG_OK;
        if (dirty)
            SLANG_RETURN_ON_FAIL(discardUncommittedState(checkout, outError));
        SLANG_RETURN_ON_FAIL(checkoutDetachedCommit(checkout, newer.commit, outError));
    }
    return _landRelease(
        projectRoot,
        manifest,
        lock,
        index,
        formatExactVersion(newer.version),
        newer.tag,
        newer.commit,
        package.pinned,
        package.branch,
        moves,
        checkout,
        outError);
}

} // namespace PackageTool
} // namespace Slang
