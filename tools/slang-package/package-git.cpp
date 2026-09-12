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
    String& outError)
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

SlangResult listReleaseTagsFromRepository(
    const String& repositoryPath,
    List<TagCandidate>& outCandidates,
    String& outError)
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
    return _parseReleaseTagLines(result.standardOutput, outCandidates, outError);
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

static SlangResult _ensureRepository(
    const String& workingDirectory,
    const String& gitURL,
    const String& repositoryPath,
    bool canReplace,
    bool allowRemote,
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
            outError);
    }

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
    String& outError)
{
    return _ensureRepository(workingDirectory, gitURL, repositoryPath, true, true, outError);
}

SlangResult requirePackageCache(
    const String& gitURL,
    const String& repositoryPath,
    String& outError)
{
    return _ensureRepository(".", gitURL, repositoryPath, false, false, outError);
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

static SlangResult _materializeRevision(
    const String& gitURL,
    const String& currentCommit,
    const String& targetCommit,
    const String& destination,
    bool allowClean,
    bool allowMovingRefs,
    const String& cachePath,
    bool& ioDidMaterialize,
    String& outError)
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
        if (SLANG_FAILED(Path::removeNonEmpty(destination)))
        {
            outError = String("Cannot replace package destination: ") + destination;
            return SLANG_FAIL;
        }
        return _materializeRevision(
            gitURL,
            String(),
            targetCommit,
            destination,
            false,
            allowMovingRefs,
            cachePath,
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
            gitURL,
            String(),
            targetCommit,
            destination,
            false,
            allowMovingRefs,
            cachePath,
            ioDidMaterialize,
            outError);
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
            if (SLANG_FAILED(Path::removeNonEmpty(destination)))
            {
                outError = String("Cannot replace package checkout: ") + destination;
                return SLANG_FAIL;
            }
            return _materializeRevision(
                gitURL,
                String(),
                targetCommit,
                destination,
                false,
                allowMovingRefs,
                cachePath,
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
            gitURL,
            String(),
            targetCommit,
            destination,
            false,
            allowMovingRefs,
            cachePath,
            ioDidMaterialize,
            outError);
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
    const String& cachePath)
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
