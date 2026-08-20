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

SlangResult materializeRevision(
    const String& workingDirectory,
    const String& gitURL,
    const String& revision,
    const String& destination,
    String& outError);

/// Return whether removing a checkout would discard no changes, commits, or stashes.
SlangResult isWorkingTreeSafeToRemove(
    const String& repositoryPath,
    const String& expectedCommit,
    bool& outIsSafe,
    String& outError);

} // namespace PackageTool
} // namespace Slang

#endif
