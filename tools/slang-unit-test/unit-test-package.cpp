// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "package-git.h"
#include "package-json.h"
#include "package-resolver.h"
#include "package-tool.h"
#include "package-types.h"
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
        File::generateTemporary(UnownedStringSlice("slang-package-test"), outDirectory.path));
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

static SlangResult _commitAndTag(
    const String& repository,
    const String& tag,
    bool annotated = false)
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
    arguments.add("-c");
    arguments.add("user.name=Slang Package Test");
    arguments.add("-c");
    arguments.add("user.email=slang-package-test@example.com");
    arguments.add("commit");
    arguments.add("-q");
    arguments.add("-m");
    arguments.add(tag);
    SLANG_RETURN_ON_FAIL(_runGitChecked(arguments));

    arguments.clear();
    arguments.add("-C");
    arguments.add(repository);
    arguments.add("tag");
    if (annotated)
    {
        arguments.add("-a");
        arguments.add("-m");
        arguments.add(tag);
    }
    arguments.add(tag);
    return _runGitChecked(arguments);
}

static SlangResult _initializeRepository(const String& repository)
{
    SLANG_RETURN_ON_FAIL(Path::createDirectoryRecursive(repository) ? SLANG_OK : SLANG_FAIL);
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

} // namespace

SLANG_UNIT_TEST(PackageVersionConstraint)
{
    VersionConstraint constraint;
    String error;
    SLANG_CHECK(SLANG_SUCCEEDED(
        parseVersionConstraint(UnownedStringSlice(">=v1.2.0 <v2.0.0"), constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 2, 0)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 9, 9)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(1, 1, 9)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(2, 0, 0)));

    SLANG_CHECK(
        SLANG_SUCCEEDED(parseVersionConstraint(UnownedStringSlice("v1.4.0"), constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 4, 0)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(1, 4, 1)));

    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice(">=v1.0"), constraint, error)));
    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice("^v1.2.0"), constraint, error)));
    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice(">=1.2.0"), constraint, error)));
    SLANG_CHECK(SLANG_FAILED(
        parseVersionConstraint(UnownedStringSlice("v1.2.0 v1.3.0"), constraint, error)));
    SLANG_CHECK(SLANG_SUCCEEDED(
        parseVersionConstraint(UnownedStringSlice("<=v2.0.0 >=v1.0.0"), constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 5, 0)));
    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice("> v1.2.0"), constraint, error)));
}

SLANG_UNIT_TEST(PackageManifestJSON)
{
    const String manifestText = "{\n"
                                "  // Package manifests allow comments.\n"
                                "  \"name\": \"root\",\n"
                                "  \"version\": \"0.1.0\",\n"
                                "  \"exports\": [\"src\"],\n"
                                "  \"dependencies\": {\n"
                                "    \"noise\": {\n"
                                "      \"git\": \"https://example.com/noise.git\",\n"
                                "      \"tag\": \">=v1.2.0 <v2.0.0\"\n"
                                "    }\n"
                                "  }\n"
                                "}\n";

    Manifest manifest;
    String error;
    SlangResult result = readManifestText("slang-package.json", manifestText, manifest, error);
    if (SLANG_FAILED(result))
        getTestReporter()->message(TestMessageType::Info, error.getBuffer());
    SLANG_CHECK_MSG(SLANG_SUCCEEDED(result), "manifest parsing failed");
    if (SLANG_FAILED(result))
        return;
    SLANG_CHECK(manifest.name == "root");
    SLANG_CHECK(manifest.version == "0.1.0");
    SLANG_CHECK(manifest.exports.getCount() == 1);
    SLANG_CHECK(manifest.exports[0] == "src");
    SLANG_CHECK(manifest.dependencies.getCount() == 1);
    SLANG_CHECK(manifest.dependencies[0].name == "noise");
    SLANG_CHECK(manifest.dependencies[0].tag == ">=v1.2.0 <v2.0.0");

    const String unsafeGitText =
        "{\"name\":\"root\",\"version\":\"0.1.0\",\"exports\":[\"src\"],"
        "\"dependencies\":{\"bad\":{\"git\":\"ext::sh -c bad\",\"tag\":\"v1.0.0\"}}}";
    SLANG_CHECK(SLANG_FAILED(readManifestText("unsafe-git.json", unsafeGitText, manifest, error)));

    const String unsafeExportText =
        "{\"name\":\"root\",\"version\":\"0.1.0\",\"exports\":[\"src\\n/etc\"],"
        "\"dependencies\":{}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("unsafe-export.json", unsafeExportText, manifest, error)));
}

SLANG_UNIT_TEST(PackageToolInit)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));

    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "src")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "tests")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "docs")));

    Manifest manifest;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readManifest(Path::combine(temp.path, "slang-package.json"), manifest, error)));
    SLANG_CHECK(manifest.name == Path::getFileName(temp.path));

    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));

    const String invalidRoot = Path::combine(temp.path, "invalid package");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(invalidRoot));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(invalidRoot, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK(!File::exists(Path::combine(invalidRoot, "slang-package.json")));
}

SLANG_UNIT_TEST(PackageResolverTransitiveRange)
{
    List<String> versionArguments;
    versionArguments.add("--version");
    ExecuteResult versionResult;
    if (SLANG_FAILED(_runGit(versionArguments, versionResult)))
    {
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(versionResult.resultCode == 0);

    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const String bRepository = Path::combine(temp.path, "b");
    const String aRepository = Path::combine(temp.path, "a");
    const String rootDirectory = Path::combine(temp.path, "root");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(bRepository)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_initializeRepository(aRepository)));
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(rootDirectory));

    String error;
    Manifest bManifest;
    bManifest.name = "b";
    bManifest.version = "1.2.0";
    bManifest.exports.add("src");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(bRepository, "slang-package.json"), bManifest, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(bRepository, "v1.2.0")));

    bManifest.version = "1.4.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(bRepository, "slang-package.json"), bManifest, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(bRepository, "v1.4.0", true)));

    List<TagCandidate> bCandidates;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(listReleaseTags(bRepository, bCandidates, error)));
    List<String> revisionArguments;
    revisionArguments.add("-C");
    revisionArguments.add(bRepository);
    revisionArguments.add("rev-parse");
    revisionArguments.add("v1.4.0^{commit}");
    ExecuteResult revisionResult;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runGit(revisionArguments, revisionResult)));
    SLANG_CHECK_ABORT(revisionResult.resultCode == 0);
    SLANG_CHECK_ABORT(bCandidates.getCount() == 2);
    SLANG_CHECK(bCandidates[0].commit == revisionResult.standardOutput.trim());

    Manifest aManifest;
    aManifest.name = "a";
    aManifest.version = "1.0.0";
    aManifest.exports.add("src");
    Dependency aToB;
    aToB.name = "b";
    aToB.git = bRepository;
    aToB.tag = ">=v1.2.0";
    aManifest.dependencies.add(aToB);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(aRepository, "slang-package.json"), aManifest, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(aRepository, "v1.0.0")));

    aManifest.version = "2.0.0";
    aManifest.dependencies[0].tag = ">=v9.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(aRepository, "slang-package.json"), aManifest, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(aRepository, "v2.0.0")));

    Manifest rootManifest;
    rootManifest.name = "root";
    rootManifest.version = "0.1.0";
    rootManifest.exports.add("src");
    Dependency rootToA;
    rootToA.name = "a";
    rootToA.git = aRepository;
    rootToA.tag = ">=v1.0.0";
    rootManifest.dependencies.add(rootToA);
    Dependency rootToB;
    rootToB.name = "b";
    rootToB.git = bRepository;
    rootToB.tag = "<v1.5.0";
    rootManifest.dependencies.add(rootToB);

    PackageTool::LockFile lock;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(resolveDependencies(rootDirectory, rootManifest, lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 2);
    for (const auto& package : lock.packages)
    {
        if (package.name == "b")
            SLANG_CHECK(package.tag == "v1.4.0");
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(rootDirectory, "slang-package.json"), rootManifest, error)));
    const char* updateArguments[] = {"slang-package", "update"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        rootDirectory,
        SLANG_COUNT_OF(updateArguments),
        updateArguments,
        error)));

    String searchPaths;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(rootDirectory, ".slang", "search-paths"), searchPaths)));
    const String aSearchPath = Path::combine(Path::combine(".slang", "packages", "a"), "src");
    const String bSearchPath = Path::combine(Path::combine(".slang", "packages", "b"), "src");
    SLANG_CHECK(searchPaths.getUnownedSlice().indexOf(aSearchPath.getUnownedSlice()) >= 0);
    SLANG_CHECK(searchPaths.getUnownedSlice().indexOf(bSearchPath.getUnownedSlice()) >= 0);

    const String lockPath = Path::combine(rootDirectory, "slang-package.lock");
    PackageTool::LockFile completeLock;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readLockFile(lockPath, completeLock, error)));
    PackageTool::LockFile incompleteLock = completeLock;
    for (Index i = 0; i < incompleteLock.packages.getCount(); ++i)
    {
        if (incompleteLock.packages[i].name == "b")
        {
            incompleteLock.packages.removeAt(i);
            break;
        }
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeLockFile(lockPath, incompleteLock, error)));
    const char* lockedFetchArguments[] = {"slang-package", "fetch", "--locked"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        rootDirectory,
        SLANG_COUNT_OF(lockedFetchArguments),
        lockedFetchArguments,
        error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeLockFile(lockPath, completeLock, error)));

    const char* editArguments[] = {"slang-package", "edit", "b"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(rootDirectory, SLANG_COUNT_OF(editArguments), editArguments, error)));
    const String editableRoot = Path::combine(rootDirectory, Path::combine(".slang", "edit", "b"));
    const String editableManifest = Path::combine(editableRoot, "slang-package.json");
    String editableManifestText;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(editableManifest, editableManifestText)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(editableManifest, editableManifestText + "\n")));

    const char* uneditArguments[] = {"slang-package", "unedit", "b"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        rootDirectory,
        SLANG_COUNT_OF(uneditArguments),
        uneditArguments,
        error)));
    SLANG_CHECK(File::exists(editableManifest));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(editableManifest, editableManifestText)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        rootDirectory,
        SLANG_COUNT_OF(uneditArguments),
        uneditArguments,
        error)));
    SLANG_CHECK(!File::exists(editableRoot));

    rootManifest.dependencies[1].tag = "<v1.4.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(rootDirectory, "slang-package.json"), rootManifest, error)));
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        rootDirectory,
        SLANG_COUNT_OF(lockedFetchArguments),
        lockedFetchArguments,
        error)));

    rootManifest.dependencies[1].tag = "<v1.2.0";
    error = String();
    SLANG_CHECK(SLANG_FAILED(resolveDependencies(rootDirectory, rootManifest, lock, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("No release tag satisfies")) >= 0);

    aManifest.version = "1.1.0";
    aManifest.dependencies[0].tag = "<v1.2.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(aRepository, "slang-package.json"), aManifest, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_commitAndTag(aRepository, "v1.1.0")));

    rootManifest.dependencies.clear();
    rootToB.tag = ">=v1.4.0";
    rootManifest.dependencies.add(rootToB);
    rootToA.tag = "v1.1.0";
    rootManifest.dependencies.add(rootToA);
    error = String();
    SLANG_CHECK(SLANG_FAILED(resolveDependencies(rootDirectory, rootManifest, lock, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("transitive constraint")) >= 0);
}
