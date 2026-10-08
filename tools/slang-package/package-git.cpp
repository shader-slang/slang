// package-git.cpp

#include "package-git.h"

#include "core/slang-command-line.h"
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

static SlangResult _executeGit(
    const String& workingDirectory,
    const List<String>& arguments,
    CommandLine& outCommandLine,
    ExecuteResult& outResult,
    String& outError)
{
    static String gitExecutable;
    if (gitExecutable.getLength() == 0)
        SLANG_RETURN_ON_FAIL(_findGitExecutable(gitExecutable, outError));

    outCommandLine = CommandLine();
    outCommandLine.setExecutableLocation(
        ExecutableLocation(ExecutableLocation::Type::Path, gitExecutable));
    outCommandLine.addArg("-C");
    outCommandLine.addArg(workingDirectory);
    for (const auto& argument : arguments)
        outCommandLine.addArg(argument);

    if (SLANG_FAILED(ProcessUtil::execute(outCommandLine, outResult)))
    {
        outError = "Unable to execute the preinstalled git command.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _runGit(
    const String& workingDirectory,
    const List<String>& arguments,
    ExecuteResult& outResult,
    String& outError)
{
    CommandLine commandLine;
    SLANG_RETURN_ON_FAIL(
        _executeGit(workingDirectory, arguments, commandLine, outResult, outError));
    if (outResult.resultCode != 0)
    {
        outError = outResult.standardError.trim();
        if (outError.getLength() == 0)
            outError = String("Git command failed: ") + commandLine.toString();
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _runGitCode(
    const String& workingDirectory,
    const List<String>& arguments,
    int& outCode,
    ExecuteResult& outResult,
    String& outError)
{
    CommandLine commandLine;
    SLANG_RETURN_ON_FAIL(
        _executeGit(workingDirectory, arguments, commandLine, outResult, outError));
    outCode = (int)outResult.resultCode;
    return SLANG_OK;
}

static String _offlineUpdateAdvice()
{
    return "run 'slang package update' without --offline";
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

static SlangResult _parseReleaseTagLines(
    const String& text,
    List<TagCandidate>& outCandidates,
    String& outError,
    List<String>* outWarnings)
{
    SLANG_UNUSED(outError);
    outCandidates.clear();
    static const UnownedStringSlice kPrefix("refs/tags/");
    for (auto line : LineParser(text.getUnownedSlice()))
    {
        List<UnownedStringSlice> fields;
        StringUtil::splitOnWhitespace(line, fields);
        if (fields.getCount() != 2 || !fields[1].startsWith(kPrefix))
            continue;

        UnownedStringSlice reference = fields[1].tail(kPrefix.getLength());
        bool isPeeled = reference.endsWith("^{}");
        UnownedStringSlice tagSlice =
            isPeeled ? reference.head(reference.getLength() - 3) : reference;
        PackageVersion version;
        if (!acceptCanonicalReleaseTag(tagSlice, version, outWarnings))
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

SlangResult listReleaseTagsFromRepository(
    const String& repositoryPath,
    List<TagCandidate>& outCandidates,
    String& outError,
    List<String>* outWarnings)
{
    List<String> arguments;
    arguments.add("show-ref");
    arguments.add("--tags");
    arguments.add("--dereference");
    ExecuteResult result;
    CommandLine commandLine;
    SLANG_RETURN_ON_FAIL(_executeGit(repositoryPath, arguments, commandLine, result, outError));
    if (result.resultCode != 0)
    {
        if (result.standardOutput.trim().getLength() != 0)
        {
            outError = result.standardError.trim();
            if (outError.getLength() == 0)
                outError = String("Git command failed: ") + commandLine.toString();
            return SLANG_FAIL;
        }
        if (result.standardError.trim().getLength() != 0)
        {
            outError = result.standardError.trim();
            return SLANG_FAIL;
        }
        outCandidates.clear();
        return SLANG_OK;
    }
    return _parseReleaseTagLines(result.standardOutput, outCandidates, outError, outWarnings);
}

static SlangResult _selectCommitFromRefLines(
    const String& text,
    const String& ref,
    String& outCommit,
    String& outError)
{
    String branchCommit;
    String tagCommit;
    String directCommit;
    for (auto line : LineParser(text.getUnownedSlice()))
    {
        List<UnownedStringSlice> fields;
        StringUtil::splitOnWhitespace(line, fields);
        if (fields.getCount() != 2)
            continue;
        String reference(fields[1]);
        String commit(fields[0]);
        if (reference == ref)
            directCommit = commit;
        else if (reference == String("refs/remotes/origin/") + ref)
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
    outCommit = directCommit.getLength() ? directCommit
                                         : (branchCommit.getLength() ? branchCommit : tagCommit);
    return SLANG_OK;
}

SlangResult resolveCachedReference(
    const String& repositoryPath,
    const String& ref,
    TagCandidate& outCandidate,
    String& outError)
{
    if (isGitObjectId(ref))
    {
        String commit;
        if (SLANG_FAILED(resolveLocalRevision(repositoryPath, ref, commit, outError)))
        {
            outError = String("Git commit is not in the local cache; ") + _offlineUpdateAdvice() +
                       ": " + ref;
            return SLANG_FAIL;
        }
        outCandidate = TagCandidate();
        outCandidate.ref = ref;
        outCandidate.commit = commit;
        return SLANG_OK;
    }

    List<String> arguments;
    arguments.add("show-ref");
    arguments.add("--dereference");
    arguments.add("--");
    if (ref == "HEAD")
    {
        arguments.add("refs/slang-cache/origin/HEAD");
        arguments.add("refs/remotes/origin/HEAD");
    }
    else if (ref.getUnownedSlice().startsWith("refs/tags/"))
    {
        arguments.add(ref);
    }
    else if (ref.getUnownedSlice().startsWith("refs/heads/"))
    {
        arguments.add(String("refs/remotes/origin/") + ref.getUnownedSlice().tail(11));
    }
    else if (!ref.getUnownedSlice().startsWith("refs/"))
    {
        arguments.add(String("refs/tags/") + ref);
        arguments.add(String("refs/remotes/origin/") + ref);
    }
    else
    {
        arguments.add(ref);
    }

    ExecuteResult result;
    CommandLine commandLine;
    SLANG_RETURN_ON_FAIL(_executeGit(repositoryPath, arguments, commandLine, result, outError));

    String commit;
    String lookupRef = ref;
    if (ref == "HEAD")
    {
        lookupRef = "refs/slang-cache/origin/HEAD";
        SLANG_RETURN_ON_FAIL(
            _selectCommitFromRefLines(result.standardOutput, lookupRef, commit, outError));
        if (!commit.getLength())
            lookupRef = "refs/remotes/origin/HEAD";
    }
    else if (ref.getUnownedSlice().startsWith("refs/heads/"))
        lookupRef = String("refs/remotes/origin/") + ref.getUnownedSlice().tail(11);
    if (!commit.getLength())
        SLANG_RETURN_ON_FAIL(
            _selectCommitFromRefLines(result.standardOutput, lookupRef, commit, outError));
    if (!commit.getLength())
    {
        outError = String("Git ref does not exist in the local cache; ") + _offlineUpdateAdvice() +
                   ": " + ref;
        return SLANG_FAIL;
    }
    outCandidate = TagCandidate();
    outCandidate.ref = ref;
    outCandidate.commit = commit;
    return SLANG_OK;
}

static bool _hasGitDir(const String& path);

static SlangResult _gitOutputLines(
    const String& repositoryPath,
    const List<String>& arguments,
    List<String>& outLines,
    String& outError)
{
    outLines.clear();
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
    {
        String text = line.trim();
        if (text.getLength())
            outLines.add(text);
    }
    return SLANG_OK;
}

static SlangResult _parseNonNegativeCount(const String& text, Index& outCount, String& outError)
{
    if (!text.getLength())
    {
        outError = "Git returned an empty commit count.";
        return SLANG_FAIL;
    }
    Index count = 0;
    for (auto c : text.getUnownedSlice())
    {
        if (c < '0' || c > '9')
        {
            outError = String("Git returned '") + text + "'.";
            return SLANG_FAIL;
        }
        count = count * 10 + Index(c - '0');
    }
    outCount = count;
    return SLANG_OK;
}

static String _shortObjectId(const String& objectId)
{
    return objectId.getLength() <= 12 ? objectId : String(objectId.getUnownedSlice().head(12));
}

static String _replacementPackageCachePath(const String& canonicalPath)
{
    return Path::combine(
        Path::combine(Path::getParentDirectory(canonicalPath), ".replacements"),
        Path::getFileName(canonicalPath));
}

static SlangResult _countCommitsNotOnRemotes(
    const String& repositoryPath,
    const String& revision,
    bool remoteTrackingRefsExist,
    Index& outCount,
    String& outError)
{
    List<String> arguments;
    arguments.add("rev-list");
    arguments.add("--count");
    arguments.add(revision);
    if (remoteTrackingRefsExist)
    {
        arguments.add("--not");
        arguments.add("--remotes");
    }
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    return _parseNonNegativeCount(result.standardOutput.trim(), outCount, outError);
}

static SlangResult _describeRevisionTip(
    const String& repositoryPath,
    const String& revision,
    String& outSummary,
    String& outError)
{
    List<String> arguments;
    arguments.add("log");
    arguments.add("-1");
    arguments.add("--format=%h %s");
    arguments.add(revision);
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    outSummary = result.standardOutput.trim();
    return SLANG_OK;
}

static void _appendUnpushedCommits(
    StringBuilder& report,
    const char* label,
    Index count,
    const String& summary)
{
    report << "  " << label << ": " << count << (count == 1 ? " commit" : " commits")
           << " not on any remote (" << summary << ")\n";
}

SlangResult collectUnpushedRepositoryReport(
    const String& repositoryPath,
    String& outReport,
    String& outError)
{
    outReport = String();
    if (!_hasGitDir(repositoryPath))
    {
        outError = String("Cannot inspect Git history because this path is not a repository: ") +
                   repositoryPath;
        return SLANG_FAIL;
    }

    List<String> remoteArguments;
    remoteArguments.add("remote");
    List<String> remotes;
    SLANG_RETURN_ON_FAIL(_gitOutputLines(repositoryPath, remoteArguments, remotes, outError));

    for (const auto& remote : remotes)
    {
        List<String> fetchArguments;
        fetchArguments.add("fetch");
        fetchArguments.add("-q");
        fetchArguments.add("--prune");
        fetchArguments.add("--no-tags");
        fetchArguments.add(remote);
        fetchArguments.add(String("+refs/heads/*:refs/remotes/") + remote + "/*");
        ExecuteResult fetchResult;
        if (SLANG_FAILED(_runGit(repositoryPath, fetchArguments, fetchResult, outError)))
        {
            outError = String("Cannot check whether ") + repositoryPath +
                       " has commits or tags that are not on a remote: " + outError +
                       " The repository was left in place.";
            return SLANG_FAIL;
        }
    }

    bool remoteTrackingRefsExist = false;
    if (remotes.getCount())
    {
        List<String> trackingArguments;
        trackingArguments.add("for-each-ref");
        trackingArguments.add("--format=%(refname)");
        trackingArguments.add("refs/remotes");
        List<String> trackingRefs;
        SLANG_RETURN_ON_FAIL(
            _gitOutputLines(repositoryPath, trackingArguments, trackingRefs, outError));
        remoteTrackingRefsExist = trackingRefs.getCount() != 0;
    }

    StringBuilder report;
    List<String> branchArguments;
    branchArguments.add("for-each-ref");
    branchArguments.add("--format=%(refname:short)");
    branchArguments.add("refs/heads");
    List<String> branches;
    SLANG_RETURN_ON_FAIL(_gitOutputLines(repositoryPath, branchArguments, branches, outError));
    for (const auto& branch : branches)
    {
        Index count = 0;
        SLANG_RETURN_ON_FAIL(_countCommitsNotOnRemotes(
            repositoryPath,
            branch,
            remoteTrackingRefsExist,
            count,
            outError));
        if (!count)
            continue;
        String summary;
        SLANG_RETURN_ON_FAIL(_describeRevisionTip(repositoryPath, branch, summary, outError));
        StringBuilder label;
        label << "branch " << branch;
        _appendUnpushedCommits(report, label.getBuffer(), count, summary);
    }

    String headBranch;
    bool detached = false;
    SLANG_RETURN_ON_FAIL(getCheckedOutBranch(repositoryPath, headBranch, detached, outError));
    if (detached)
    {
        Index count = 0;
        SLANG_RETURN_ON_FAIL(_countCommitsNotOnRemotes(
            repositoryPath,
            "HEAD",
            remoteTrackingRefsExist,
            count,
            outError));
        if (count)
        {
            String summary;
            SLANG_RETURN_ON_FAIL(_describeRevisionTip(repositoryPath, "HEAD", summary, outError));
            _appendUnpushedCommits(report, "detached HEAD", count, summary);
        }
    }

    List<String> stashArguments;
    stashArguments.add("rev-parse");
    stashArguments.add("--verify");
    stashArguments.add("--quiet");
    stashArguments.add("refs/stash");
    int stashCode = 1;
    ExecuteResult stashResult;
    SLANG_RETURN_ON_FAIL(
        _runGitCode(repositoryPath, stashArguments, stashCode, stashResult, outError));
    if (stashCode == 0)
    {
        Index count = 0;
        SLANG_RETURN_ON_FAIL(_countCommitsNotOnRemotes(
            repositoryPath,
            "refs/stash",
            remoteTrackingRefsExist,
            count,
            outError));
        if (count)
        {
            String summary;
            SLANG_RETURN_ON_FAIL(
                _describeRevisionTip(repositoryPath, "refs/stash", summary, outError));
            _appendUnpushedCommits(report, "stash", count, summary);
        }
    }

    struct RemoteTag
    {
        String remote;
        String name;
        String peeled;
    };
    List<RemoteTag> remoteTags;
    for (const auto& remote : remotes)
    {
        List<String> tagArguments;
        tagArguments.add("ls-remote");
        tagArguments.add("--tags");
        tagArguments.add(remote);
        ExecuteResult tagResult;
        if (SLANG_FAILED(_runGit(repositoryPath, tagArguments, tagResult, outError)))
        {
            outError = String("Cannot check whether ") + repositoryPath +
                       " has tags that are not on a remote: " + outError +
                       " The repository was left in place.";
            return SLANG_FAIL;
        }
        for (auto line : LineParser(tagResult.standardOutput.getUnownedSlice()))
        {
            String text = line.trim();
            Index tab = text.indexOf('\t');
            if (tab < 0)
                continue;
            String objectId = String(text.getUnownedSlice().head(tab));
            String ref = String(text.getUnownedSlice().tail(tab + 1));
            bool peeled = ref.endsWith("^{}");
            String name = peeled ? String(ref.getUnownedSlice().head(ref.getLength() - 3)) : ref;
            UnownedStringSlice tagPrefix = UnownedStringSlice::fromLiteral("refs/tags/");
            if (name.startsWith(tagPrefix))
                name = String(name.getUnownedSlice().tail(tagPrefix.getLength()));
            bool found = false;
            for (auto& remoteTag : remoteTags)
            {
                if (remoteTag.remote == remote && remoteTag.name == name)
                {
                    if (peeled)
                        remoteTag.peeled = objectId;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                RemoteTag remoteTag;
                remoteTag.remote = remote;
                remoteTag.name = name;
                remoteTag.peeled = objectId;
                remoteTags.add(remoteTag);
            }
        }
    }

    List<String> localTagArguments;
    localTagArguments.add("for-each-ref");
    localTagArguments.add("--format=%(refname:short)|%(objectname)|%(*objectname)");
    localTagArguments.add("refs/tags");
    List<String> localTagLines;
    SLANG_RETURN_ON_FAIL(
        _gitOutputLines(repositoryPath, localTagArguments, localTagLines, outError));
    for (const auto& line : localTagLines)
    {
        Index firstTab = line.indexOf('|');
        if (firstTab < 0)
            continue;
        String name = String(line.getUnownedSlice().head(firstTab));
        String rest = String(line.getUnownedSlice().tail(firstTab + 1));
        Index secondTab = rest.indexOf('|');
        String objectId = secondTab < 0 ? rest : String(rest.getUnownedSlice().head(secondTab));
        String peeledObject =
            secondTab < 0 ? String() : String(rest.getUnownedSlice().tail(secondTab + 1));
        String peeled = peeledObject.getLength() ? peeledObject : objectId;
        bool matched = false;
        bool namedOnRemote = false;
        for (const auto& remoteTag : remoteTags)
        {
            if (remoteTag.name != name)
                continue;
            namedOnRemote = true;
            if (remoteTag.peeled == peeled)
            {
                matched = true;
                break;
            }
        }
        if (matched)
            continue;
        if (!namedOnRemote)
        {
            report << "  tag " << name << " is not on any remote\n";
            continue;
        }
        for (const auto& remoteTag : remoteTags)
        {
            if (remoteTag.name != name || remoteTag.peeled == peeled)
                continue;
            report << "  tag " << name << " points at " << _shortObjectId(peeled) << ", but "
                   << remoteTag.remote << " points at " << _shortObjectId(remoteTag.peeled) << "\n";
        }
    }

    outReport = report.produceString();
    return SLANG_OK;
}

SlangResult deleteDisclosedGitRepository(
    const String& repositoryPath,
    const String& disclosedReport,
    bool unpushedDeletionApproved,
    String& outError)
{
    if (!_hasGitDir(repositoryPath))
    {
        outError = String("Cannot delete Git repository: ") + repositoryPath;
        return SLANG_FAIL;
    }
    String report;
    SLANG_RETURN_ON_FAIL(collectUnpushedRepositoryReport(repositoryPath, report, outError));
    if (report.getLength() && (!unpushedDeletionApproved || report != disclosedReport))
    {
        outError = String("Refusing to delete ") + repositoryPath +
                   " because it has commits or tags that are not on a remote:\n" + report +
                   "The repository was left in place.";
        return SLANG_FAIL;
    }
    if (SLANG_FAILED(Path::removeNonEmpty(repositoryPath)))
    {
        outError = String("Cannot delete Git repository: ") + repositoryPath;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult locatePreparedPackageCache(
    const String& canonicalPath,
    const String& gitURL,
    String& outPath,
    String& outError)
{
    auto originMatches = [&](const String& path, bool& outMatches) -> SlangResult
    {
        outMatches = false;
        if (!_hasGitDir(path))
            return SLANG_OK;
        String origin;
        SLANG_RETURN_ON_FAIL(getRepositoryOrigin(path, origin, outError));
        outMatches = origin == gitURL;
        return SLANG_OK;
    };
    bool canonicalMatches = false;
    SLANG_RETURN_ON_FAIL(originMatches(canonicalPath, canonicalMatches));
    if (canonicalMatches)
    {
        outPath = canonicalPath;
        return SLANG_OK;
    }
    String replacementPath = _replacementPackageCachePath(canonicalPath);
    bool replacementMatches = false;
    SLANG_RETURN_ON_FAIL(originMatches(replacementPath, replacementMatches));
    outPath = replacementMatches ? replacementPath : canonicalPath;
    return SLANG_OK;
}

SlangResult discardReplacementPackageCache(const String& replacementPath, String& outError)
{
    if (!_hasGitDir(replacementPath))
    {
        if (SLANG_FAILED(Path::removeNonEmpty(replacementPath)))
        {
            outError = String("Cannot remove package cache replacement: ") + replacementPath;
            return SLANG_FAIL;
        }
        return SLANG_OK;
    }
    String report;
    SLANG_RETURN_ON_FAIL(collectUnpushedRepositoryReport(replacementPath, report, outError));
    if (report.getLength())
    {
        outError = String("Refusing to remove package cache replacement ") + replacementPath +
                   " because it has commits or tags that are not on a remote:\n" + report;
        return SLANG_FAIL;
    }
    if (SLANG_FAILED(Path::removeNonEmpty(replacementPath)))
    {
        outError = String("Cannot remove package cache replacement: ") + replacementPath;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _ensureRepository(
    const String& workingDirectory,
    const String& gitURL,
    const String& repositoryPath,
    bool canReplace,
    bool allowRemote,
    String& outError,
    bool assumeYes,
    String* outActivePath,
    String* outUnpushedReport)
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
        if (!allowRemote)
        {
            outError = String("Package cache is missing; ") + _offlineUpdateAdvice() + ": " +
                       repositoryPath;
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
        if (!allowRemote || !canReplace)
        {
            outError =
                allowRemote
                    ? String("Git reports a different origin after replacing package cache: ") +
                          repositoryPath
                    : String("Cached package origin does not match; ") + _offlineUpdateAdvice() +
                          ": " + repositoryPath;
            return SLANG_FAIL;
        }
        String report;
        SLANG_RETURN_ON_FAIL(collectUnpushedRepositoryReport(repositoryPath, report, outError));
        if (report.getLength() && outUnpushedReport)
        {
            String replacementPath = _replacementPackageCachePath(repositoryPath);
            if (!Path::createDirectoryRecursive(Path::getParentDirectory(replacementPath)))
            {
                outError = String("Cannot create package cache directory: ") +
                           Path::getParentDirectory(replacementPath);
                return SLANG_FAIL;
            }
            SLANG_RETURN_ON_FAIL(_ensureRepository(
                workingDirectory,
                gitURL,
                replacementPath,
                true,
                allowRemote,
                outError,
                false,
                nullptr,
                nullptr));
            *outUnpushedReport = report;
            if (outActivePath)
                *outActivePath = replacementPath;
            return SLANG_OK;
        }
        if (report.getLength())
        {
            if (!assumeYes)
            {
                outError = String("Refusing to delete ") + repositoryPath +
                           " because it has commits or tags that are not on a remote:\n" + report +
                           "The repository was left in place. Re-run with --yes to delete it.";
                return SLANG_FAIL;
            }
            fprintf(
                stdout,
                "Deleting %s would drop commits or tags that are not on a remote:\n%s",
                repositoryPath.getBuffer(),
                report.getBuffer());
        }
        if (SLANG_FAILED(Path::removeNonEmpty(repositoryPath)))
        {
            outError = String("Cannot replace stale package cache: ") + repositoryPath;
            return SLANG_FAIL;
        }
        return _ensureRepository(
            workingDirectory,
            gitURL,
            repositoryPath,
            false,
            allowRemote,
            outError,
            assumeYes,
            outActivePath,
            outUnpushedReport);
    }

    if (outActivePath)
        *outActivePath = repositoryPath;
    if (outUnpushedReport)
        *outUnpushedReport = String();

    if (!allowRemote)
        return SLANG_OK;

    List<String> fetchArguments;
    fetchArguments.add("fetch");
    fetchArguments.add("--prune");
    fetchArguments.add("--prune-tags");
    fetchArguments.add("--force");
    fetchArguments.add("origin");
    fetchArguments.add("+refs/heads/*:refs/remotes/origin/*");
    fetchArguments.add("+refs/tags/*:refs/tags/*");
    fetchArguments.add("+HEAD:refs/slang-cache/origin/HEAD");
    return _runGit(repositoryPath, fetchArguments, result, outError);
}

SlangResult refreshPackageCache(
    const String& workingDirectory,
    const String& gitURL,
    const String& repositoryPath,
    String& outError,
    bool assumeYes,
    String* outActivePath,
    String* outUnpushedReport)
{
    if (outActivePath)
        *outActivePath = String();
    if (outUnpushedReport)
        *outUnpushedReport = String();
    return _ensureRepository(
        workingDirectory,
        gitURL,
        repositoryPath,
        true,
        true,
        outError,
        assumeYes,
        outActivePath,
        outUnpushedReport);
}

SlangResult commitDeferredCacheReplacement(
    const String& workingDirectory,
    const DeferredCacheReplacement& replacement,
    String& outError)
{
    if (_hasGitDir(replacement.canonicalPath))
    {
        SLANG_RETURN_ON_FAIL(deleteDisclosedGitRepository(
            replacement.canonicalPath,
            replacement.report,
            true,
            outError));
    }
    else
    {
        SlangPathType pathType;
        if (SLANG_SUCCEEDED(Path::getPathType(replacement.canonicalPath, &pathType)) &&
            SLANG_FAILED(Path::removeNonEmpty(replacement.canonicalPath)))
        {
            outError = String("Cannot replace stale package cache: ") + replacement.canonicalPath;
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(refreshPackageCache(
        workingDirectory,
        replacement.gitURL,
        replacement.canonicalPath,
        outError,
        true));
    return discardReplacementPackageCache(replacement.replacementPath, outError);
}

SlangResult requirePackageCache(
    const String& gitURL,
    const String& repositoryPath,
    String& outError)
{
    return _ensureRepository(
        ".",
        gitURL,
        repositoryPath,
        false,
        false,
        outError,
        false,
        nullptr,
        nullptr);
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
    PackageVersion& outVersion,
    bool& outFound,
    String& outError,
    List<String>* outWarnings)
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
        PackageVersion version;
        if (!tag.getLength() || !acceptCanonicalReleaseTag(tag, version, outWarnings))
            continue;
        if (outFound)
        {
            outError = "HEAD has multiple release-version tags; pass --as explicitly.";
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
    PackageVersion& outVersion,
    bool& outFound,
    String& outError,
    List<String>* outWarnings)
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
    PackageVersion nearestVersion;
    Int nearestDistance = 0;
    bool haveNearest = false;
    bool nearestTied = false;
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
    {
        String tag = line.trim();
        PackageVersion version;
        if (!tag.getLength() || !acceptCanonicalReleaseTag(tag, version, outWarnings))
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

static bool _hasGitDir(const String& path)
{
    return File::exists(Path::combine(path, ".git"));
}

static SlangResult _commitExists(
    const String& repositoryPath,
    const String& commit,
    bool& outExists,
    String& outError)
{
    List<String> arguments;
    arguments.add("cat-file");
    arguments.add("-e");
    arguments.add(commit + "^{commit}");
    ExecuteResult result;
    CommandLine commandLine;
    SLANG_RETURN_ON_FAIL(_executeGit(repositoryPath, arguments, commandLine, result, outError));
    outExists = result.resultCode == 0;
    return SLANG_OK;
}

static SlangResult _ensureCachedCommit(
    const String& repositoryPath,
    const String& commit,
    bool allowRemote,
    String& outError)
{
    bool exists = false;
    SLANG_RETURN_ON_FAIL(_commitExists(repositoryPath, commit, exists, outError));
    if (exists)
        return SLANG_OK;
    if (!allowRemote)
    {
        outError = String("Git commit is not in the local cache; ") + _offlineUpdateAdvice() +
                   ": " + commit;
        return SLANG_FAIL;
    }

    List<String> arguments;
    arguments.add("fetch");
    arguments.add("origin");
    arguments.add(commit);
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    SLANG_RETURN_ON_FAIL(_commitExists(repositoryPath, commit, exists, outError));
    if (!exists)
    {
        outError = String("Git origin did not provide commit: ") + commit;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult fetchCachedCommit(const String& repositoryPath, const String& commit, String& outError)
{
    return _ensureCachedCommit(repositoryPath, commit, true, outError);
}

SlangResult requireCachedCommit(
    const String& repositoryPath,
    const String& commit,
    String& outError)
{
    return _ensureCachedCommit(repositoryPath, commit, false, outError);
}

static SlangResult _listCachedRefNames(
    const String& cachePath,
    List<String>& outRefs,
    String& outError)
{
    outRefs.clear();
    List<String> arguments;
    arguments.add("for-each-ref");
    arguments.add("--format=%(refname)");
    arguments.add("refs/remotes/origin");
    arguments.add("refs/tags");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(cachePath, arguments, result, outError));
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
    {
        String ref = line.trim();
        if (ref.getLength() && ref != "refs/remotes/origin/HEAD")
            outRefs.add(ref);
    }
    return SLANG_OK;
}

SlangResult collectMovingCachedRefs(
    const String& cachePath,
    const String& destination,
    List<String>& outRefs,
    String& outError)
{
    outRefs.clear();
    if (!_hasGitDir(destination))
        return SLANG_OK;

    List<String> cachedRefs;
    SLANG_RETURN_ON_FAIL(_listCachedRefNames(cachePath, cachedRefs, outError));
    for (const auto& ref : cachedRefs)
    {
        String cachedCommit;
        SLANG_RETURN_ON_FAIL(resolveLocalRevision(cachePath, ref, cachedCommit, outError));
        String destinationCommit;
        String resolveError;
        if (SLANG_FAILED(resolveLocalRevision(destination, ref, destinationCommit, resolveError)))
            continue;
        if (destinationCommit != cachedCommit)
            outRefs.add(ref);
    }
    return SLANG_OK;
}

static SlangResult _stageCachedRepository(
    const String& cachePath,
    const String& destination,
    const String& targetCommit,
    bool allowMovingRefs,
    String& outError)
{
    bool cachedCommitExists = false;
    SLANG_RETURN_ON_FAIL(_commitExists(cachePath, targetCommit, cachedCommitExists, outError));
    if (!cachedCommitExists)
    {
        outError = String("Selected commit is not in the local cache: ") + targetCommit;
        return SLANG_FAIL;
    }

    if (!allowMovingRefs)
    {
        List<String> movingRefs;
        SLANG_RETURN_ON_FAIL(collectMovingCachedRefs(cachePath, destination, movingRefs, outError));
        if (movingRefs.getCount())
        {
            outError = String("Cache staging would move an existing dependency ref without "
                              "confirmation: ") +
                       movingRefs[0];
            return SLANG_FAIL;
        }
    }

    List<String> cachedRefs;
    SLANG_RETURN_ON_FAIL(_listCachedRefNames(cachePath, cachedRefs, outError));
    List<String> arguments;
    ExecuteResult result;
    if (cachedRefs.getCount())
    {
        arguments.add("fetch");
        arguments.add("--force");
        arguments.add("--no-tags");
        arguments.add("--");
        arguments.add(cachePath);
        for (const auto& ref : cachedRefs)
            arguments.add(String("+") + ref + ":" + ref);
        SLANG_RETURN_ON_FAIL(_runGit(destination, arguments, result, outError));
    }

    bool exists = false;
    SLANG_RETURN_ON_FAIL(_commitExists(destination, targetCommit, exists, outError));
    if (exists)
        return SLANG_OK;

    arguments.clear();
    arguments.add("fetch");
    arguments.add("--no-tags");
    arguments.add("--");
    arguments.add(cachePath);
    arguments.add(targetCommit);
    SLANG_RETURN_ON_FAIL(_runGit(destination, arguments, result, outError));
    SLANG_RETURN_ON_FAIL(_commitExists(destination, targetCommit, exists, outError));
    if (!exists)
    {
        outError = String("Selected commit is not in the local cache: ") + targetCommit;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _removeCheckoutForReplacement(
    const String& destination,
    const String& disclosedUnpushedReport,
    bool unpushedDeletionApproved,
    const char* failureText,
    String& outError)
{
    if (_hasGitDir(destination))
    {
        return deleteDisclosedGitRepository(
            destination,
            disclosedUnpushedReport,
            unpushedDeletionApproved,
            outError);
    }
    if (SLANG_FAILED(Path::removeNonEmpty(destination)))
    {
        outError = String(failureText) + destination;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _materializeRevision(
    const String& gitURL,
    const String& currentCommit,
    const String& targetCommit,
    const String& destination,
    bool allowClean,
    bool allowMovingRefs,
    const String& cachePath,
    bool& ioDidMaterialize,
    String& outError,
    const String& disclosedUnpushedReport,
    bool unpushedDeletionApproved)
{
    ExecuteResult result;
    SlangPathType pathType;
    bool destinationExisted = SLANG_SUCCEEDED(Path::getPathType(destination, &pathType));
    if (!destinationExisted)
    {
        if (!_hasGitDir(cachePath))
        {
            outError = String("Package cache is missing: ") + cachePath;
            return SLANG_FAIL;
        }
        if (!Path::createDirectoryRecursive(destination))
        {
            outError = String("Cannot create package destination: ") + destination;
            return SLANG_FAIL;
        }
        List<String> initArguments;
        initArguments.add("init");
        initArguments.add("-q");
        SLANG_RETURN_ON_FAIL(_runGit(destination, initArguments, result, outError));
        List<String> remoteAddArguments;
        remoteAddArguments.add("remote");
        remoteAddArguments.add("add");
        remoteAddArguments.add("origin");
        remoteAddArguments.add(gitURL);
        SLANG_RETURN_ON_FAIL(_runGit(destination, remoteAddArguments, result, outError));
        ioDidMaterialize = true;
    }
    else if (!_hasGitDir(destination))
    {
        if (!allowClean)
        {
            outError = String("Package destination is not a Git repository; refusing to replace "
                              "it without --clean: ") +
                       destination;
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(_removeCheckoutForReplacement(
            destination,
            disclosedUnpushedReport,
            unpushedDeletionApproved,
            "Cannot replace package destination: ",
            outError));
        return _materializeRevision(
            gitURL,
            String(),
            targetCommit,
            destination,
            false,
            allowMovingRefs,
            cachePath,
            ioDidMaterialize,
            outError,
            disclosedUnpushedReport,
            unpushedDeletionApproved);
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
        SLANG_RETURN_ON_FAIL(_removeCheckoutForReplacement(
            destination,
            disclosedUnpushedReport,
            unpushedDeletionApproved,
            "Cannot replace package checkout: ",
            outError));
        return _materializeRevision(
            gitURL,
            String(),
            targetCommit,
            destination,
            false,
            allowMovingRefs,
            cachePath,
            ioDidMaterialize,
            outError,
            disclosedUnpushedReport,
            unpushedDeletionApproved);
    }

    if (destinationExisted && currentCommit.getLength())
    {
        GitWorkingTreeStatus status;
        SLANG_RETURN_ON_FAIL(getWorkingTreeStatus(destination, currentCommit, status, outError));
        const bool hasUncommittedState = status.changedFileCount != 0 || status.stashCount != 0;
        if (!hasUncommittedState && status.headCommit == targetCommit)
        {
            // The work tree is current, but its tags and origin-tracking branches may still lag
            // behind the cache. Stage those refs without checking out the files again.
            return _stageCachedRepository(
                cachePath,
                destination,
                targetCommit,
                allowMovingRefs,
                outError);
        }

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
            SLANG_RETURN_ON_FAIL(_removeCheckoutForReplacement(
                destination,
                disclosedUnpushedReport,
                unpushedDeletionApproved,
                "Cannot replace package checkout: ",
                outError));
            return _materializeRevision(
                gitURL,
                String(),
                targetCommit,
                destination,
                false,
                allowMovingRefs,
                cachePath,
                ioDidMaterialize,
                outError,
                disclosedUnpushedReport,
                unpushedDeletionApproved);
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
        SLANG_RETURN_ON_FAIL(_removeCheckoutForReplacement(
            destination,
            disclosedUnpushedReport,
            unpushedDeletionApproved,
            "Cannot replace package checkout: ",
            outError));
        return _materializeRevision(
            gitURL,
            String(),
            targetCommit,
            destination,
            false,
            allowMovingRefs,
            cachePath,
            ioDidMaterialize,
            outError,
            disclosedUnpushedReport,
            unpushedDeletionApproved);
    }

    ioDidMaterialize = true;
    SLANG_RETURN_ON_FAIL(
        _stageCachedRepository(cachePath, destination, targetCommit, allowMovingRefs, outError));

    List<String> checkoutArguments;
    checkoutArguments.add("checkout");
    checkoutArguments.add("--detach");
    checkoutArguments.add(targetCommit);
    return _runGit(destination, checkoutArguments, result, outError);
}

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
    const String& disclosedUnpushedReport,
    bool unpushedDeletionApproved)
{
    outDidMaterialize = false;
    return _materializeRevision(
        gitURL,
        currentCommit,
        targetCommit,
        destination,
        allowClean,
        allowMovingRefs,
        cachePath,
        outDidMaterialize,
        outError,
        disclosedUnpushedReport,
        unpushedDeletionApproved);
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

SlangResult getCheckedOutBranch(
    const String& repositoryPath,
    String& outBranch,
    bool& outDetached,
    String& outError)
{
    List<String> arguments;
    arguments.add("rev-parse");
    arguments.add("--abbrev-ref");
    arguments.add("HEAD");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    outBranch = result.standardOutput.trim();
    outDetached = outBranch == "HEAD";
    return SLANG_OK;
}

SlangResult localBranchExists(
    const String& repositoryPath,
    const String& branch,
    bool& outExists,
    String& outError)
{
    List<String> arguments;
    arguments.add("show-ref");
    arguments.add("--verify");
    arguments.add("--quiet");
    arguments.add(String("refs/heads/") + branch);
    ExecuteResult result;
    int code = 0;
    SLANG_RETURN_ON_FAIL(_runGitCode(repositoryPath, arguments, code, result, outError));
    if (code != 0 && code != 1)
    {
        outError = result.standardError.trim();
        if (!outError.getLength())
            outError = String("Cannot tell whether branch exists: ") + branch;
        return SLANG_FAIL;
    }
    outExists = code == 0;
    return SLANG_OK;
}

SlangResult checkoutLocalBranch(
    const String& repositoryPath,
    const String& branch,
    String& outError)
{
    List<String> arguments;
    arguments.add("checkout");
    arguments.add(branch);
    ExecuteResult result;
    return _runGit(repositoryPath, arguments, result, outError);
}

SlangResult createLocalBranch(
    const String& repositoryPath,
    const String& branch,
    const String& commit,
    String& outError)
{
    List<String> arguments;
    arguments.add("branch");
    arguments.add(branch);
    arguments.add(commit);
    ExecuteResult result;
    return _runGit(repositoryPath, arguments, result, outError);
}

SlangResult checkoutDetachedCommit(
    const String& repositoryPath,
    const String& commit,
    String& outError)
{
    List<String> arguments;
    arguments.add("checkout");
    arguments.add("--detach");
    arguments.add(commit);
    ExecuteResult result;
    return _runGit(repositoryPath, arguments, result, outError);
}

SlangResult discardUncommittedState(const String& repositoryPath, String& outError)
{
    ExecuteResult result;
    List<String> arguments;
    arguments.add("reset");
    arguments.add("--hard");
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    arguments.clear();
    arguments.add("clean");
    arguments.add("-fd");
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    arguments.clear();
    arguments.add("stash");
    arguments.add("clear");
    return _runGit(repositoryPath, arguments, result, outError);
}

SlangResult createAnnotatedTag(const String& repositoryPath, const String& tag, String& outError)
{
    List<String> arguments;
    arguments.add("tag");
    arguments.add("-a");
    arguments.add(tag);
    arguments.add("-m");
    arguments.add(tag);
    ExecuteResult result;
    return _runGit(repositoryPath, arguments, result, outError);
}

SlangResult listTagNames(const String& repositoryPath, List<String>& outTags, String& outError)
{
    outTags.clear();
    List<String> arguments;
    arguments.add("tag");
    arguments.add("--list");
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
    {
        String tag = line.trim();
        if (tag.getLength())
            outTags.add(tag);
    }
    return SLANG_OK;
}

static SlangResult _commitParents(
    const String& repositoryPath,
    const String& commit,
    List<String>& outParents,
    String& outError)
{
    outParents.clear();
    List<String> arguments;
    arguments.add("rev-list");
    arguments.add("--parents");
    arguments.add("-n");
    arguments.add("1");
    arguments.add(commit);
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    List<UnownedStringSlice> fields;
    String line = String(result.standardOutput.trim());
    StringUtil::splitOnWhitespace(line.getUnownedSlice(), fields);
    for (Index i = 1; i < fields.getCount(); ++i)
        outParents.add(String(fields[i]));
    return SLANG_OK;
}

static SlangResult _commitContains(
    const String& repositoryPath,
    const String& ancestor,
    const String& commit,
    bool& outContains,
    String& outError)
{
    List<String> arguments;
    arguments.add("merge-base");
    arguments.add("--is-ancestor");
    arguments.add(ancestor);
    arguments.add(commit);
    ExecuteResult result;
    int code = 0;
    SLANG_RETURN_ON_FAIL(_runGitCode(repositoryPath, arguments, code, result, outError));
    if (code != 0 && code != 1)
    {
        outError = result.standardError.trim();
        if (!outError.getLength())
            outError = "git merge-base --is-ancestor failed.";
        return SLANG_FAIL;
    }
    outContains = code == 0;
    return SLANG_OK;
}

static SlangResult _canonicalTagsAtCommit(
    const String& repositoryPath,
    const String& commit,
    List<EditLineTag>& ioTags,
    String& outError)
{
    List<String> arguments;
    arguments.add("tag");
    arguments.add("--points-at");
    arguments.add(commit);
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    for (auto line : LineParser(result.standardOutput.getUnownedSlice()))
    {
        String tag = line.trim();
        PackageVersion version;
        if (!tag.getLength() || !acceptCanonicalReleaseTag(tag, version, nullptr))
            continue;
        EditLineTag found;
        found.tag = tag;
        found.commit = commit;
        found.version = version;
        ioTags.add(found);
    }
    return SLANG_OK;
}

SlangResult collectCanonicalTagsOnEditLine(
    const String& repositoryPath,
    const String& headCommit,
    const String& pinCommit,
    List<EditLineTag>& outTags,
    bool& outReachedPin,
    String& outError)
{
    outTags.clear();
    outReachedPin = false;
    String pin;
    SLANG_RETURN_ON_FAIL(resolveLocalRevision(repositoryPath, pinCommit, pin, outError));
    String current;
    SLANG_RETURN_ON_FAIL(resolveLocalRevision(repositoryPath, headCommit, current, outError));
    const Index kMaxSteps = 100000;
    for (Index step = 0; step < kMaxSteps; ++step)
    {
        SLANG_RETURN_ON_FAIL(_canonicalTagsAtCommit(repositoryPath, current, outTags, outError));
        if (current == pin)
        {
            outReachedPin = true;
            return SLANG_OK;
        }
        List<String> parents;
        SLANG_RETURN_ON_FAIL(_commitParents(repositoryPath, current, parents, outError));
        if (!parents.getCount())
            return SLANG_OK;
        List<String> containing;
        for (const auto& parent : parents)
        {
            bool contains = false;
            SLANG_RETURN_ON_FAIL(_commitContains(repositoryPath, pin, parent, contains, outError));
            if (contains)
                containing.add(parent);
        }
        if (!containing.getCount())
            return SLANG_OK;
        bool firstContains = false;
        for (const auto& parent : containing)
        {
            if (parent == parents[0])
                firstContains = true;
        }
        current = firstContains ? parents[0] : containing[0];
    }
    outError = "Edit-line walk exceeded the commit limit.";
    return SLANG_FAIL;
}

SlangResult countCommitsAfter(
    const String& repositoryPath,
    const String& ancestor,
    const String& descendant,
    Index& outCount,
    String& outError)
{
    outCount = 0;
    List<String> arguments;
    arguments.add("rev-list");
    arguments.add("--count");
    arguments.add(ancestor + ".." + descendant);
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(repositoryPath, arguments, result, outError));
    String text = result.standardOutput.trim();
    Index count = 0;
    if (!text.getLength())
    {
        outError = "git rev-list --count returned no count.";
        return SLANG_FAIL;
    }
    for (auto c : text.getUnownedSlice())
    {
        if (c < '0' || c > '9')
        {
            outError = String("git rev-list --count returned '") + text + "'.";
            return SLANG_FAIL;
        }
        count = count * 10 + Index(c - '0');
    }
    outCount = count;
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
