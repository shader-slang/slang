// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "package-git.h"
#include "package-json.h"
#include "package-local.h"
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

SLANG_UNIT_TEST(PackageGitFreshCheckoutIgnoresRemoteHead)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const String repository = Path::combine(temp.path, "repository");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(repository));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(repository)));
    String sourcePath = Path::combine(repository, "content.txt");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(sourcePath, "version 1")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(repository, "v1.0.0")));

    String version1Commit;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(getRepositoryHeadCommit(repository, version1Commit, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(sourcePath, "version 2")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(repository, "v1.1.0")));

    String checkout = Path::combine(temp.path, "checkout");
    bool didMaterialize = false;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(materializeLockedRevision(
        temp.path,
        repository,
        version1Commit,
        version1Commit,
        checkout,
        false,
        didMaterialize,
        error)));
    SLANG_CHECK(didMaterialize);
    String checkoutCommit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(getRepositoryHeadCommit(checkout, checkoutCommit, error)));
    SLANG_CHECK(checkoutCommit == version1Commit);
}

SLANG_UNIT_TEST(PackageGitSkipsAlreadyMaterializedRevision)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const String repository = Path::combine(temp.path, "repository");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(repository));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(repository)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(repository, "content.txt"), "version 1")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(repository, "v1.0.0")));

    String commit;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(getRepositoryHeadCommit(repository, commit, error)));
    const String checkout = Path::combine(temp.path, "checkout");
    bool didMaterialize = false;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(materializeLockedRevision(
        temp.path,
        repository,
        commit,
        commit,
        checkout,
        false,
        didMaterialize,
        error)));
    SLANG_CHECK(didMaterialize);

    // Once the checkout is clean at the target commit, materialization needs only its local state.
    // Removing the remote makes this test fail if the implementation tries to fetch again.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(Path::removeNonEmpty(repository)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(materializeLockedRevision(
        temp.path,
        repository,
        commit,
        commit,
        checkout,
        false,
        didMaterialize,
        error)));
    SLANG_CHECK(!didMaterialize);
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
    GitWorkingTreeStatus committedStatus;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        getWorkingTreeStatus(committedRepository, expectedCommit, committedStatus, error)));
    SLANG_CHECK(committedStatus.commitsAhead == 1);
    SLANG_CHECK(committedStatus.commitsBehind == 0);
    String aheadCommit;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(getRepositoryHeadCommit(committedRepository, aheadCommit, error)));
    arguments.clear();
    arguments.add("-C");
    arguments.add(committedRepository);
    arguments.add("checkout");
    arguments.add("-q");
    arguments.add(expectedCommit);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runGitChecked(arguments)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        getWorkingTreeStatus(committedRepository, aheadCommit, committedStatus, error)));
    SLANG_CHECK(committedStatus.commitsAhead == 0);
    SLANG_CHECK(committedStatus.commitsBehind == 1);

    String stashedRepository = Path::combine(temp.path, "stashed");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(stashedRepository));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(stashedRepository)));
    String stashedContent = Path::combine(stashedRepository, "content.txt");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(stashedContent, "base")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(stashedRepository, "v1.0.0")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(getRepositoryHeadCommit(stashedRepository, expectedCommit, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(stashedContent, "changed")));
    GitWorkingTreeStatus changedStatus;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        getWorkingTreeStatus(stashedRepository, expectedCommit, changedStatus, error)));
    SLANG_CHECK(changedStatus.changedFileCount == 1);
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
    GitWorkingTreeStatus stashedStatus;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        getWorkingTreeStatus(stashedRepository, expectedCommit, stashedStatus, error)));
    SLANG_CHECK(stashedStatus.stashCount == 1);
}

// Status must name an absent checkout as unmaterialized rather than reporting it indirectly
// through a failed dependency-manifest read and a raw Git "cannot change to" complaint, and it
// must still inspect the checkouts that are present.
SLANG_UNIT_TEST(PackageToolStatusReportsUnmaterializedCheckouts)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    // Two independent dependencies, so one can be removed while the other stays present.
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    Manifest root;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    const char* packageNames[] = {"noise", "color"};
    for (const char* packageName : packageNames)
    {
        String repository = Path::combine(temp.path, String("upstream-") + packageName);
        SLANG_CHECK_ABORT(Path::createDirectoryRecursive(repository));
        Manifest dependencyManifest;
        dependencyManifest.name = packageName;
        dependencyManifest.exports.add("src");
        dependencyManifest.licenseFiles.add("LICENSE");
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(
            Path::combine(repository, "slang-package.json"),
            dependencyManifest,
            error)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            _writeFile(Path::combine(repository, "LICENSE"), String(packageName) + " license\n")));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
            Path::combine(repository, String("src/") + packageName + ".slang"),
            String("module ") + packageName + ";\n")));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(repository)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(repository, "v1.0.0")));

        Dependency dependency;
        dependency.name = packageName;
        dependency.git = repository;
        dependency.version = "1.0.0";
        root.dependencies.add(dependency);
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));

    const char* statusArguments[] = {"slang-package", "status"};
    SLANG_CHECK(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));

    Path::removeNonEmpty(Path::combine(temp.path, "deps/noise"));
    error = String();
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("are not materialized under 'deps/'")) >=
        0);
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("noise")) >= 0);
    // The absent checkout is reported once: neither the dependency-manifest read nor Git's own
    // missing-directory text should restate it, and the present sibling must not be implicated.
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("dependency manifest")) < 0);
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("cannot change to")) < 0);
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("color")) < 0);

    const char* fetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));

    // A checkout that is present but dirty is still inspected while a sibling is absent.
    Path::removeNonEmpty(Path::combine(temp.path, "deps/noise"));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::writeAllText(Path::combine(temp.path, "deps/color/stray.txt"), "stray\n")));
    error = String();
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("are not materialized under 'deps/'")) >=
        0);
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(
            UnownedStringSlice("Package checkout 'color' is not clean")) >= 0);
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

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
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
    const char* statusArguments[] = {"slang-package", "status"};
    root.dependencies.clear();
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("edit mode")) >= 0);
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("unreachable")) >= 0);
    root.dependencies.add(dependency);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* disableEditArguments[] = {
        "slang-package",
        "override",
        "disable",
        "noise",
    };
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(disableEditArguments),
        disableEditArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("no local override")) >= 0);

    const char* localUpdateArguments[] = {"slang-package", "update", "--yes"};
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
    SLANG_CHECK(editedLock.packages[0].ref == "v1.0.0");
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

    const char* unconfirmedCleanFetchArguments[] = {"slang-package", "fetch", "--clean"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(unconfirmedCleanFetchArguments),
        unconfirmedCleanFetchArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("--yes")) >= 0);
    const char* cleanFetchArguments[] = {"slang-package", "fetch", "--clean", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(cleanFetchArguments),
        cleanFetchArguments,
        error)));
    String restoredSource;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(checkoutSource, restoredSource)));
    SLANG_CHECK(restoredSource == "module noise;\n");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(unconfirmedCleanFetchArguments),
        unconfirmedCleanFetchArguments,
        error)));

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
    SLANG_CHECK(editedLock.packages[0].ref == "v1.1.0");
    SLANG_CHECK(File::exists(checkoutSource));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(checkoutSource, restoredSource)));
    SLANG_CHECK(restoredSource == "module noise;\n// v1.1");

    const char* addOverrideArguments[] = {
        "slang-package",
        "override",
        "add",
        "noise",
        repository.getBuffer(),
        "1.1.0",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(addOverrideArguments),
        addOverrideArguments,
        error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), editedLock, error)));
    SLANG_CHECK(editedLock.packages[0].path.getLength() != 0);

    const char* disableOverrideArguments[] = {
        "slang-package",
        "override",
        "disable",
        "noise",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(disableOverrideArguments),
        disableOverrideArguments,
        error)));
    String workspaceBeforeNoOp;
    String workspacePath = Path::combine(temp.path, "slang-workspace.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(workspacePath, workspaceBeforeNoOp)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(disableOverrideArguments),
        disableOverrideArguments,
        error)));
    String workspaceAfterNoOp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(workspacePath, workspaceAfterNoOp)));
    SLANG_CHECK(workspaceAfterNoOp == workspaceBeforeNoOp);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), editedLock, error)));
    SLANG_CHECK(editedLock.packages[0].path.getLength() == 0);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "out/search-paths"), searchPaths)));
    SLANG_CHECK(
        searchPaths.getUnownedSlice().indexOf(UnownedStringSlice("upstream-noise/src")) < 0);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));
    const char* removeOverrideArguments[] = {
        "slang-package",
        "override",
        "remove",
        "noise",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(removeOverrideArguments),
        removeOverrideArguments,
        error)));

    List<String> branchArguments;
    branchArguments.add("-C");
    branchArguments.add(repository);
    branchArguments.add("branch");
    branchArguments.add("feature-ref");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runGitChecked(branchArguments)));
    root.dependencies[0].version = ">=1.0.0 <2.0.0";
    root.dependencies[0].ref = "feature-ref";
    root.dependencies[0].as = "1.1.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), editedLock, error)));
    SLANG_CHECK(editedLock.packages[0].ref == "feature-ref");
    SLANG_CHECK(editedLock.packages[0].version == "1.1.0");
}

SLANG_UNIT_TEST(PackageToolUpdateIgnoresOverrides)
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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));

    String localRoot = Path::combine(temp.path, "local-noise");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(Path::combine(localRoot, "src")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(localRoot, "slang-package.json"), noise, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(localRoot, "LICENSE"), "Local noise license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(localRoot, "src/noise.slang"), "module noise;\n// local\n")));
    String relativeLocalRoot = Path::getRelativePath(temp.path, localRoot);
    const char* overrideArguments[] = {
        "slang-package",
        "override",
        "add",
        "noise",
        relativeLocalRoot.getBuffer(),
        "1.0.0",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(overrideArguments),
        overrideArguments,
        error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    PackageTool::LockFile lock;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 1);
    SLANG_CHECK(lock.packages[0].path.getLength() != 0);

    const char* unknownFromLocalArguments[] = {
        "slang-package",
        "update",
        "--from-local",
        "--yes",
    };
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(unknownFromLocalArguments),
        unknownFromLocalArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Unknown update option")) >= 0);

    const char* ignoreArguments[] = {
        "slang-package",
        "update",
        "--ignore-overrides",
        "--yes",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(ignoreArguments), ignoreArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));
    SLANG_CHECK(lock.packages[0].path.getLength() == 0);
    SLANG_CHECK(lock.packages[0].ref == "v1.0.0");
    SLANG_CHECK(File::exists(Path::combine(temp.path, "deps/noise/src/noise.slang")));

    List<LocalPackage> registered;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readProjectLocalPackages(temp.path, registered, error)));
    Index noiseIndex = findLocalPackageIndex(registered, "noise");
    SLANG_CHECK_ABORT(noiseIndex >= 0);
    SLANG_CHECK(!isEditedLocalPackage(registered[noiseIndex]));
    SLANG_CHECK(registered[noiseIndex].enabled);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));
    SLANG_CHECK(lock.packages[0].path.getLength() != 0);
}

SLANG_UNIT_TEST(PackageToolIgnoreOverridesParksEditedDependency)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    String helperRepo = Path::combine(temp.path, "upstream-helper");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(helperRepo));
    Manifest helper;
    helper.name = "helper";
    helper.exports.add("src");
    helper.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(helperRepo, "slang-package.json"), helper, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(helperRepo, "LICENSE"), "Helper license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(helperRepo, "src/helper.slang"), "module helper;\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(helperRepo)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(helperRepo, "v1.0.0")));

    String displayRepo = Path::combine(temp.path, "upstream-display");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(displayRepo));
    Manifest displayV1;
    displayV1.name = "display";
    displayV1.exports.add("src");
    displayV1.licenseFiles.add("LICENSE");
    Dependency helperDep;
    helperDep.name = "helper";
    helperDep.git = helperRepo;
    helperDep.version = ">=1.0.0";
    displayV1.dependencies.add(helperDep);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(displayRepo, "slang-package.json"), displayV1, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(displayRepo, "LICENSE"), "Display license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(displayRepo, "src/display.slang"), "module display;\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(displayRepo)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(displayRepo, "v1.0.0")));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency displayDep;
    displayDep.name = "display";
    displayDep.git = displayRepo;
    displayDep.version = ">=1.0.0";
    root.dependencies.add(displayDep);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));

    String helperCheckout = Path::combine(temp.path, "deps/helper/src/helper.slang");
    const char* editArguments[] = {"slang-package", "edit", "helper"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(editArguments), editArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(helperCheckout, "module helper;\n// edited\n")));

    String localDisplay = Path::combine(temp.path, "local-display");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(Path::combine(localDisplay, "src")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(localDisplay, "slang-package.json"), displayV1, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(localDisplay, "LICENSE"), "Local display license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(localDisplay, "src/display.slang"), "module display;\n")));
    String relativeLocalDisplay = Path::getRelativePath(temp.path, localDisplay);

    Manifest displayV2;
    displayV2.name = "display";
    displayV2.exports.add("src");
    displayV2.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(displayRepo, "slang-package.json"), displayV2, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(displayRepo, "v2.0.0")));

    const char* overrideArguments[] = {
        "slang-package",
        "override",
        "add",
        "display",
        relativeLocalDisplay.getBuffer(),
        "1.0.0",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(overrideArguments),
        overrideArguments,
        error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));

    PackageTool::LockFile lockAfterFirst;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lockAfterFirst, error)));
    SLANG_CHECK(lockAfterFirst.packages.getCount() == 2);
    String workspaceAfterFirst;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "slang-workspace.json"), workspaceAfterFirst)));
    String helperAfterFirst;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(helperCheckout, helperAfterFirst)));
    SLANG_CHECK(helperAfterFirst == "module helper;\n// edited\n");

    // --clean is only for leftover deps/display from the first Git checkout; materialize skips
    // registered edits, so helper's dirty checkout must survive.
    const char* ignoreArguments[] = {
        "slang-package",
        "update",
        "--ignore-overrides",
        "--clean",
        "--yes",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(ignoreArguments), ignoreArguments, error)));
    PackageTool::LockFile lockAfterIgnore;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lockAfterIgnore, error)));
    SLANG_CHECK(lockAfterIgnore.packages.getCount() == 1);
    SLANG_CHECK(lockAfterIgnore.packages[0].name == "display");
    SLANG_CHECK(lockAfterIgnore.packages[0].ref == "v2.0.0");
    SLANG_CHECK(File::exists(helperCheckout));
    String helperAfterIgnore;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(helperCheckout, helperAfterIgnore)));
    SLANG_CHECK(helperAfterIgnore == helperAfterFirst);
    String workspaceAfterIgnore;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "slang-workspace.json"), workspaceAfterIgnore)));
    SLANG_CHECK(workspaceAfterIgnore == workspaceAfterFirst);
    List<LocalPackage> registered;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readProjectLocalPackages(temp.path, registered, error)));
    Index helperIndex = findLocalPackageIndex(registered, "helper");
    SLANG_CHECK_ABORT(helperIndex >= 0);
    SLANG_CHECK(isEditedLocalPackage(registered[helperIndex]));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    PackageTool::LockFile lockAfterRestore;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readLockFile(
        Path::combine(temp.path, "slang-package-lock.json"),
        lockAfterRestore,
        error)));
    SLANG_CHECK(lockAfterRestore.packages.getCount() == 2);
    String helperAfterRestore;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(helperCheckout, helperAfterRestore)));
    SLANG_CHECK(helperAfterRestore == helperAfterFirst);
    String workspaceAfterRestore;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(
        Path::combine(temp.path, "slang-workspace.json"),
        workspaceAfterRestore)));
    SLANG_CHECK(workspaceAfterRestore == workspaceAfterFirst);
}
