// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "package-git.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using namespace Slang::PackageTool;

namespace
{

struct TemporaryDirectory
{
    String path;

    ~TemporaryDirectory()
    {
        if (path.getLength())
            Path::removeNonEmpty(path);
    }
};

static SlangResult _makeTemporaryDirectory(TemporaryDirectory& outDirectory)
{
    SLANG_RETURN_ON_FAIL(
        File::generateTemporary(UnownedStringSlice("slang-package-git-test"), outDirectory.path));
    SLANG_RETURN_ON_FAIL(File::remove(outDirectory.path));
    return Path::createDirectoryRecursive(outDirectory.path) ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _runGit(const List<String>& arguments, ExecuteResult& outResult)
{
    CommandLine commandLine;
    commandLine.setExecutableLocation(ExecutableLocation(ExecutableLocation::Type::Name, "git"));
    for (const auto& argument : arguments)
        commandLine.addArg(argument);
    return ProcessUtil::execute(commandLine, outResult);
}

static SlangResult _runGitChecked(const List<String>& arguments)
{
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runGit(arguments, result));
    return result.resultCode == 0 ? SLANG_OK : SLANG_FAIL;
}

static void _addTestIdentity(List<String>& arguments)
{
    arguments.add("-c");
    arguments.add("user.name=Slang Package Test");
    arguments.add("-c");
    arguments.add("user.email=slang-package-test@example.com");
}

static SlangResult _initializeRepository(const String& repository)
{
    List<String> arguments;
    arguments.add("-c");
    arguments.add("init.defaultBranch=main");
    arguments.add("-c");
    arguments.add("init.templateDir=");
    arguments.add("init");
    arguments.add("-q");
    arguments.add(repository);
    return _runGitChecked(arguments);
}

static SlangResult _commitAndTag(const String& repository, const String& tag)
{
    List<String> arguments;
    arguments.add("-C");
    arguments.add(repository);
    arguments.add("add");
    arguments.add(".");
    SLANG_RETURN_ON_FAIL(_runGitChecked(arguments));

    arguments.clear();
    arguments.add("-C");
    arguments.add(repository);
    _addTestIdentity(arguments);
    arguments.add("commit");
    arguments.add("-q");
    arguments.add("-m");
    arguments.add(tag);
    SLANG_RETURN_ON_FAIL(_runGitChecked(arguments));

    arguments.clear();
    arguments.add("-C");
    arguments.add(repository);
    _addTestIdentity(arguments);
    arguments.add("tag");
    arguments.add("-a");
    arguments.add("-m");
    arguments.add(tag);
    arguments.add(tag);
    return _runGitChecked(arguments);
}

} // namespace

SLANG_UNIT_TEST(PackageGitResolvesAnnotatedTagToCommit)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const String repository = Path::combine(temp.path, "repository");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(repository));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(repository)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(repository, "content.txt"), "content")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(repository, "v1.0.0")));

    List<TagCandidate> candidates;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(listReleaseTags(repository, candidates, error)));
    SLANG_CHECK_ABORT(candidates.getCount() == 1);

    List<String> arguments;
    arguments.add("-C");
    arguments.add(repository);
    arguments.add("rev-parse");
    arguments.add("v1.0.0^{commit}");
    ExecuteResult result;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runGit(arguments, result)));
    SLANG_CHECK_ABORT(result.resultCode == 0);
    SLANG_CHECK(candidates[0].commit == result.standardOutput.trim());
}
