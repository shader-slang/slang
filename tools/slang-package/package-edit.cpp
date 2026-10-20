// package-edit.cpp

#include "package-edit.h"

#include "core/slang-io.h"
#include "core/slang-string-util.h"
#include "core/slang-writer.h"
#include "package-git.h"
#include "package-json.h"
#include "package-lock.h"
#include "package-report.h"
#include "package-resolver.h"
#include "package-tool.h"
#include "package-validate.h"

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
/// Report whether `checkout` has uncommitted or untracked files.
///
/// Stashes are not counted, because checking out another commit leaves them in place.
static SlangResult _hasUncommittedFiles(const String& checkout, bool& outDirty, String& outError)
{
    String head;
    SLANG_RETURN_ON_FAIL(getRepositoryHeadCommit(checkout, head, outError));
    GitWorkingTreeStatus status;
    SLANG_RETURN_ON_FAIL(getWorkingTreeStatus(checkout, head, status, outError));
    outDirty = status.changedFileCount != 0;
    return SLANG_OK;
}

/// Read the remap index `lock` records, if any. `outRemap` is null when the lock has none.
static SlangResult _readLockRemap(
    const LockFile& lock,
    List<RepositoryLocation>& outPackages,
    const List<RepositoryLocation>*& outRemap,
    String& outError)
{
    outRemap = nullptr;
    if (!lock.remapIndex.getLength())
        return SLANG_OK;
    SLANG_RETURN_ON_FAIL(readPackageIndex(lock.remapIndex, outPackages, outError));
    outRemap = &outPackages;
    return SLANG_OK;
}

/// Solve `lock` the way `slang package update` would, without writing anything.
///
/// `update` keeps pinned and edited rows and moves every other row to the greatest release the
/// constraints allow. The solve reads `.slang/repositories` without fetching. A non-null `pending`
/// is listed among its package's release tags as if it were published.
static SlangResult _solveLikeUpdate(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const List<RepositoryLocation>* remapPackages,
    const PendingRelease* pending,
    LockFile& outLock,
    ResolveReport& outReport,
    String& outError)
{
    List<String> ignoredWarnings;
    return resolveDependencies(
        projectRoot,
        manifest,
        outLock,
        outError,
        &ignoredWarnings,
        &outReport,
        true,
        &lock,
        remapPackages,
        pending);
}

/// Format the conflict that ended a failed solve, falling back to the error text when the solver
/// did not record a package conflict.
static String _describeFailedSolve(const ResolveReport& report, const String& error)
{
    String text =
        report.failure.packageName.getLength() ? formatResolveFailure(report.failure) : error;
    if (!text.endsWith("\n"))
        text = text + "\n";
    return text;
}

/// Prefix every line of `text` with `indent`.
static String _indentLines(const String& text, const char* indent)
{
    StringBuilder builder;
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(text.getUnownedSlice(), lines);
    for (const auto& line : lines)
    {
        if (line.getLength())
            builder << indent << line << "\n";
    }
    return builder.produceString();
}

/// Append one line to `outText` for each row other than `skipName` that differs between
/// `baseline` and `changed`, two solved locks.
///
/// Consider `pin noise 1.2` where `grain` 1.3 requires `noise >=1.3`. Solved without the pin, the
/// workspace selects `grain` 1.3; solved with it, `grain` 1.2. The line names both releases and,
/// from `changedReport`, the requirement that rejected the release the baseline selected.
static void _appendOtherPackageChanges(
    const LockFile& baseline,
    const LockFile& changed,
    const ResolveReport& changedReport,
    const String& skipName,
    StringBuilder& outText)
{
    for (const auto& package : changed.packages)
    {
        if (package.name == skipName)
            continue;
        Index before = findLockedPackageIndex(baseline, package.name);
        if (before < 0)
        {
            outText << "  add '" << package.name << "' at " << package.ref << " (" << package.commit
                    << ")\n";
            continue;
        }
        const LockedPackage& previous = baseline.packages[before];
        if (previous.commit == package.commit)
            continue;
        outText << "  select '" << package.name << "' " << package.ref << " (" << package.commit
                << ") instead of " << previous.ref << " (" << previous.commit << ")\n";
        PackageVersion previousVersion;
        String parseError;
        if (SLANG_SUCCEEDED(parseExactVersion(previous.version, previousVersion, parseError)))
            outText << _indentLines(
                formatReleaseRejections(changedReport, package.name, previousVersion),
                "  ");
    }
    for (const auto& package : baseline.packages)
    {
        if (package.name != skipName && findLockedPackageIndex(changed, package.name) < 0)
            outText << "  remove '" << package.name << "'\n";
    }
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
    String cache = packageRepositoryPath(projectRoot, package.git);
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

/// Ask before pinning a release when a later `update` would change other packages because of it.
///
/// Consider `pin noise 1.2` where the latest `grain` requires `noise >=1.3`. `pin` writes only the
/// `noise` row; the next `update` reconciles the rest. To show what that `update` will do, `lock`
/// (the lock as it is) and `pinned` (the lock this command is about to write) are both solved the
/// way `update` would solve them. Rows that differ between the two solves, other than `name`, are
/// the impact of the pin: here `grain` would move to an older release that accepts `noise` 1.2.
/// When nothing else differs, `outApproved` is true without a prompt. When the pinned solve fails,
/// the warning shows the conflict, or says that `update` already fails without the pin.
static SlangResult _confirmPinImpact(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    const LockFile& pinned,
    const String& name,
    const String& tag,
    bool& outApproved,
    String& outError)
{
    outApproved = false;
    List<RepositoryLocation> remapPackages;
    const List<RepositoryLocation>* remap = nullptr;
    SLANG_RETURN_ON_FAIL(_readLockRemap(lock, remapPackages, remap, outError));

    LockFile baselineSolved;
    ResolveReport baselineReport;
    String baselineError;
    bool baselineSucceeded = SLANG_SUCCEEDED(_solveLikeUpdate(
        projectRoot,
        manifest,
        lock,
        remap,
        nullptr,
        baselineSolved,
        baselineReport,
        baselineError));
    LockFile pinnedSolved;
    ResolveReport pinnedReport;
    String pinnedError;
    bool pinnedSucceeded = SLANG_SUCCEEDED(_solveLikeUpdate(
        projectRoot,
        manifest,
        pinned,
        remap,
        nullptr,
        pinnedSolved,
        pinnedReport,
        pinnedError));

    StringBuilder warning;
    if (!pinnedSucceeded && !baselineSucceeded)
    {
        warning << "'slang package update' already fails without pinning '" << name << "':\n"
                << _describeFailedSolve(baselineReport, baselineError);
    }
    else if (!pinnedSucceeded)
    {
        warning << "after this pin, 'slang package update' would fail:\n"
                << _describeFailedSolve(pinnedReport, pinnedError);
    }
    else if (baselineSucceeded)
    {
        StringBuilder changes;
        _appendOtherPackageChanges(baselineSolved, pinnedSolved, pinnedReport, name, changes);
        if (changes.getLength())
            warning << "after this pin, 'slang package update' would change other packages:\n"
                    << changes.produceString();
    }
    if (!warning.getLength())
    {
        outApproved = true;
        return SLANG_OK;
    }
    fprintf(stderr, "slang-package: warning: %s", warning.produceString().getBuffer());

    StringBuilder prompt;
    prompt << "Pin '" << name << "' at " << tag << "?";
    return _confirm(prompt.getBuffer(), outApproved, outError);
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
                       "' is edited. The version moves with 'slang package unedit " + name +
                       " --tag <version>'.";
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
    String repository;
    SLANG_RETURN_ON_FAIL(_findTagRepository(projectRoot, manifest, package, repository, outError));
    TagCandidate tag;
    SLANG_RETURN_ON_FAIL(_findCanonicalTag(repository, version, tag, outError));

    LockFile pinned = lock;
    LockedPackage& pinnedPackage = pinned.packages[index];
    pinnedPackage.version = formatExactVersion(version);
    pinnedPackage.ref = tag.ref;
    pinnedPackage.commit = tag.commit;
    pinnedPackage.pinned = true;
    bool approved = false;
    SLANG_RETURN_ON_FAIL(
        _confirmPinImpact(projectRoot, manifest, lock, pinned, name, tag.ref, approved, outError));
    if (!approved)
        return SLANG_OK;

    // The checkout moves to the pinned commit before the lock names it, the same way `fetch`
    // would, so the workspace builds what the lock says. Uncommitted files block the move, because
    // `pin` never discards work.
    String checkout = _checkoutPath(projectRoot, manifest, name);
    String cachePath = packageRepositoryPath(projectRoot, package.git);
    SlangPathType checkoutType;
    if (SLANG_SUCCEEDED(Path::getPathType(checkout, &checkoutType)))
    {
        bool dirty = false;
        SLANG_RETURN_ON_FAIL(_hasUncommittedFiles(checkout, dirty, outError));
        if (dirty)
        {
            outError = checkout + " has uncommitted changes. Commit or stash them before pinning.";
            return SLANG_FAIL;
        }
    }
    else if (!Path::createDirectoryRecursive(Path::getParentDirectory(checkout)))
    {
        outError =
            String("Cannot create dependency directory: ") + Path::getParentDirectory(checkout);
        return SLANG_FAIL;
    }
    bool moved = false;
    SLANG_RETURN_ON_FAIL(materializeLockedRevision(
        package.git,
        package.commit,
        tag.commit,
        checkout,
        false,
        false,
        moved,
        outError,
        cachePath));
    if (SLANG_FAILED(_writeLock(projectRoot, pinned, outError)))
    {
        bool restored = false;
        String restoreError;
        if (moved)
            materializeLockedRevision(
                package.git,
                tag.commit,
                package.commit,
                checkout,
                false,
                false,
                restored,
                restoreError,
                cachePath);
        return SLANG_FAIL;
    }
    if (moved)
        fprintf(
            stdout,
            "Checked out '%s' at %s (%s).\n",
            name.getBuffer(),
            tag.ref.getBuffer(),
            tag.commit.getBuffer());
    fprintf(
        stdout,
        "Pinned '%s' at %s (%s).\n",
        name.getBuffer(),
        pinnedPackage.version.getBuffer(),
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
    // A failed edit returns the checkout to the commit it was on and, when this command created
    // the branch, deletes it, so the repository is as it was before the command ran.
    auto abandon = [&]() -> SlangResult
    {
        String restoreError;
        if (SLANG_SUCCEEDED(checkoutDetachedCommit(checkout, representedCommit, restoreError)) &&
            !exists && SLANG_FAILED(deleteLocalBranch(checkout, branch, restoreError)))
            outError = outError + "\nBranch '" + branch + "' was left in place: " + restoreError;
        return SLANG_FAIL;
    };
    if (SLANG_FAILED(checkoutLocalBranch(checkout, branch, outError)))
        return abandon();

    package.branch = branch;
    if (SLANG_FAILED(_writeLock(projectRoot, lock, outError)))
        return abandon();
    fprintf(
        stdout,
        "Editing '%s' on branch '%s' as %s.\n",
        name.getBuffer(),
        branch.getBuffer(),
        package.version.getBuffer());
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

/// Ask before ending an edit on a new release that a later `update` would not keep.
///
/// Consider `unedit noise --tag 1.4` in a workspace where `grain` requires `noise >=1, <1.4`.
/// `landed` is the lock this command is about to write, with `noise` at 1.4 and every other row
/// as it is. This command does not resolve the rest of the lock; the next `update` does. To show
/// what that `update` will do, `landed` is solved the way `update` would solve it, with `v1.4`
/// listed among the `noise` tags before it exists. When that solve keeps `commit`, `outApproved`
/// is true without a prompt. Otherwise the warning says what `update` would do instead and why:
/// move `noise` to another release, or fail. Here it would move `noise` back below 1.4, and the
/// warning names the `grain` requirement.
///
/// A conflict reached through the new release's own requirements, such as `noise` 1.4 requiring a
/// `hash` that another package rejects, is reported the same way, because it is the same solve.
/// When the solve fails, `baseline` (the lock as the edit started) is solved too, so a workspace
/// that fails without the new release is reported as already failing.
static SlangResult _confirmUpdateKeepsRelease(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& baseline,
    const LockFile& landed,
    const String& name,
    const PackageVersion& version,
    const String& tag,
    const String& commit,
    bool& outApproved,
    String& outError)
{
    outApproved = false;
    List<RepositoryLocation> remapPackages;
    const List<RepositoryLocation>* remap = nullptr;
    SLANG_RETURN_ON_FAIL(_readLockRemap(landed, remapPackages, remap, outError));
    Index landedIndex = findLockedPackageIndex(landed, name);
    SLANG_RELEASE_ASSERT(landedIndex >= 0);
    PendingRelease pending;
    pending.packageName = name;
    pending.git = landed.packages[landedIndex].git;
    pending.candidate.ref = tag;
    pending.candidate.commit = commit;
    pending.candidate.version = version;

    LockFile solved;
    ResolveReport report;
    String solveError;
    bool succeeded = SLANG_SUCCEEDED(_solveLikeUpdate(
        projectRoot,
        manifest,
        landed,
        remap,
        &pending,
        solved,
        report,
        solveError));
    Index selectedIndex = succeeded ? findLockedPackageIndex(solved, name) : -1;
    if (selectedIndex >= 0 && solved.packages[selectedIndex].commit == commit)
    {
        outApproved = true;
        return SLANG_OK;
    }

    StringBuilder warning;
    const char* after = "after this unedit, 'slang package update' would ";
    if (succeeded && selectedIndex < 0)
    {
        warning << after << "remove '" << name << "' from the lock, because nothing requires it.\n";
    }
    else if (succeeded)
    {
        const LockedPackage& selected = solved.packages[selectedIndex];
        warning << after << "move '" << name << "' from " << tag << " to " << selected.ref << " ("
                << selected.commit << "):\n";
        PackageVersion selectedVersion;
        String parseError;
        String rejections = formatReleaseRejections(report, name, version);
        if (SLANG_SUCCEEDED(parseExactVersion(selected.version, selectedVersion, parseError)) &&
            selectedVersion > version)
            warning << "  " << selected.ref
                    << " is a greater release that every requirement accepts.\n";
        else if (rejections.getLength())
            warning << rejections;
        else
            warning << "  the solver selects " << selected.ref
                    << " when the other packages move to their newest releases.\n";
    }
    else
    {
        LockFile baselineSolved;
        ResolveReport baselineReport;
        String baselineError;
        if (SLANG_FAILED(_solveLikeUpdate(
                projectRoot,
                manifest,
                baseline,
                remap,
                nullptr,
                baselineSolved,
                baselineReport,
                baselineError)))
        {
            warning << "'slang package update' already fails without " << tag << ":\n"
                    << _describeFailedSolve(baselineReport, baselineError);
        }
        else
        {
            warning << after << "fail:\n" << _describeFailedSolve(report, solveError);
        }
    }
    fprintf(stderr, "slang-package: warning: %s", warning.produceString().getBuffer());

    StringBuilder prompt;
    prompt << "Create " << tag << " and end the edit?";
    return _confirm(prompt.getBuffer(), outApproved, outError);
}

/// Print how ending an edit changed the row from `before` to `after`.
static void _printEndedEdit(const LockedPackage& before, const LockedPackage& after)
{
    if (before.commit == after.commit)
    {
        fprintf(
            stdout,
            "Package '%s' is a release at %s (%s).",
            after.name.getBuffer(),
            after.version.getBuffer(),
            after.commit.getBuffer());
    }
    else if (after.pinned)
    {
        fprintf(
            stdout,
            "Pin on '%s' moved from %s (%s) to %s (%s).",
            after.name.getBuffer(),
            before.ref.getBuffer(),
            before.commit.getBuffer(),
            after.ref.getBuffer(),
            after.commit.getBuffer());
    }
    else
    {
        fprintf(
            stdout,
            "Package '%s' is a release at %s (%s), was %s (%s).",
            after.name.getBuffer(),
            after.version.getBuffer(),
            after.commit.getBuffer(),
            before.version.getBuffer(),
            before.commit.getBuffer());
    }
}

/// End the edit at row `index` by creating the release `tagVersionText` on `head`.
///
/// The tag is created only after `_confirmUpdateKeepsRelease` approves it, so declining leaves
/// the repository and the edit unchanged. Only the edited row is rewritten; the checkout stays on
/// the branch. A dependency the new release declares that the lock does not yet contain is named,
/// because `fetch` cannot install the workspace until `update` adds it.
static SlangResult _endEditOnNewTag(
    const String& projectRoot,
    const Manifest& manifest,
    const LockFile& lock,
    Index index,
    const String& checkout,
    const String& head,
    const List<EditLineTag>& lineTags,
    const String& tagVersionText,
    String& outError)
{
    const LockedPackage& package = lock.packages[index];
    if (!tagVersionText.getLength())
    {
        outError = "unedit --tag requires a version.";
        return SLANG_FAIL;
    }
    PackageVersion represented;
    SLANG_RETURN_ON_FAIL(parseExactVersion(package.version, represented, outError));
    PackageVersion requested;
    SLANG_RETURN_ON_FAIL(parseExactVersion(tagVersionText, requested, outError));
    Manifest checkoutManifest;
    SLANG_RETURN_ON_FAIL(
        readManifest(Path::combine(checkout, kPackageFileName), checkoutManifest, outError));
    SLANG_RETURN_ON_FAIL(validatePublishablePackage(checkout, checkoutManifest, outError));
    SLANG_RETURN_ON_FAIL(requireCleanReleaseCheckout(checkout, outError));
    String cachePath = packageRepositoryPath(projectRoot, package.git);
    SLANG_RETURN_ON_FAIL(refreshPackageCache(projectRoot, package.git, cachePath, outError));
    bool already = false;
    SLANG_RETURN_ON_FAIL(_releaseAlreadyTagged(checkout, requested, already, outError));
    if (already)
    {
        outError = String("Release ") + formatExactVersion(requested) + " already has a tag.";
        return SLANG_FAIL;
    }
    if (!(requested > represented))
    {
        outError =
            String("Tag version must be greater than the edited version ") + package.version + ".";
        return SLANG_FAIL;
    }
    for (const auto& tag : lineTags)
    {
        if (!(requested > tag.version))
        {
            outError = String("Tag version must be greater than every canonical tag on the "
                              "edit line, including ") +
                       tag.tag + ".";
            return SLANG_FAIL;
        }
    }

    String tagName = String("v") + formatExactVersion(requested);
    LockFile baseline = lock;
    baseline.packages[index].branch = String();
    LockFile landed = baseline;
    LockedPackage& landedPackage = landed.packages[index];
    landedPackage.version = formatExactVersion(requested);
    landedPackage.ref = tagName;
    landedPackage.commit = head;

    bool approved = false;
    SLANG_RETURN_ON_FAIL(_confirmUpdateKeepsRelease(
        projectRoot,
        manifest,
        baseline,
        landed,
        package.name,
        requested,
        tagName,
        head,
        approved,
        outError));
    if (!approved)
        return SLANG_OK;

    SLANG_RETURN_ON_FAIL(createAnnotatedTag(checkout, tagName, outError));
    if (SLANG_FAILED(_writeLock(projectRoot, landed, outError)))
    {
        String deleteError;
        if (SLANG_FAILED(deleteLocalTag(checkout, tagName, deleteError)))
            outError = outError + "\nTag " + tagName + " was left in place: " + deleteError;
        return SLANG_FAIL;
    }
    fprintf(
        stdout,
        "Created tag %s at %s in %s.\n",
        tagName.getBuffer(),
        head.getBuffer(),
        checkout.getBuffer());
    _printEndedEdit(package, landedPackage);
    fprintf(stdout, "\n");
    for (const auto& dependency : checkoutManifest.dependencies)
    {
        if (findLockedPackageIndex(landed, dependency.name) >= 0)
            continue;
        fprintf(
            stdout,
            "%s %s requires '%s', which is not in the lock. Run 'slang package update' to add "
            "it.\n",
            package.name.getBuffer(),
            landedPackage.version.getBuffer(),
            dependency.name.getBuffer());
    }
    return SLANG_OK;
}

SlangResult endPackageEdit(
    const String& projectRoot,
    const String& name,
    UneditMode mode,
    const String& tagVersionText,
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
    List<EditLineTag> lineTags;
    bool reachedPin = false;
    SLANG_RETURN_ON_FAIL(_loadEditLine(checkout, package, head, lineTags, reachedPin, outError));

    if (mode == UneditMode::Tag)
    {
        return _endEditOnNewTag(
            projectRoot,
            manifest,
            lock,
            index,
            checkout,
            head,
            lineTags,
            tagVersionText,
            outError);
    }

    // Ending an edit without a release never discards work. A plain `unedit` accepts only an edit
    // with nothing to lose. `--abort` refuses uncommitted files, and the commits it leaves behind
    // stay on the branch, which is not deleted.
    bool dirty = false;
    SLANG_RETURN_ON_FAIL(_hasUncommittedFiles(checkout, dirty, outError));
    if (mode == UneditMode::Plain && (dirty || head != package.commit))
    {
        outError = String("'") + name + "' has changes since " + package.ref +
                   ". Run 'slang package unedit " + name +
                   " --tag <version>' to release them, or '--abort' to return to " + package.ref +
                   ".";
        return SLANG_FAIL;
    }
    if (dirty)
    {
        outError = checkout + " has uncommitted changes. Commit or stash them before --abort.";
        return SLANG_FAIL;
    }
    Index commitsKept = 0;
    if (head != package.commit)
        SLANG_RETURN_ON_FAIL(
            countCommitsAfter(checkout, package.commit, head, commitsKept, outError));
    SLANG_RETURN_ON_FAIL(checkoutDetachedCommit(checkout, package.commit, outError));
    lock.packages[index].branch = String();
    if (SLANG_FAILED(_writeLock(projectRoot, lock, outError)))
    {
        String restoreError;
        checkoutLocalBranch(checkout, package.branch, restoreError);
        return SLANG_FAIL;
    }
    _printEndedEdit(package, lock.packages[index]);
    if (commitsKept)
        fprintf(
            stdout,
            " Branch '%s' keeps its %d %s.",
            package.branch.getBuffer(),
            int(commitsKept),
            commitsKept == 1 ? "commit" : "commits");
    fprintf(stdout, "\n");
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
