#pragma once

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

/// Directory of the permanent repository for `gitURL` under `projectRoot`.
///
/// A URL whose last path segment is `noise.git` is stored at
/// `.slang/repositories/noise-` plus the first 8 hexadecimal characters of the SHA-1 of the
/// exact URL. The name keeps the directory readable, and the hash keeps two URLs that end in
/// the same name apart. The repository records that URL; opening the directory for a different
/// URL fails and leaves it in place.
String packageRepositoryPath(const String& projectRoot, const String& gitURL);

/// Final path segment of `packageRepositoryPath`, without the `.slang/repositories/` prefix.
String packageRepositoryDirectoryName(const String& gitURL);

/// List canonical dotted release tags already present in a local clone, without contacting a
/// remote.
///
/// Published tags are read from `refs/slang-cache/tags`. Tags under `refs/tags` are included only
/// when that published namespace has no tags, so a normal repository still lists the tags created
/// with `git tag`. A tag in `refs/tags` is not treated as a published release once the published
/// namespace exists. A tag that names a release with a non-canonical spelling, such as `v1.2.0`
/// when the canonical spelling is `v1.2`, is omitted. When `outWarnings` is set, each omitted tag
/// is reported once.
SlangResult listReleaseTagsFromRepository(
    const String& repositoryPath,
    List<TagCandidate>& outCandidates,
    String& outError,
    List<String>* outWarnings = nullptr);

/// Resolve `ref` from the origin-tracking refs and published tags already present in a package
/// repository.
///
/// A short tag name is read from `refs/slang-cache/tags` when that namespace has any tags, and
/// from `refs/tags` otherwise. `HEAD` is `refs/slang-cache/origin/HEAD`.
SlangResult resolveCachedReference(
    const String& repositoryPath,
    const String& ref,
    TagCandidate& outCandidate,
    String& outError);

/// A package cache kept beside `.slang/cache/<name>` until the user agrees to replace it.
///
/// `canonicalPath` is the cache a later command will read. `replacementPath` is a clone of
/// `gitURL` used to resolve versions while `canonicalPath` still holds the previous repository.
/// `report` lists commits and tags in the canonical repository that are not on a remote. A cache
/// with an empty report is replaced immediately and is not recorded here.
struct DeferredCacheReplacement
{
    String packageName;
    String gitURL;
    String canonicalPath;
    String replacementPath;
    String report;
};

/// Describe commits and tags in `repositoryPath` that are not on any of its remotes.
///
/// Local branch tips, a detached `HEAD`, and stashes are compared with remote-tracking refs after
/// those refs are updated. A tag is included when no remote has the same name and the same peeled
/// object. `outReport` is empty when deleting the repository would not drop unique history. The
/// repository is not deleted. Contacting a remote is required when one is configured; if that
/// check cannot be completed, the error says the repository was left in place.
SlangResult collectUnpushedRepositoryReport(
    const String& repositoryPath,
    String& outReport,
    String& outError);

/// Delete a Git repository whose unpushed commits and tags were shown to the user.
///
/// An empty current report is deleted, because every commit and tag was found on a remote.
/// A non-empty report is deleted only when `unpushedDeletionApproved` is true and the report still
/// equals `disclosedReport`, the text shown before the user agreed. Any other unpushed state is
/// left on disk and copied into `outError`.
SlangResult deleteDisclosedGitRepository(
    const String& repositoryPath,
    const String& disclosedReport,
    bool unpushedDeletionApproved,
    String& outError);

/// Return the cache directory that already has `gitURL` as its origin.
///
/// This is `canonicalPath` when that repository's origin matches. Otherwise it is the deferred
/// replacement clone beside the cache, when that clone's origin matches. When neither matches,
/// `outPath` is `canonicalPath` so the caller reports the ordinary cache mismatch.
SlangResult locatePreparedPackageCache(
    const String& canonicalPath,
    const String& gitURL,
    String& outPath,
    String& outError);

/// Remove a deferred replacement clone that does not itself hold unpushed commits or tags.
///
/// The canonical cache is not touched. A replacement that has its own unpushed state is left in
/// place, because that directory is no longer a scratch clone.
SlangResult discardReplacementPackageCache(const String& replacementPath, String& outError);

/// Replace `canonicalPath` with a clone of the deferred repository's URL.
///
/// The canonical repository is deleted only when its unpushed report still matches the report
/// that was shown. The replacement clone is removed after the canonical path has been recloned.
SlangResult commitDeferredCacheReplacement(
    const String& workingDirectory,
    const DeferredCacheReplacement& replacement,
    String& outError);

/// Create or refresh the permanent repository for `gitURL` at `repositoryPath`.
///
/// The repository is bare. Published branches are stored as `refs/remotes/origin/*` and published
/// tags as `refs/slang-cache/tags/*`. Fetch does not write or prune `refs/heads/*` or
/// `refs/tags/*`. A directory that already records a different URL is left in place and the call
/// fails. The `assumeYes` and unpushed-report arguments remain for callers that used to replace a
/// cache; this function does not delete the repository.
SlangResult refreshPackageCache(
    const String& workingDirectory,
    const String& gitURL,
    const String& repositoryPath,
    String& outError,
    bool assumeYes = false,
    String* outActivePath = nullptr,
    String* outUnpushedReport = nullptr);

/// Require an existing package cache with the expected origin, without contacting that origin.
SlangResult requirePackageCache(
    const String& gitURL,
    const String& repositoryPath,
    String& outError);

/// Fetch `commit` from a package cache's origin when it is not already present.
SlangResult fetchCachedCommit(const String& repositoryPath, const String& commit, String& outError);

/// Require `commit` to exist in a package cache without contacting its origin.
SlangResult requireCachedCommit(
    const String& repositoryPath,
    const String& commit,
    String& outError);

SlangResult readFileAtRevision(
    const String& repositoryPath,
    const String& revision,
    const String& filePath,
    String& outContents,
    String& outError);

/// Return the commit currently checked out at `HEAD`.
SlangResult getRepositoryHeadCommit(
    const String& repositoryPath,
    String& outCommit,
    String& outError);

/// Return the branch checked out at `HEAD`.
///
/// `outDetached` is true when `HEAD` is not a branch. `outBranch` is `HEAD` in that case.
SlangResult getCheckedOutBranch(
    const String& repositoryPath,
    String& outBranch,
    bool& outDetached,
    String& outError);

/// Return whether `refs/heads/<branch>` exists in `repositoryPath`.
SlangResult localBranchExists(
    const String& repositoryPath,
    const String& branch,
    bool& outExists,
    String& outError);

/// Check out an existing local branch. The branch is not created or reset.
SlangResult checkoutLocalBranch(
    const String& repositoryPath,
    const String& branch,
    String& outError);

/// Create `branch` at `commit` without moving `HEAD`.
SlangResult createLocalBranch(
    const String& repositoryPath,
    const String& branch,
    const String& commit,
    String& outError);

/// Check out `commit` with a detached `HEAD`.
SlangResult checkoutDetachedCommit(
    const String& repositoryPath,
    const String& commit,
    String& outError);

/// Discard uncommitted files, untracked files, and stashes. Commits are left in place.
SlangResult discardUncommittedState(const String& repositoryPath, String& outError);

/// Create an annotated tag at `HEAD`. The tag is local and is not pushed.
SlangResult createAnnotatedTag(const String& repositoryPath, const String& tag, String& outError);

/// List every tag name in the repository, including names that are not release tags.
SlangResult listTagNames(const String& repositoryPath, List<String>& outTags, String& outError);

/// One canonical release tag visited while walking an edit line back to its restore commit.
struct EditLineTag
{
    String tag;
    String commit;
    PackageVersion version;
};

/// Collect canonical release tags on the ancestry line from `headCommit` back to `pinCommit`.
///
/// At a merge, a parent is eligible when `pinCommit` is an ancestor of that parent. When more
/// than one parent contains the pin, the walk takes the first parent: that is the line that was
/// checked out when the merge was created. `outReachedPin` is false when the walk ends without
/// visiting `pinCommit`.
SlangResult collectCanonicalTagsOnEditLine(
    const String& repositoryPath,
    const String& headCommit,
    const String& pinCommit,
    List<EditLineTag>& outTags,
    bool& outReachedPin,
    String& outError);

/// Count commits reachable from `descendant` and not from `ancestor`.
SlangResult countCommitsAfter(
    const String& repositoryPath,
    const String& ancestor,
    const String& descendant,
    Index& outCount,
    String& outError);

/// Find the one canonical release tag that points at `HEAD`, or report that none exists.
///
/// A non-canonical release tag is not a match. When `outWarnings` is set, each such tag is
/// reported once.
SlangResult findVersionTagAtHead(
    const String& repositoryPath,
    String& outTag,
    PackageVersion& outVersion,
    bool& outFound,
    String& outError,
    List<String>* outWarnings = nullptr);

/// Resolve `revision` in `repositoryPath` to a 40-character commit ID.
SlangResult resolveLocalRevision(
    const String& repositoryPath,
    const String& revision,
    String& outCommit,
    String& outError);

/// Find the nearest canonical release tag that is an ancestor of `commit`.
///
/// Consider this example: `main` is three commits after `v1.3`. The pin still checks out `main`,
/// and this helper reports `v1.3` so the solver can treat that tree as 1.3 when `as` is omitted.
/// Tags that are not ancestors of `commit` are ignored. A non-canonical release tag, such as
/// `v1.3.0`, is not a candidate; when `outWarnings` is set it is reported once. Two equally near
/// release tags are an error.
SlangResult findNearestReleaseTag(
    const String& repositoryPath,
    const String& commit,
    String& outTag,
    PackageVersion& outVersion,
    bool& outFound,
    String& outError,
    List<String>* outWarnings = nullptr);

/// Return whether `text` is a full Git object ID (40- or 64-character hex).
bool isGitObjectId(const UnownedStringSlice& text);
inline bool isGitObjectId(const String& text)
{
    return isGitObjectId(text.getUnownedSlice());
}

/// Return the working-tree root of the Git repository that contains `workingDirectory`.
///
/// This is `git -C workingDirectory rev-parse --show-toplevel`. A nested checkout, such as a
/// materialized dependency under `deps/`, reports that checkout rather than a parent superproject.
SlangResult getGitWorkingTreeRoot(
    const String& workingDirectory,
    String& outRoot,
    String& outError);

/// Return the configured URL for the repository's `origin` remote.
SlangResult getRepositoryOrigin(const String& repositoryPath, String& outOrigin, String& outError);

/// Check out `targetCommit` at `destination` as a worktree of `cachePath`.
///
/// `cachePath` is the permanent repository for `gitURL`. A missing destination is added as a
/// detached worktree. A worktree of that same repository is checked out in place. `allowClean`
/// permits discarding uncommitted files, or detaching a worktree that belongs to a different
/// repository. Commits, branches, tags, and stashes stay in the permanent repository either way.
/// A destination that is its own Git repository is deleted only through the disclosed unpushed
/// report. `allowMovingRefs` is unused: published refs are not copied into the worktree, because
/// the worktree and the permanent repository share one ref namespace. If the checkout is already
/// clean at `targetCommit`, its work tree stays untouched and `outDidMaterialize` is false.
SlangResult materializeLockedRevision(
    const String& gitURL,
    const String& currentCommit,
    const String& targetCommit,
    const String& destination,
    bool allowClean,
    bool allowMovingRefs,
    bool& outDidMaterialize,
    String& outError,
    const String& cachePath,
    const String& disclosedUnpushedReport = String(),
    bool unpushedDeletionApproved = false);

/// List cache refs whose existing names would move when staged into `destination`.
///
/// New refs and objects are additive and are not reported. Published tags keep their
/// `refs/slang-cache/tags/*` names; origin branches keep their `refs/remotes/origin/*` names.
/// Tags under `refs/tags` are compared too, so a separate checkout that still has those names can
/// be reported. A worktree of `cachePath` shares that namespace, so this list is empty.
SlangResult collectMovingCachedRefs(
    const String& cachePath,
    const String& destination,
    List<String>& outRefs,
    String& outError);

/// Return whether removing a checkout would discard no changes, commits, or stashes.
SlangResult isWorkingTreeSafeToRemove(
    const String& repositoryPath,
    const String& expectedCommit,
    bool& outIsSafe,
    String& outError);

/// Inspect every kind of local Git state that makes a tool-owned checkout non-reproducible.
SlangResult getWorkingTreeStatus(
    const String& repositoryPath,
    const String& expectedCommit,
    GitWorkingTreeStatus& outStatus,
    String& outError);

} // namespace PackageTool
} // namespace Slang
