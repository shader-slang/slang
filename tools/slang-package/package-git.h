#pragma once

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

/// List canonical dotted release tags already present in a local clone, without contacting a
/// remote.
///
/// A tag that names a release with a non-canonical spelling, such as `v1.2.0` or `v01.2.3`, is
/// omitted. When `outWarnings` is set, each omitted tag is reported once.
SlangResult listReleaseTagsFromRepository(
    const String& repositoryPath,
    List<TagCandidate>& outCandidates,
    String& outError,
    List<String>* outWarnings = nullptr);

/// Resolve `ref` from the origin-tracking refs and objects already present in a package cache.
SlangResult resolveCachedReference(
    const String& repositoryPath,
    const String& ref,
    TagCandidate& outCandidate,
    String& outError);

/// Clone or refresh a package cache from its origin.
SlangResult refreshPackageCache(
    const String& workingDirectory,
    const String& gitURL,
    const String& repositoryPath,
    String& outError);

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

/// Materialize `targetCommit` without discarding an existing checkout's work.
///
/// If `destination` exists, it must be clean at `currentCommit`. `allowClean` explicitly permits
/// deleting and recreating a checkout that has changed files, commits, stashes, or a different
/// origin. `allowMovingRefs` permits cache staging to change existing named refs after the caller
/// has disclosed and confirmed those moves. If the checkout is already clean at `targetCommit`,
/// its work tree stays untouched and `outDidMaterialize` is false, although additive cached refs
/// may still be staged.
SlangResult materializeLockedRevision(
    const String& gitURL,
    const String& currentCommit,
    const String& targetCommit,
    const String& destination,
    bool allowClean,
    bool allowMovingRefs,
    bool& outDidMaterialize,
    String& outError,
    const String& cachePath);

/// List cache refs whose existing names would move when staged into `destination`.
///
/// New refs and objects are additive and are not reported. Tags keep their `refs/tags/*` names;
/// origin branches keep their `refs/remotes/origin/*` names.
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
