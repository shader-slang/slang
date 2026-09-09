// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-git.h"

#include "core/slang-io.h"
#include "core/slang-platform.h"
#include "core/slang-process-util.h"
#include "core/slang-string-util.h"

#include <stdio.h>

namespace Slang
{
namespace PackageTool
{

static SlangResult _findGitExecutable(String& outPath, String& outError)
{
    StringBuilder pathValue;
    if (SLANG_FAILED(PlatformUtil::getEnvironmentVariable(
            UnownedStringSlice::fromLiteral("PATH"),
            pathValue)))
    {
        outError = "Cannot locate git because PATH is unavailable.";
        return SLANG_FAIL;
    }

    List<UnownedStringSlice> directories;
#if SLANG_WINDOWS_FAMILY
    StringUtil::split(pathValue.getUnownedSlice(), ';', directories);
    const char* executableName = "git.exe";
#else
    StringUtil::split(pathValue.getUnownedSlice(), ':', directories);
    const char* executableName = "git";
#endif
    for (auto directory : directories)
    {
        if (directory.getLength() == 0)
            continue;
        String candidate = Path::combine(directory, executableName);
        if (File::exists(candidate))
        {
            outPath = candidate;
            return SLANG_OK;
        }
    }

    outError = "Unable to find the preinstalled git command on PATH.";
    return SLANG_FAIL;
}

static SlangResult _runGit(
    const String& workingDirectory,
    const List<String>& arguments,
    ExecuteResult& outResult,
    String& outError)
{
    static String gitExecutable;
    if (gitExecutable.getLength() == 0)
        SLANG_RETURN_ON_FAIL(_findGitExecutable(gitExecutable, outError));

    CommandLine commandLine;
    commandLine.setExecutableLocation(
        ExecutableLocation(ExecutableLocation::Type::Path, gitExecutable));
    commandLine.addArg("-C");
    commandLine.addArg(workingDirectory);
    for (const auto& argument : arguments)
        commandLine.addArg(argument);

    if (SLANG_FAILED(ProcessUtil::execute(commandLine, outResult)))
    {
        outError = "Unable to execute the preinstalled git command.";
        return SLANG_FAIL;
    }
    if (outResult.resultCode != 0)
    {
        outError = outResult.standardError.trim();
        if (outError.getLength() == 0)
            outError = String("Git command failed: ") + commandLine.toString();
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static Index _findCandidate(const List<TagCandidate>& candidates, const String& tag)
{
    for (Index i = 0; i < candidates.getCount(); ++i)
    {
        if (candidates[i].ref == tag)
            return i;
    }
    return -1;
}

SlangResult listReleaseTags(
    const String& gitURL,
    List<TagCandidate>& outCandidates,
    String& outError)
{
    List<String> arguments;
    arguments.add("ls-remote");
    arguments.add("--tags");
    arguments.add("--");
    arguments.add(gitURL);
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(".", arguments, result, outError));

    outCandidates.clear();
    static const UnownedStringSlice kPrefix("refs/tags/");
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
    {
        List<UnownedStringSlice> fields;
        StringUtil::splitOnWhitespace(line, fields);
        if (fields.getCount() != 2 || !fields[1].startsWith(kPrefix))
            continue;

        UnownedStringSlice reference = fields[1].tail(kPrefix.getLength());
        bool isPeeled = reference.endsWith("^{}");
        UnownedStringSlice tagSlice =
            isPeeled ? reference.head(reference.getLength() - 3) : reference;
        SemanticVersion version;
        if (SLANG_FAILED(parseReleaseTag(tagSlice, version)))
            continue;

        String tag(tagSlice);
        Index candidateIndex = _findCandidate(outCandidates, tag);
        if (candidateIndex < 0)
        {
            TagCandidate candidate;
            candidate.ref = tag;
            candidate.commit = fields[0];
            candidate.version = version;
            outCandidates.add(candidate);
        }
        else if (isPeeled)
        {
            outCandidates[candidateIndex].commit = fields[0];
        }
    }
    outCandidates.sort([](const TagCandidate& left, const TagCandidate& right)
                       { return left.version > right.version; });
    return SLANG_OK;
}

SlangResult resolveReference(
    const String& gitURL,
    const String& ref,
    TagCandidate& outCandidate,
    String& outError)
{
    bool isCommit = ref.getLength() == 40;
    for (Index i = 0; isCommit && i < ref.getLength(); ++i)
    {
        char c = ref[i];
        isCommit = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }
    if (isCommit)
    {
        // A commit pin need not be an advertised branch or tag tip. The later manifest load
        // verifies that the object is reachable from the configured repository.
        outCandidate = TagCandidate();
        outCandidate.ref = ref;
        outCandidate.commit = ref;
        return SLANG_OK;
    }

    List<String> arguments;
    arguments.add("ls-remote");
    arguments.add("--");
    arguments.add(gitURL);
    arguments.add(ref);
    if (ref.getUnownedSlice().startsWith("refs/tags/"))
    {
        arguments.add(ref + "^{}");
    }
    else if (!ref.getUnownedSlice().startsWith("refs/") && ref != "HEAD")
    {
        arguments.add(String("refs/heads/") + ref);
        arguments.add(String("refs/tags/") + ref);
        arguments.add(String("refs/tags/") + ref + "^{}");
    }
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(".", arguments, result, outError));

    String branchCommit;
    String tagCommit;
    String directCommit;
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
    {
        List<UnownedStringSlice> fields;
        StringUtil::splitOnWhitespace(line, fields);
        if (fields.getCount() != 2)
            continue;
        String reference(fields[1]);
        String commit(fields[0]);
        if (reference == ref || (ref == "HEAD" && reference == "HEAD"))
            directCommit = commit;
        else if (reference == String("refs/heads/") + ref)
            branchCommit = commit;
        else if (reference == String("refs/tags/") + ref)
        {
            if (!tagCommit.getLength())
                tagCommit = commit;
        }
        else if (reference == String("refs/tags/") + ref + "^{}")
            tagCommit = commit;
        else if (ref.getUnownedSlice().startsWith("refs/tags/") && reference == ref + "^{}")
            directCommit = commit;
    }
    if (branchCommit.getLength() && tagCommit.getLength())
    {
        outError = String("Git ref is ambiguous between a branch and tag; use a full ref: ") + ref;
        return SLANG_FAIL;
    }
    String commit = directCommit.getLength()
                        ? directCommit
                        : (branchCommit.getLength() ? branchCommit : tagCommit);
    if (!commit.getLength())
    {
        outError = String("Git ref does not exist: ") + ref;
        return SLANG_FAIL;
    }
    outCandidate = TagCandidate();
    outCandidate.ref = ref;
    outCandidate.commit = commit;
    return SLANG_OK;
}

static SlangResult _ensureRepository(
    const String& workingDirectory,
    const String& gitURL,
    const String& repositoryPath,
    bool canReplace,
    String& outError)
{
    ExecuteResult result;
    if (!File::exists(Path::combine(repositoryPath, ".git")))
    {
        SlangPathType pathType;
        if (SLANG_SUCCEEDED(Path::getPathType(repositoryPath, &pathType)))
        {
            outError = String("Package cache path is not a Git repository: ") + repositoryPath;
            return SLANG_FAIL;
        }
        List<String> cloneArguments;
        cloneArguments.add("clone");
        cloneArguments.add("--no-checkout");
        cloneArguments.add("--");
        cloneArguments.add(gitURL);
        cloneArguments.add(repositoryPath);
        SLANG_RETURN_ON_FAIL(_runGit(workingDirectory, cloneArguments, result, outError));
    }

    List<String> remoteArguments;
    remoteArguments.add("remote");
    remoteArguments.add("get-url");
    remoteArguments.add("origin");
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, remoteArguments, result, outError));
    if (String(result.standardOutput.trim()) != gitURL)
    {
        if (!canReplace)
        {
            outError = String("Git reports a different origin after replacing package cache: ") +
                       repositoryPath;
            return SLANG_FAIL;
        }
        if (SLANG_FAILED(Path::removeNonEmpty(repositoryPath)))
        {
            outError = String("Cannot replace stale package cache: ") + repositoryPath;
            return SLANG_FAIL;
        }
        return _ensureRepository(workingDirectory, gitURL, repositoryPath, false, outError);
    }

    List<String> fetchArguments;
    fetchArguments.add("fetch");
    fetchArguments.add("--tags");
    fetchArguments.add("--force");
    fetchArguments.add("origin");
    return _runGit(repositoryPath, fetchArguments, result, outError);
}

SlangResult ensureRepository(
    const String& workingDirectory,
    const String& gitURL,
    const String& repositoryPath,
    String& outError)
{
    return _ensureRepository(workingDirectory, gitURL, repositoryPath, true, outError);
}

SlangResult readFileAtRevision(
    const String& repositoryPath,
    const String& revision,
    const String& filePath,
    String& outContents,
    String& outError)
{
    List<String> arguments;
    arguments.add("show");
    arguments.add(revision + ":" + filePath);
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    outContents = result.standardOutput;
    return SLANG_OK;
}

SlangResult getRepositoryHeadCommit(
    const String& repositoryPath,
    String& outCommit,
    String& outError)
{
    List<String> arguments;
    arguments.add("rev-parse");
    arguments.add("HEAD");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    outCommit = result.standardOutput.trim();
    return SLANG_OK;
}

bool isGitObjectId(const UnownedStringSlice& text)
{
    if (text.getLength() != 40 && text.getLength() != 64)
        return false;
    for (Index i = 0; i < text.getLength(); ++i)
    {
        char c = text[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            return false;
    }
    return true;
}

SlangResult resolveLocalRevision(
    const String& repositoryPath,
    const String& revision,
    String& outCommit,
    String& outError)
{
    List<String> arguments;
    arguments.add("rev-parse");
    arguments.add("--verify");
    arguments.add("--end-of-options");
    arguments.add(revision + "^{commit}");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    outCommit = result.standardOutput.trim();
    return SLANG_OK;
}

SlangResult findVersionTagAtHead(
    const String& repositoryPath,
    String& outTag,
    SemanticVersion& outVersion,
    bool& outFound,
    String& outError)
{
    List<String> arguments;
    arguments.add("tag");
    arguments.add("--points-at");
    arguments.add("HEAD");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));

    outFound = false;
    outTag = String();
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
    {
        String tag = line.trim();
        SemanticVersion version;
        if (!tag.getLength() || SLANG_FAILED(parseReleaseTag(tag, version)))
            continue;
        if (outFound)
        {
            outError = "HEAD has multiple semantic-version tags; pass --as explicitly.";
            return SLANG_FAIL;
        }
        outFound = true;
        outTag = tag;
        outVersion = version;
    }
    return SLANG_OK;
}

SlangResult findNearestReleaseTag(
    const String& repositoryPath,
    const String& commit,
    String& outTag,
    SemanticVersion& outVersion,
    bool& outFound,
    String& outError)
{
    outFound = false;
    outTag = String();
    List<String> arguments;
    arguments.add("tag");
    arguments.add("--merged");
    arguments.add(commit);
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));

    String nearestTag;
    SemanticVersion nearestVersion;
    Int nearestDistance = 0;
    bool haveNearest = false;
    bool nearestTied = false;
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
    {
        String tag = line.trim();
        SemanticVersion version;
        if (!tag.getLength() || SLANG_FAILED(parseReleaseTag(tag, version)))
            continue;

        List<String> countArguments;
        countArguments.add("rev-list");
        countArguments.add("--count");
        countArguments.add(tag + ".." + commit);
        ExecuteResult countResult;
        SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, countArguments, countResult, outError));
        String countText = countResult.standardOutput.trim();
        Int distance = 0;
        if (SLANG_FAILED(StringUtil::parseInt(countText.getUnownedSlice(), distance)) ||
            distance < 0)
        {
            outError =
                String("Cannot measure distance from tag '") + tag + "' to commit " + commit + ".";
            return SLANG_FAIL;
        }

        if (!haveNearest || distance < nearestDistance)
        {
            haveNearest = true;
            nearestTied = false;
            nearestDistance = distance;
            nearestTag = tag;
            nearestVersion = version;
        }
        else if (distance == nearestDistance && (nearestTag != tag || nearestVersion != version))
        {
            nearestTied = true;
        }
    }
    if (!haveNearest)
        return SLANG_OK;
    if (nearestTied)
    {
        outError = "Commit has more than one equally near semantic-version tag; pass --as "
                   "explicitly.";
        return SLANG_FAIL;
    }
    outFound = true;
    outTag = nearestTag;
    outVersion = nearestVersion;
    return SLANG_OK;
}

SlangResult getGitWorkingTreeRoot(const String& workingDirectory, String& outRoot, String& outError)
{
    List<String> arguments;
    arguments.add("rev-parse");
    arguments.add("--show-toplevel");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(workingDirectory, arguments, result, outError));
    String gitRoot = result.standardOutput.trim();
    if (gitRoot.getLength() == 0)
    {
        outError = String("Git did not report a working-tree root for: ") + workingDirectory;
        return SLANG_FAIL;
    }
    if (SLANG_FAILED(Path::getCanonical(gitRoot, outRoot)))
    {
        outError = String("Cannot canonicalize the Git working-tree root: ") + gitRoot;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult getRepositoryOrigin(const String& repositoryPath, String& outOrigin, String& outError)
{
    List<String> arguments;
    arguments.add("remote");
    arguments.add("get-url");
    arguments.add("origin");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    outOrigin = result.standardOutput.trim();
    return SLANG_OK;
}

static SlangResult _materializeRevision(
    const String& workingDirectory,
    const String& gitURL,
    const String& currentCommit,
    const String& targetCommit,
    const String& destination,
    bool allowClean,
    bool& ioDidMaterialize,
    String& outError)
{
    ExecuteResult result;
    SlangPathType pathType;
    bool destinationExisted = SLANG_SUCCEEDED(Path::getPathType(destination, &pathType));
    if (!destinationExisted)
    {
        List<String> cloneArguments;
        cloneArguments.add("clone");
        cloneArguments.add("--no-checkout");
        cloneArguments.add("--");
        cloneArguments.add(gitURL);
        cloneArguments.add(destination);
        SLANG_RETURN_ON_FAIL(_runGit(workingDirectory, cloneArguments, result, outError));
        ioDidMaterialize = true;
    }
    else if (!File::exists(Path::combine(destination, ".git")))
    {
        if (!allowClean)
        {
            outError = String("Package destination is not a Git repository; refusing to replace "
                              "it without --clean: ") +
                       destination;
            return SLANG_FAIL;
        }
        if (SLANG_FAILED(Path::removeNonEmpty(destination)))
        {
            outError = String("Cannot replace package destination: ") + destination;
            return SLANG_FAIL;
        }
        return _materializeRevision(
            workingDirectory,
            gitURL,
            String(),
            targetCommit,
            destination,
            false,
            ioDidMaterialize,
            outError);
    }

    List<String> remoteArguments;
    remoteArguments.add("remote");
    remoteArguments.add("get-url");
    remoteArguments.add("origin");
    SLANG_RETURN_ON_FAIL(_runGit(destination, remoteArguments, result, outError));
    if (String(result.standardOutput.trim()) != gitURL)
    {
        if (!allowClean)
        {
            outError = String("Package checkout has a different origin; refusing to replace it "
                              "without --clean: ") +
                       destination;
            return SLANG_FAIL;
        }
        if (SLANG_FAILED(Path::removeNonEmpty(destination)))
        {
            outError = String("Cannot replace package checkout: ") + destination;
            return SLANG_FAIL;
        }
        return _materializeRevision(
            workingDirectory,
            gitURL,
            String(),
            targetCommit,
            destination,
            false,
            ioDidMaterialize,
            outError);
    }

    if (destinationExisted && currentCommit.getLength())
    {
        GitWorkingTreeStatus status;
        SLANG_RETURN_ON_FAIL(getWorkingTreeStatus(destination, currentCommit, status, outError));
        const bool hasUncommittedState = status.changedFileCount != 0 || status.stashCount != 0;
        if (!hasUncommittedState && status.headCommit == targetCommit)
            return SLANG_OK;

        const bool isSafe = !hasUncommittedState && status.commitsAhead == 0 &&
                            status.commitsBehind == 0 && status.headCommit == currentCommit;
        if (!isSafe)
        {
            if (!allowClean)
            {
                outError = String("Package checkout has changed files, commits, or stashes; "
                                  "refusing to replace it without --clean: ") +
                           destination;
                return SLANG_FAIL;
            }
            if (SLANG_FAILED(Path::removeNonEmpty(destination)))
            {
                outError = String("Cannot replace package checkout: ") + destination;
                return SLANG_FAIL;
            }
            return _materializeRevision(
                workingDirectory,
                gitURL,
                String(),
                targetCommit,
                destination,
                false,
                ioDidMaterialize,
                outError);
        }
    }
    else if (destinationExisted)
    {
        if (!allowClean)
        {
            outError = String("Package checkout is not owned by the current lock; refusing to "
                              "replace it without --clean: ") +
                       destination;
            return SLANG_FAIL;
        }
        if (SLANG_FAILED(Path::removeNonEmpty(destination)))
        {
            outError = String("Cannot replace package checkout: ") + destination;
            return SLANG_FAIL;
        }
        return _materializeRevision(
            workingDirectory,
            gitURL,
            String(),
            targetCommit,
            destination,
            false,
            ioDidMaterialize,
            outError);
    }

    ioDidMaterialize = true;
    List<String> fetchArguments;
    fetchArguments.add("fetch");
    fetchArguments.add("origin");
    fetchArguments.add(targetCommit);
    SLANG_RETURN_ON_FAIL(_runGit(destination, fetchArguments, result, outError));

    List<String> checkoutArguments;
    checkoutArguments.add("checkout");
    checkoutArguments.add("--detach");
    checkoutArguments.add(targetCommit);
    return _runGit(destination, checkoutArguments, result, outError);
}

SlangResult materializeRevision(
    const String& workingDirectory,
    const String& gitURL,
    const String& revision,
    const String& destination,
    String& outError)
{
    bool didMaterialize = false;
    return _materializeRevision(
        workingDirectory,
        gitURL,
        revision,
        revision,
        destination,
        false,
        didMaterialize,
        outError);
}

SlangResult materializeLockedRevision(
    const String& workingDirectory,
    const String& gitURL,
    const String& currentCommit,
    const String& targetCommit,
    const String& destination,
    bool allowClean,
    bool& outDidMaterialize,
    String& outError)
{
    outDidMaterialize = false;
    return _materializeRevision(
        workingDirectory,
        gitURL,
        currentCommit,
        targetCommit,
        destination,
        allowClean,
        outDidMaterialize,
        outError);
}

SlangResult getWorkingTreeStatus(
    const String& repositoryPath,
    const String& expectedCommit,
    GitWorkingTreeStatus& outStatus,
    String& outError)
{
    outStatus = GitWorkingTreeStatus();
    List<String> arguments;
    arguments.add("status");
    arguments.add("--porcelain");
    arguments.add("--untracked-files=normal");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
        if (line.trim().getLength())
            ++outStatus.changedFileCount;

    arguments.clear();
    arguments.add("rev-parse");
    arguments.add("HEAD");
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    outStatus.headCommit = result.standardOutput.trim();

    if (outStatus.headCommit != expectedCommit)
    {
        arguments.clear();
        arguments.add("rev-list");
        arguments.add("--left-right");
        arguments.add("--count");
        arguments.add(expectedCommit + "...HEAD");
        SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
        long long behind = 0;
        long long ahead = 0;
        if (sscanf(result.standardOutput.getBuffer(), "%lld %lld", &behind, &ahead) == 2)
        {
            outStatus.commitsBehind = Index(behind);
            outStatus.commitsAhead = Index(ahead);
        }
    }

    arguments.clear();
    arguments.add("stash");
    arguments.add("list");
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
        if (line.trim().getLength())
            ++outStatus.stashCount;
    return SLANG_OK;
}

SlangResult isWorkingTreeSafeToRemove(
    const String& repositoryPath,
    const String& expectedCommit,
    bool& outIsSafe,
    String& outError)
{
    GitWorkingTreeStatus status;
    SLANG_RETURN_ON_FAIL(getWorkingTreeStatus(repositoryPath, expectedCommit, status, outError));
    outIsSafe = status.changedFileCount == 0 && status.commitsAhead == 0 &&
                status.commitsBehind == 0 && status.stashCount == 0 &&
                status.headCommit == expectedCommit;
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
