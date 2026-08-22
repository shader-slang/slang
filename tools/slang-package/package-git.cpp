// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-git.h"

#include "core/slang-io.h"
#include "core/slang-platform.h"
#include "core/slang-process-util.h"
#include "core/slang-string-util.h"

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
        if (candidates[i].tag == tag)
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
            candidate.tag = tag;
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

static SlangResult _materializeRevision(
    const String& workingDirectory,
    const String& gitURL,
    const String& revision,
    const String& destination,
    bool canReplace,
    String& outError)
{
    ExecuteResult result;
    SlangPathType pathType;
    if (SLANG_FAILED(Path::getPathType(destination, &pathType)))
    {
        List<String> cloneArguments;
        cloneArguments.add("clone");
        cloneArguments.add("--no-checkout");
        cloneArguments.add("--");
        cloneArguments.add(gitURL);
        cloneArguments.add(destination);
        SLANG_RETURN_ON_FAIL(_runGit(workingDirectory, cloneArguments, result, outError));
    }
    else if (!File::exists(Path::combine(destination, ".git")))
    {
        outError = String("Package destination is not a Git repository: ") + destination;
        return SLANG_FAIL;
    }

    List<String> remoteArguments;
    remoteArguments.add("remote");
    remoteArguments.add("get-url");
    remoteArguments.add("origin");
    SLANG_RETURN_ON_FAIL(_runGit(destination, remoteArguments, result, outError));
    if (String(result.standardOutput.trim()) != gitURL)
    {
        if (!canReplace)
        {
            outError = String("Git reports a different origin after replacing package checkout: ") +
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
            revision,
            destination,
            false,
            outError);
    }

    List<String> fetchArguments;
    fetchArguments.add("fetch");
    fetchArguments.add("origin");
    fetchArguments.add(revision);
    SLANG_RETURN_ON_FAIL(_runGit(destination, fetchArguments, result, outError));

    List<String> checkoutArguments;
    checkoutArguments.add("checkout");
    checkoutArguments.add("--detach");
    checkoutArguments.add("--force");
    checkoutArguments.add(revision);
    SLANG_RETURN_ON_FAIL(_runGit(destination, checkoutArguments, result, outError));

    List<String> cleanArguments;
    cleanArguments.add("clean");
    cleanArguments.add("-d");
    cleanArguments.add("-f");
    cleanArguments.add("-x");
    return _runGit(destination, cleanArguments, result, outError);
}

SlangResult materializeRevision(
    const String& workingDirectory,
    const String& gitURL,
    const String& revision,
    const String& destination,
    String& outError)
{
    return _materializeRevision(workingDirectory, gitURL, revision, destination, true, outError);
}

SlangResult isWorkingTreeSafeToRemove(
    const String& repositoryPath,
    const String& expectedCommit,
    bool& outIsSafe,
    String& outError)
{
    List<String> arguments;
    arguments.add("status");
    arguments.add("--porcelain");
    arguments.add("--untracked-files=normal");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    if (result.standardOutput.trim().getLength() != 0)
    {
        outIsSafe = false;
        return SLANG_OK;
    }

    arguments.clear();
    arguments.add("rev-parse");
    arguments.add("HEAD");
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    if (String(result.standardOutput.trim()) != expectedCommit)
    {
        outIsSafe = false;
        return SLANG_OK;
    }

    arguments.clear();
    arguments.add("stash");
    arguments.add("list");
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    outIsSafe = result.standardOutput.trim().getLength() == 0;
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
