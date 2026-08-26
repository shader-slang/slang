// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "package-git.h"
#include "package-json.h"
#include "package-tool.h"
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

static SlangResult _writeFile(const String& path, const String& contents)
{
    if (!Path::createDirectoryRecursive(Path::getParentDirectory(path)))
        return SLANG_FAIL;
    return File::writeAllText(path, contents);
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

SLANG_UNIT_TEST(PackageGitDirtyPredicateIncludesCommitsAndStashes)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;

    String committedRepository = Path::combine(temp.path, "committed");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(committedRepository));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(committedRepository)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(committedRepository, "content.txt"), "base")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(committedRepository, "v1.0.0")));
    String expectedCommit;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(getRepositoryHeadCommit(committedRepository, expectedCommit, error)));

    List<String> arguments;
    arguments.add("-C");
    arguments.add(committedRepository);
    _addTestIdentity(arguments);
    arguments.add("commit");
    arguments.add("-q");
    arguments.add("--allow-empty");
    arguments.add("-m");
    arguments.add("local commit");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runGitChecked(arguments)));
    bool isSafe = true;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        isWorkingTreeSafeToRemove(committedRepository, expectedCommit, isSafe, error)));
    SLANG_CHECK(!isSafe);

    String stashedRepository = Path::combine(temp.path, "stashed");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(stashedRepository));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(stashedRepository)));
    String stashedContent = Path::combine(stashedRepository, "content.txt");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(stashedContent, "base")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(stashedRepository, "v1.0.0")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(getRepositoryHeadCommit(stashedRepository, expectedCommit, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(stashedContent, "changed")));
    arguments.clear();
    arguments.add("-C");
    arguments.add(stashedRepository);
    arguments.add("stash");
    arguments.add("push");
    arguments.add("-q");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runGitChecked(arguments)));
    isSafe = true;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        isWorkingTreeSafeToRemove(stashedRepository, expectedCommit, isSafe, error)));
    SLANG_CHECK(!isSafe);
}

SLANG_UNIT_TEST(PackageToolEditKeepsStableDependencyPath)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    String repository = Path::combine(temp.path, "upstream-noise");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(repository));
    Manifest noise;
    noise.name = "noise";
    noise.exports.add("src");
    noise.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(repository, "slang-package.json"), noise, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(repository, "LICENSE"), "Noise license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(repository, "src/noise.slang"), "module noise;\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(repository)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(repository, "v1.0.0")));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency dependency;
    dependency.name = "noise";
    dependency.git = repository;
    dependency.version = "1.0.0";
    root.dependencies.add(dependency);
    root.workspace.depsDirectory = "third-party";
    root.workspace.buildDirectory = "out";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    String checkout = Path::combine(temp.path, "third-party/noise");
    String checkoutSource = Path::combine(checkout, "src/noise.slang");
    SLANG_CHECK(File::exists(checkoutSource));
    SLANG_CHECK(!File::exists(Path::combine(temp.path, "deps/noise")));
    String searchPaths;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "out/search-paths"), searchPaths)));
    SLANG_CHECK(
        searchPaths.getUnownedSlice().indexOf(UnownedStringSlice("third-party/noise/src")) >= 0);

    const char* editArguments[] = {"slang-package", "edit", "noise"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(editArguments), editArguments, error)));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "slang-workspace.json")));
    SLANG_CHECK(File::exists(checkoutSource));

    const char* localUpdateArguments[] = {"slang-package", "update", "--from-local"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(localUpdateArguments),
        localUpdateArguments,
        error)));
    PackageTool::LockFile editedLock;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), editedLock, error)));
    SLANG_CHECK_ABORT(editedLock.packages.getCount() == 1);
    SLANG_CHECK(editedLock.packages[0].git == repository);
    SLANG_CHECK(editedLock.packages[0].tag == "v1.0.0");
    SLANG_CHECK(editedLock.packages[0].path.getLength() == 0);

    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(checkoutSource, "module noise;\n// edited\n")));
    const char* fetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    String editedSource;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(checkoutSource, editedSource)));
    SLANG_CHECK(editedSource == "module noise;\n// edited\n");

    const char* uneditArguments[] = {"slang-package", "unedit", "noise"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(uneditArguments), uneditArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("local changes")) >= 0);

    List<String> restoreArguments;
    restoreArguments.add("-C");
    restoreArguments.add(checkout);
    restoreArguments.add("restore");
    restoreArguments.add(".");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runGitChecked(restoreArguments)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(uneditArguments), uneditArguments, error)));
    SLANG_CHECK(File::exists(checkoutSource));

    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(checkoutSource, "module noise;\n// unregistered\n")));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("without --clean")) >= 0);

    const char* cleanFetchArguments[] = {"slang-package", "fetch", "--clean"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(cleanFetchArguments),
        cleanFetchArguments,
        error)));
    String restoredSource;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(checkoutSource, restoredSource)));
    SLANG_CHECK(restoredSource == "module noise;\n");

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(
        Path::combine(repository, "src/noise.slang"),
        "module noise;\n// v1.1")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(repository, "v1.1.0")));
    root.dependencies[0].version = ">=1.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), editedLock, error)));
    SLANG_CHECK(editedLock.packages[0].tag == "v1.1.0");
    SLANG_CHECK(File::exists(checkoutSource));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(checkoutSource, restoredSource)));
    SLANG_CHECK(restoredSource == "module noise;\n// v1.1");
}
