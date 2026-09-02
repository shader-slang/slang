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
