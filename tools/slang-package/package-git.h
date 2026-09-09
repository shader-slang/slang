// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_GIT_H
#define SLANG_PACKAGE_GIT_H

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

SlangResult listReleaseTags(
    const String& gitURL,
    List<TagCandidate>& outCandidates,
    String& outError);

/// Resolve an opaque branch or tag name to the commit currently advertised by the remote.
SlangResult resolveReference(
    const String& gitURL,
    const String& ref,
    TagCandidate& outCandidate,
    String& outError);

SlangResult ensureRepository(
    const String& workingDirectory,
    const String& gitURL,
    const String& repositoryPath,
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

/// Find the one semantic-version tag that points at `HEAD`, or report that none exists.
SlangResult findVersionTagAtHead(
    const String& repositoryPath,
    String& outTag,
    SemanticVersion& outVersion,
    bool& outFound,
    String& outError);

/// Resolve `revision` in `repositoryPath` to a 40-character commit ID.
SlangResult resolveLocalRevision(
    const String& repositoryPath,
    const String& revision,
    String& outCommit,
    String& outError);

/// Find the nearest `vMAJOR.MINOR.PATCH` tag that is an ancestor of `commit`.
///
/// Consider this example: `main` is three commits after `v1.3.0`. The pin still checks out
/// `main`, and this helper reports `v1.3.0` so the solver can treat that tree as 1.3.0 when `as`
/// is omitted. Tags that are not ancestors of `commit` are ignored. Two equally near release tags
/// are an error.
SlangResult findNearestReleaseTag(
    const String& repositoryPath,
    const String& commit,
    String& outTag,
    SemanticVersion& outVersion,
    bool& outFound,
    String& outError);

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

SlangResult materializeRevision(
    const String& workingDirectory,
    const String& gitURL,
    const String& revision,
    const String& destination,
    String& outError);

/// Materialize `targetCommit` without discarding an existing checkout's work.
///
/// If `destination` exists, it must be clean at `currentCommit`. `allowClean` explicitly permits
/// deleting and cloning a checkout that has changed files, commits, stashes, or a different origin.
/// If the checkout is already clean at `targetCommit`, leave it untouched and set
/// `outDidMaterialize` to false.
SlangResult materializeLockedRevision(
    const String& workingDirectory,
    const String& gitURL,
    const String& currentCommit,
    const String& targetCommit,
    const String& destination,
    bool allowClean,
    bool& outDidMaterialize,
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

#endif
