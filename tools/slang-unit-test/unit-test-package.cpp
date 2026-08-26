// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "core/slang-io.h"
#include "core/slang-string-util.h"
#include "package-json.h"
#include "package-lock.h"
#include "package-resolver.h"
#include "package-tool.h"
#include "package-types.h"
#include "package-validate.h"
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

static SlangResult _writeFile(const String& path, const String& contents)
{
    String parent = Path::getParentDirectory(path);
    if (!Path::createDirectoryRecursive(parent))
        return SLANG_FAIL;
    return File::writeAllText(path, contents);
}

} // namespace

SLANG_UNIT_TEST(PackageVersionConstraint)
{
    VersionConstraint constraint;
    String error;
    SLANG_CHECK(SLANG_SUCCEEDED(
        parseVersionConstraint(UnownedStringSlice(">=1.2.0 <2.0.0"), constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 2, 0)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 9, 9)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(1, 1, 9)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(2, 0, 0)));

    SLANG_CHECK(
        SLANG_SUCCEEDED(parseVersionConstraint(UnownedStringSlice("1.4.0"), constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 4, 0)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(1, 4, 1)));

    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice(">=1.0"), constraint, error)));
    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice("^1.2.0"), constraint, error)));
    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice(">=v1.2.0"), constraint, error)));
    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice("1.2.0 1.3.0"), constraint, error)));
    SLANG_CHECK(SLANG_SUCCEEDED(
        parseVersionConstraint(UnownedStringSlice("<=2.0.0 >=1.0.0"), constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 5, 0)));
    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice("> 1.2.0"), constraint, error)));

    Dependency tagged;
    tagged.name = "noise";
    tagged.version = ">=9.0.0";
    tagged.tag = "v1.4.0";
    SLANG_CHECK(SLANG_SUCCEEDED(parseDependencyConstraint(tagged, constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 4, 0)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(9, 0, 0)));
}

SLANG_UNIT_TEST(PackageManifestJSON)
{
    const String manifestText = "{\n"
                                "  // Package manifests allow comments.\n"
                                "  \"name\": \"root\",\n"
                                "  \"exports\": [\"src\"],\n"
                                "  \"license_files\": [\"LICENSE\"],\n"
                                "  \"dependencies\": {\n"
                                "    \"noise\": {\n"
                                "      \"git\": \"https://example.com/noise.git\",\n"
                                "      \"version\": \">=1.2.0 <2.0.0\"\n"
                                "    }\n"
                                "  },\n"
                                "  \"workspace\": {\"deps\": \"third-party\", \"build\": \"out\"}\n"
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
    SLANG_CHECK(manifest.exports.getCount() == 1);
    SLANG_CHECK(manifest.exports[0] == "src");
    SLANG_CHECK(manifest.licenseFiles.getCount() == 1);
    SLANG_CHECK(manifest.licenseFiles[0] == "LICENSE");
    SLANG_CHECK(manifest.dependencies.getCount() == 1);
    SLANG_CHECK(manifest.dependencies[0].name == "noise");
    SLANG_CHECK(manifest.dependencies[0].version == ">=1.2.0 <2.0.0");
    SLANG_CHECK(manifest.dependencies[0].tag.getLength() == 0);
    SLANG_CHECK(manifest.workspace.depsDirectory == "third-party");
    SLANG_CHECK(manifest.workspace.buildDirectory == "out");

    const String taggedText = "{\n"
                              "  \"name\": \"root\",\n"
                              "  \"exports\": [\"src\"],\n"
                              "  \"license_files\": [\"LICENSE\"],\n"
                              "  \"dependencies\": {\n"
                              "    \"noise\": {\n"
                              "      \"git\": \"https://example.com/noise.git\",\n"
                              "      \"version\": \">=9.0.0\",\n"
                              "      \"tag\": \"v1.4.0\"\n"
                              "    }\n"
                              "  }\n"
                              "}\n";
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(readManifestText("tagged.json", taggedText, manifest, error)));
    SLANG_CHECK(manifest.dependencies[0].version == ">=9.0.0");
    SLANG_CHECK(manifest.dependencies[0].tag == "v1.4.0");

    const String unsafeGitText =
        "{\"name\":\"root\",\"exports\":[\"src\"],"
        "\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{\"bad\":{\"git\":\"ext::sh -c bad\",\"version\":\"1.0.0\"}}}";
    SLANG_CHECK(SLANG_FAILED(readManifestText("unsafe-git.json", unsafeGitText, manifest, error)));

    const String unsafeExportText = "{\"name\":\"root\",\"exports\":[\"src\\n/etc\"],"
                                    "\"license_files\":[\"LICENSE\"],"
                                    "\"dependencies\":{}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("unsafe-export.json", unsafeExportText, manifest, error)));

    const String missingLicenseFilesText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"dependencies\":{}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("missing-license-files.json", missingLicenseFilesText, manifest, error)));

    const String pathText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{\"noise\":{\"path\":\"../noise\"}}}";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifestText("path.json", pathText, manifest, error)));
    SLANG_CHECK(manifest.dependencies[0].path == "../noise");
    SLANG_CHECK(manifest.dependencies[0].git.getLength() == 0);

    const String mixedSourceText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{\"noise\":{\"git\":\"memory:noise\",\"path\":\"../noise\","
        "\"version\":\"1.0.0\"}}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("mixed-source.json", mixedSourceText, manifest, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("exactly one of 'git' or 'path'")) >= 0);

    const String versionedPathText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{\"noise\":{\"path\":\"../noise\",\"version\":\"1.0.0\"}}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("versioned-path.json", versionedPathText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Path dependency cannot")) >= 0);

    const String absolutePathText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{\"noise\":{\"path\":\"/tmp/noise\"}}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("absolute-path.json", absolutePathText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("must be relative")) >= 0);

    const String selfVersionText = "{\"name\":\"root\",\"version\":\"1.0.0\",\"exports\":[\"src\"],"
                                   "\"license_files\":[\"LICENSE\"],\"dependencies\":{}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("self-version.json", selfVersionText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("self version")) >= 0);

    const String unsafeWorkspaceText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"deps\":\"../deps\",\"build\":\"build\"}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("unsafe-workspace.json", unsafeWorkspaceText, manifest, error)));

    const String overlappingWorkspaceText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"deps\":\"out\",\"build\":\"out\"}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("overlapping-workspace.json", overlappingWorkspaceText, manifest, error)));

    const String implicitDepsOverlapText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"build\":\"deps\"}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("implicit-deps-overlap.json", implicitDepsOverlapText, manifest, error)));

    const String implicitBuildOverlapText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"deps\":\"build\"}}";
    SLANG_CHECK(SLANG_FAILED(readManifestText(
        "implicit-build-overlap.json",
        implicitBuildOverlapText,
        manifest,
        error)));

    const String nestedWorkspaceText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"deps\":\"state/deps\",\"build\":\"state\"}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("nested-workspace.json", nestedWorkspaceText, manifest, error)));
}

SLANG_UNIT_TEST(PackageLockRejectsOldVersion)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String path = Path::combine(temp.path, "slang-package-lock.json");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(path, "{\"lock_version\":1,\"packages\":{}}")));
    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK(SLANG_FAILED(readLockFile(path, lock, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("integer 3")) >= 0);
    SLANG_CHECK(SLANG_FAILED(readPreviousLockFile(path, lock, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(path, "{\"lock_version\":2,\"packages\":{}}")));
    SLANG_CHECK(SLANG_FAILED(readLockFile(path, lock, error)));
    SLANG_CHECK(SLANG_SUCCEEDED(readPreviousLockFile(path, lock, error)));
    SLANG_CHECK(lock.lockVersion == 2);
}

SLANG_UNIT_TEST(PackageLocalRegistryJSON)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String path = Path::combine(temp.path, "slang-workspace.json");
    List<LocalPackage> packages;
    LocalPackage package;
    package.name = "noise";
    package.path = "../noise";
    packages.add(package);
    package.name = "helper";
    package.path = "deps/helper";
    package.kind = LocalPackageKind::Edit;
    packages.add(package);

    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeLocalPackages(path, packages, error)));
    List<LocalPackage> roundTrip;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readLocalPackages(path, roundTrip, error)));
    SLANG_CHECK(roundTrip.getCount() == 2);
    SLANG_CHECK(roundTrip[0].name == "helper");
    SLANG_CHECK(roundTrip[0].path.getLength() == 0);
    SLANG_CHECK(isEditedLocalPackage(roundTrip[0]));
    SLANG_CHECK(roundTrip[1].name == "noise");
    SLANG_CHECK(roundTrip[1].path == "../noise");
    SLANG_CHECK(!isEditedLocalPackage(roundTrip[1]));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::writeAllText(path, "{\"overrides\":{\"noise\":{\"path\":\"/absolute/noise\"}}}")));
    SLANG_CHECK(SLANG_FAILED(readLocalPackages(path, roundTrip, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(path, "{\"edits\":{\"noise\":{\"unknown\":\"bad\"}}}")));
    SLANG_CHECK(SLANG_FAILED(readLocalPackages(path, roundTrip, error)));
}

SLANG_UNIT_TEST(PackageToolInit)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::writeAllText(Path::combine(temp.path, ".gitignore"), "node_modules\nbuild/\n")));

    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "src")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "tests")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "docs")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "deps")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "build")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "LICENSE")));
    String gitIgnore;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::readAllText(Path::combine(temp.path, ".gitignore"), gitIgnore)));
    SLANG_CHECK(gitIgnore.getUnownedSlice().indexOf(UnownedStringSlice(".slang/")) >= 0);
    SLANG_CHECK(gitIgnore.getUnownedSlice().indexOf(UnownedStringSlice("deps/")) >= 0);
    SLANG_CHECK(gitIgnore.getUnownedSlice().indexOf(UnownedStringSlice("build/")) >= 0);
    SLANG_CHECK(
        gitIgnore.getUnownedSlice().indexOf(UnownedStringSlice("slang-workspace.json")) >= 0);
    Index buildIgnoreCount = 0;
    for (auto line : LineParser(gitIgnore.getUnownedSlice()))
        buildIgnoreCount += line.trim() == "build/";
    SLANG_CHECK(buildIgnoreCount == 1);

    Manifest manifest;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readManifest(Path::combine(temp.path, "slang-package.json"), manifest, error)));
    SLANG_CHECK(manifest.name == Path::getFileName(temp.path));
    SLANG_CHECK(manifest.licenseFiles.getCount() == 1);
    SLANG_CHECK(manifest.licenseFiles[0] == "LICENSE");
    SLANG_CHECK(manifest.workspace.depsDirectory == "deps");
    SLANG_CHECK(manifest.workspace.buildDirectory == "build");

    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));

    TemporaryDirectory noNewlineTemp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(noNewlineTemp)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::writeAllText(Path::combine(noNewlineTemp.path, ".gitignore"), "node_modules")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        noNewlineTemp.path,
        SLANG_COUNT_OF(initArguments),
        initArguments,
        error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(noNewlineTemp.path, ".gitignore"), gitIgnore)));
    SLANG_CHECK(
        gitIgnore.getUnownedSlice().indexOf(UnownedStringSlice("node_modules\n.slang/")) >= 0);

    const String invalidRoot = Path::combine(temp.path, "invalid package");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(invalidRoot));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(invalidRoot, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK(!File::exists(Path::combine(invalidRoot, "slang-package.json")));
}

SLANG_UNIT_TEST(PackageToolFetchRequiresLock)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));

    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));

    const char* fetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("slang-package-lock.json")) >= 0);
}

SLANG_UNIT_TEST(PackageToolUpdateUpgradesPreviousLock)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    String lockPath = Path::combine(temp.path, "slang-package-lock.json");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(lockPath, "{\"lock_version\":2,\"packages\":{}}")));

    const char* updateArguments[] = {"slang-package", "update"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    PackageTool::LockFile lock;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readLockFile(lockPath, lock, error)));
    SLANG_CHECK(lock.lockVersion == 3);
}

SLANG_UNIT_TEST(PackageToolFetchRejectsPathLockForGitDependency)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency dependency;
    dependency.name = "noise";
    dependency.git = "memory:noise";
    dependency.version = ">=1.0.0";
    root.dependencies.add(dependency);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    LockedPackage locked;
    locked.name = "noise";
    locked.path = "../untrusted-noise";
    locked.exports.add("src");
    PackageTool::LockFile lock;
    lock.packages.add(locked);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));

    const char* fetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("trusted path dependency")) >= 0);
    const char* validateArguments[] = {"slang-package", "validate"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("trusted path dependency")) >= 0);
}

SLANG_UNIT_TEST(PackageToolRejectsPathIntoSlangState)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    String evilRoot = Path::combine(temp.path, ".slang/evil");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(evilRoot));
    Manifest evil;
    evil.name = "evil";
    evil.exports.add("src");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(evilRoot, "slang-package.json"), evil, error)));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency pathDep;
    pathDep.name = "evil";
    pathDep.path = ".slang/evil";
    root.dependencies.add(pathDep);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("package-tool state under .slang")) >=
        0);

    LockedPackage locked;
    locked.name = "evil";
    locked.path = ".slang/evil";
    locked.exports.add("src");
    PackageTool::LockFile lock;
    lock.packages.add(locked);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));

    const char* fetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("package-tool state under .slang")) >=
        0);
    const char* validateArguments[] = {"slang-package", "validate"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("package-tool state under .slang")) >=
        0);
}

SLANG_UNIT_TEST(PackageToolPathDependencies)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    String aRoot = Path::combine(temp.path, "vendor/a");
    String bRoot = Path::combine(aRoot, "vendor/b");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(bRoot));
    Manifest b;
    b.name = "b";
    b.exports.add("src");
    b.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(bRoot, "slang-package.json"), b, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(Path::combine(bRoot, "LICENSE"), "B license\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(bRoot, "src/b.slang"), "module b;\n")));

    String cRoot = Path::combine(temp.path, "vendor/c");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(cRoot));
    Manifest c;
    c.name = "c";
    c.exports.add("src");
    c.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(cRoot, "slang-package.json"), c, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(Path::combine(cRoot, "LICENSE"), "C license\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(cRoot, "src/c.slang"), "module c;\n")));

    Manifest a;
    a.name = "a";
    a.exports.add("src");
    a.licenseFiles.add("LICENSE");
    Dependency bPath;
    bPath.name = "b";
    bPath.path = "vendor/b";
    a.dependencies.add(bPath);
    Dependency cPath;
    cPath.name = "c";
    cPath.path = "../c";
    a.dependencies.add(cPath);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(aRoot, "slang-package.json"), a, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(Path::combine(aRoot, "LICENSE"), "A license\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(aRoot, "src/a.slang"), "module a;\n")));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency bGit;
    bGit.name = "b";
    bGit.git = "memory:b";
    bGit.version = ">=9.0.0";
    root.dependencies.add(bGit);
    Dependency aPath;
    aPath.name = "a";
    aPath.path = "vendor/a";
    root.dependencies.add(aPath);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    PackageTool::LockFile previewLock;
    List<String> resolveWarnings;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        resolveDependencies(temp.path, root, previewLock, error, &resolveWarnings)));
    SLANG_CHECK(resolveWarnings.getCount() == 2);
    bool foundShadowWarning = false;
    for (const auto& warning : resolveWarnings)
    {
        foundShadowWarning =
            foundShadowWarning ||
            warning.getUnownedSlice().indexOf(UnownedStringSlice("shadows a Git dependency")) >= 0;
    }
    SLANG_CHECK(foundShadowWarning);

    root.dependencies.clear();
    root.dependencies.add(aPath);
    root.dependencies.add(bGit);
    resolveWarnings.clear();
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        resolveDependencies(temp.path, root, previewLock, error, &resolveWarnings)));
    SLANG_CHECK(resolveWarnings.getCount() == 2);
    root.dependencies.clear();
    root.dependencies.add(bGit);
    root.dependencies.add(aPath);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    PackageTool::LockFile lock;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));
    SLANG_CHECK(lock.lockVersion == 3);
    SLANG_CHECK(lock.packages.getCount() == 3);
    Index lockedAIndex = findLockedPackageIndex(lock, "a");
    Index lockedBIndex = findLockedPackageIndex(lock, "b");
    SLANG_CHECK_ABORT(lockedAIndex >= 0 && lockedBIndex >= 0);
    SLANG_CHECK(lock.packages[lockedAIndex].path == "vendor/a");
    SLANG_CHECK(lock.packages[lockedBIndex].path == "vendor/a/vendor/b");
    SLANG_CHECK(lock.packages[lockedBIndex].git.getLength() == 0);
    Index lockedCIndex = findLockedPackageIndex(lock, "c");
    SLANG_CHECK_ABORT(lockedCIndex >= 0);
    SLANG_CHECK(lock.packages[lockedCIndex].path == "vendor/c");

    const char* overridePathArguments[] = {"slang-package", "override", "a", "vendor/a"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(overridePathArguments),
        overridePathArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("cannot be overridden")) >= 0);

    const char* fetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    String searchPaths;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "build/search-paths"), searchPaths)));
    SLANG_CHECK(searchPaths.getUnownedSlice().indexOf(UnownedStringSlice("vendor/a/src")) >= 0);
    SLANG_CHECK(
        searchPaths.getUnownedSlice().indexOf(UnownedStringSlice("vendor/a/vendor/b/src")) >= 0);
    SLANG_CHECK(searchPaths.getUnownedSlice().indexOf(UnownedStringSlice("vendor/c/src")) >= 0);

    List<String> warnings;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(validateProject(temp.path, error, &warnings)));
    SLANG_CHECK(warnings.getCount() == 1);
    SLANG_CHECK(
        warnings[0].getUnownedSlice().indexOf(UnownedStringSlice("escapes package 'a'")) >= 0);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::remove(Path::combine(cRoot, "slang-package.json"))));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("manifest")) >= 0);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(cRoot, "slang-package.json"), c, error)));

    String otherBRoot = Path::combine(temp.path, "vendor/other-b");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(otherBRoot));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(otherBRoot, "slang-package.json"), b, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(otherBRoot, "LICENSE"), "Other B license\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(otherBRoot, "src/b.slang"), "module b;\n")));
    Dependency conflictingB;
    conflictingB.name = "b";
    conflictingB.path = "vendor/other-b";
    root.dependencies.clear();
    root.dependencies.add(conflictingB);
    root.dependencies.add(aPath);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("more than one path")) >= 0);
}

SLANG_UNIT_TEST(PackageToolLocalOverrideUpdatesDefinitiveLock)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency dependency;
    dependency.name = "noise";
    dependency.git = "memory:noise";
    dependency.version = ">=5.0.0";
    root.dependencies.add(dependency);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    LockedPackage locked;
    locked.name = "noise";
    locked.git = dependency.git;
    locked.tag = "v1.0.0";
    locked.commit = "0000000000000000000000000000000000000000";
    locked.exports.add("src");
    PackageTool::LockFile lock;
    lock.packages.add(locked);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));

    TemporaryDirectory localTemp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(localTemp)));
    const String& localRoot = localTemp.path;
    Manifest local;
    local.name = "noise";
    local.exports.add("src");
    local.licenseFiles.add("LICENSE");
    Dependency localDependency;
    localDependency.name = "helper";
    localDependency.git = "memory:helper";
    localDependency.version = ">=2.0.0";
    local.dependencies.add(localDependency);
    localDependency.name = "noise";
    localDependency.git = "memory:noise";
    localDependency.version = ">=1.0.0";
    local.dependencies.add(localDependency);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(localRoot, "slang-package.json"), local, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(localRoot, "LICENSE"), "Noise license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(localRoot, "src", "noise.slang"), "module noise;\n")));

    String relativeLocalRoot = Path::getRelativePath(temp.path, localRoot);
    const char* overrideArguments[] = {
        "slang-package",
        "override",
        "noise",
        relativeLocalRoot.getBuffer(),
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(overrideArguments),
        overrideArguments,
        error)));

    TemporaryDirectory helperTemp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(helperTemp)));
    Manifest helper;
    helper.name = "helper";
    helper.exports.add("src");
    helper.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(helperTemp.path, "slang-package.json"), helper, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(helperTemp.path, "LICENSE"), "Helper license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(helperTemp.path, "src", "helper.slang"), "module helper;\n")));
    String relativeHelperRoot = Path::getRelativePath(temp.path, helperTemp.path);
    const char* helperOverrideArguments[] = {
        "slang-package",
        "override",
        "helper",
        relativeHelperRoot.getBuffer(),
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(helperOverrideArguments),
        helperOverrideArguments,
        error)));

    const char* validateArguments[] = {"slang-package", "validate"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));

    const char* updateArguments[] = {"slang-package", "update", "--from-local"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 2);
    for (const auto& package : lock.packages)
    {
        SLANG_CHECK(package.path.getLength() != 0);
        SLANG_CHECK(package.tag.getLength() == 0);
        SLANG_CHECK(package.commit.getLength() == 0);
    }
    SLANG_CHECK(lock.packages[0].name == "helper");
    SLANG_CHECK(lock.packages[1].name == "noise");
    SLANG_CHECK(lock.packages[1].dependencies.getCount() == 2);
    SLANG_CHECK(lock.packages[1].dependencies[0].name == "helper");
    SLANG_CHECK(lock.packages[1].dependencies[1].name == "noise");

    const char* unoverrideNoiseArguments[] = {"slang-package", "unoverride", "noise"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(unoverrideNoiseArguments),
        unoverrideNoiseArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("lock still points")) >= 0);
    const char* uneditNoiseArguments[] = {"slang-package", "unedit", "noise"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(uneditNoiseArguments),
        uneditNoiseArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("not editable")) >= 0);

    String lockBeforeFailedUpdate;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(
        Path::combine(temp.path, "slang-package-lock.json"),
        lockBeforeFailedUpdate)));
    TemporaryDirectory unusedTemp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(unusedTemp)));
    Manifest unused;
    unused.name = "unused";
    unused.exports.add("src");
    unused.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(unusedTemp.path, "slang-package.json"), unused, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(unusedTemp.path, "LICENSE"), "Unused license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(unusedTemp.path, "src", "unused.slang"), "module unused;\n")));
    String relativeUnusedRoot = Path::getRelativePath(temp.path, unusedTemp.path);
    const char* unusedOverrideArguments[] = {
        "slang-package",
        "override",
        "unused",
        relativeUnusedRoot.getBuffer(),
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(unusedOverrideArguments),
        unusedOverrideArguments,
        error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    String lockAfterFailedUpdate;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(
        Path::combine(temp.path, "slang-package-lock.json"),
        lockAfterFailedUpdate)));
    SLANG_CHECK(lockAfterFailedUpdate == lockBeforeFailedUpdate);
    const char* unoverrideUnusedArguments[] = {"slang-package", "unoverride", "unused"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(unoverrideUnusedArguments),
        unoverrideUnusedArguments,
        error)));

    const char* fetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    String searchPaths;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "build", "search-paths"), searchPaths)));
    String expectedSearchPath = Path::combine(relativeLocalRoot, "src");
    SLANG_CHECK(searchPaths.getUnownedSlice().indexOf(expectedSearchPath.getUnownedSlice()) >= 0);
    expectedSearchPath = Path::combine(relativeHelperRoot, "src");
    SLANG_CHECK(searchPaths.getUnownedSlice().indexOf(expectedSearchPath.getUnownedSlice()) >= 0);

    String registryPath = Path::combine(temp.path, "slang-workspace.json");
    List<LocalPackage> registeredPackages;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readLocalPackages(registryPath, registeredPackages, error)));
    Index noiseRegistration = -1;
    for (Index i = 0; i < registeredPackages.getCount(); ++i)
    {
        if (registeredPackages[i].name == "noise")
            noiseRegistration = i;
    }
    SLANG_CHECK_ABORT(noiseRegistration >= 0);
    registeredPackages[noiseRegistration].path = relativeHelperRoot;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeLocalPackages(registryPath, registeredPackages, error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("does not match")) >= 0);
    registeredPackages[noiseRegistration].path = relativeLocalRoot;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeLocalPackages(registryPath, registeredPackages, error)));

    local.dependencies[0].version = ">=2.0.0 <3.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(localRoot, "slang-package.json"), local, error)));
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("dependencies do not match")) >= 0);
    local.dependencies[0].version = ">=2.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(localRoot, "slang-package.json"), local, error)));

    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::remove(Path::combine(temp.path, "slang-workspace.json"))));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("is not registered")) >= 0);
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("is not registered")) >= 0);
}

SLANG_UNIT_TEST(PackageValidateStructureAndLicense)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));

    const char* validateArguments[] = {"slang-package", "validate"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("license placeholder")) >= 0);

    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Test license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, Path::combine("src", "acme", "noise.slang")),
        "// Primary module.\nmodule noise;\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(
            temp.path,
            Path::combine("src", "acme", Path::combine("noise", "helper.slang"))),
        "implementing noise;\n")));
    error = String();
    SLANG_CHECK(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, Path::combine("src", "acme", Path::combine("noise", "bad.slang"))),
        "module bad;\n")));
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Companion")) >= 0);
}

SLANG_UNIT_TEST(PackageValidateRejectsFlattenedModuleAlias)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(temp.path, "src", "noise.slang"), "module noise;\n")));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency dependency;
    dependency.name = "b";
    dependency.git = "memory:b";
    dependency.version = "1.0.0";
    root.dependencies.add(dependency);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    String packageRoot = Path::combine(Path::combine(temp.path, "deps"), "b");
    Manifest package;
    package.name = "b";
    package.exports.add("src");
    package.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(packageRoot));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(packageRoot, "slang-package.json"), package, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(packageRoot, "LICENSE"), "B license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(packageRoot, "src", "noise.slang"), "module noise;\n")));

    LockedPackage locked;
    locked.name = "b";
    locked.git = "memory:b";
    locked.tag = "v1.0.0";
    locked.commit = "0000000000000000000000000000000000000000";
    locked.exports.add("src");
    PackageTool::LockFile lock;
    lock.packages.add(locked);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));

    const char* validateArguments[] = {"slang-package", "validate"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("exported by both")) >= 0);
}

namespace
{

struct InMemoryRelease
{
    String git;
    String sourceRoot;
    TagCandidate candidate;
    Manifest manifest;
};

class InMemoryPackageSource : public IPackageResolverSource
{
public:
    List<InMemoryRelease> releases;

    void addRelease(
        const String& git,
        const String& version,
        const Manifest& manifest,
        const String& sourceRoot = String())
    {
        InMemoryRelease release;
        release.git = git;
        release.sourceRoot = sourceRoot;
        release.manifest = manifest;
        release.candidate.tag = String("v") + version;
        release.candidate.commit = release.candidate.tag;
        SLANG_RELEASE_ASSERT(SLANG_SUCCEEDED(
            SemanticVersion::parse(version.getUnownedSlice(), release.candidate.version)));
        releases.add(release);
    }

    virtual SlangResult listReleaseTags(
        const String&,
        const String& git,
        List<TagCandidate>& outCandidates,
        String& outError) override
    {
        outCandidates.clear();
        for (const auto& release : releases)
        {
            if (release.git == git)
                outCandidates.add(release.candidate);
        }
        if (outCandidates.getCount() == 0)
        {
            outError = String("No in-memory releases for ") + git;
            return SLANG_FAIL;
        }
        outCandidates.sort([](const TagCandidate& left, const TagCandidate& right)
                           { return left.version > right.version; });
        return SLANG_OK;
    }

    virtual SlangResult loadManifest(
        const String&,
        const String& git,
        const TagCandidate& candidate,
        ResolvedManifest& outManifest,
        String& outError) override
    {
        for (const auto& release : releases)
        {
            if (release.git == git && release.candidate.tag == candidate.tag)
            {
                outManifest.manifest = release.manifest;
                outManifest.sourceRoot = release.sourceRoot;
                outManifest.lockRoot = Path::combine("deps", outManifest.manifest.name);
                return SLANG_OK;
            }
        }
        outError = String("Missing in-memory manifest for ") + git + "@" + candidate.tag;
        return SLANG_FAIL;
    }
};

static Manifest _makeManifest(const char* name)
{
    Manifest manifest;
    manifest.name = name;
    manifest.exports.add("src");
    manifest.licenseFiles.add("LICENSE");
    return manifest;
}

static void _addDependency(
    Manifest& manifest,
    const char* name,
    const char* git,
    const char* version)
{
    Dependency dependency;
    dependency.name = name;
    dependency.git = git;
    dependency.version = version;
    manifest.dependencies.add(dependency);
}

static const LockedPackage* _findLockedPackage(const PackageTool::LockFile& lock, const char* name)
{
    for (const auto& package : lock.packages)
    {
        if (package.name == name)
            return &package;
    }
    return nullptr;
}

} // namespace

SLANG_UNIT_TEST(PackageResolverTransitiveRange)
{
    InMemoryPackageSource source;
    source.addRelease("memory:b", "1.2.0", _makeManifest("b"));
    source.addRelease("memory:b", "1.4.0", _makeManifest("b"));

    Manifest a1 = _makeManifest("a");
    _addDependency(a1, "b", "memory:b", ">=1.2.0");
    source.addRelease("memory:a", "1.0.0", a1);
    Manifest a2 = _makeManifest("a");
    _addDependency(a2, "b", "memory:b", ">=9.0.0");
    source.addRelease("memory:a", "2.0.0", a2);

    Manifest root = _makeManifest("root");
    _addDependency(root, "a", "memory:a", ">=1.0.0");
    _addDependency(root, "b", "memory:b", "<1.5.0");

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 2);
    const LockedPackage* a = _findLockedPackage(lock, "a");
    const LockedPackage* b = _findLockedPackage(lock, "b");
    SLANG_CHECK(a && a->tag == "v1.0.0");
    SLANG_CHECK(b && b->tag == "v1.4.0");
}

SLANG_UNIT_TEST(PackageResolverPathPackageGitTransitive)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String pRoot = Path::combine(temp.path, "vendor/p");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(pRoot));

    Manifest p = _makeManifest("p");
    _addDependency(p, "b", "memory:b", ">=1.2.0");
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(pRoot, "slang-package.json"), p, error)));

    InMemoryPackageSource source;
    source.addRelease("memory:b", "1.2.0", _makeManifest("b"));
    source.addRelease("memory:b", "1.4.0", _makeManifest("b"));
    Manifest root = _makeManifest("root");
    Dependency pPath;
    pPath.name = "p";
    pPath.path = "vendor/p";
    root.dependencies.add(pPath);
    _addDependency(root, "b", "memory:b", "<1.5.0");

    PackageTool::LockFile lock;
    List<String> warnings;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        resolveDependenciesWithSource(temp.path, root, source, lock, error, &warnings)));
    const LockedPackage* lockedP = _findLockedPackage(lock, "p");
    const LockedPackage* lockedB = _findLockedPackage(lock, "b");
    SLANG_CHECK(lockedP && lockedP->path == "vendor/p");
    SLANG_CHECK(lockedB && lockedB->tag == "v1.4.0");
}

SLANG_UNIT_TEST(PackageResolverPathShadowsSelectedGit)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String mRoot = Path::combine(temp.path, "m");
    String pathQRoot = Path::combine(mRoot, "vendor/q");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(pathQRoot));

    Manifest pathQ = _makeManifest("q");
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(pathQRoot, "slang-package.json"), pathQ, error)));

    InMemoryPackageSource source;
    Manifest gitQ = _makeManifest("q");
    _addDependency(gitQ, "w", "memory:w", ">=1.0.0");
    source.addRelease("memory:q", "1.0.0", gitQ);
    Manifest w = _makeManifest("w");
    _addDependency(w, "stale", "memory:stale", ">=9.0.0");
    source.addRelease("memory:w", "1.0.0", w);
    Manifest m = _makeManifest("m");
    Dependency qPath;
    qPath.name = "q";
    qPath.path = "vendor/q";
    m.dependencies.add(qPath);
    source.addRelease("memory:m", "1.0.0", m, mRoot);
    Manifest z = _makeManifest("z");
    _addDependency(z, "m", "memory:m", ">=1.0.0");
    source.addRelease("memory:z", "1.0.0", z);

    Manifest root = _makeManifest("root");
    _addDependency(root, "q", "memory:q", ">=1.0.0");
    _addDependency(root, "z", "memory:z", ">=1.0.0");

    PackageTool::LockFile lock;
    List<String> warnings;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        resolveDependenciesWithSource(temp.path, root, source, lock, error, &warnings)));
    const LockedPackage* lockedQ = _findLockedPackage(lock, "q");
    SLANG_CHECK_ABORT(lockedQ);
    SLANG_CHECK(lockedQ->path == "deps/m/vendor/q");
    SLANG_CHECK(lockedQ->git.getLength() == 0);
    SLANG_CHECK(_findLockedPackage(lock, "w") == nullptr);
    SLANG_CHECK(_findLockedPackage(lock, "stale") == nullptr);
    SLANG_CHECK(warnings.getCount() == 1);
}

SLANG_UNIT_TEST(PackageResolverCompatibleSelfCycle)
{
    InMemoryPackageSource source;
    Manifest a = _makeManifest("a");
    _addDependency(a, "a", "memory:a", ">=1.0.0");
    source.addRelease("memory:a", "1.0.0", a);
    Manifest root = _makeManifest("root");
    _addDependency(root, "a", "memory:a", ">=1.0.0");

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 1);
    const LockedPackage* lockedA = _findLockedPackage(lock, "a");
    SLANG_CHECK_ABORT(lockedA);
    SLANG_CHECK(lockedA->tag == "v1.0.0");
}

SLANG_UNIT_TEST(PackageResolverCompatibleCycle)
{
    InMemoryPackageSource source;
    Manifest a = _makeManifest("a");
    _addDependency(a, "b", "memory:b", ">=1.0.0");
    source.addRelease("memory:a", "1.0.0", a);
    Manifest b = _makeManifest("b");
    _addDependency(b, "a", "memory:a", ">=1.0.0");
    source.addRelease("memory:b", "1.0.0", b);
    Manifest root = _makeManifest("root");
    _addDependency(root, "a", "memory:a", ">=1.0.0");

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 2);
}

SLANG_UNIT_TEST(PackageResolverCycleBacktracksEarlierSelection)
{
    InMemoryPackageSource source;
    source.addRelease("memory:a", "1.0.0", _makeManifest("a"));
    source.addRelease("memory:a", "2.0.0", _makeManifest("a"));
    Manifest b = _makeManifest("b");
    _addDependency(b, "a", "memory:a", "<2.0.0");
    source.addRelease("memory:b", "1.0.0", b);
    Manifest root = _makeManifest("root");
    _addDependency(root, "a", "memory:a", ">=1.0.0");
    _addDependency(root, "b", "memory:b", ">=1.0.0");

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    const LockedPackage* lockedA = _findLockedPackage(lock, "a");
    SLANG_CHECK_ABORT(lockedA);
    SLANG_CHECK(lockedA->tag == "v1.0.0");
}

SLANG_UNIT_TEST(PackageResolverRejectsUnsatisfiableCycle)
{
    InMemoryPackageSource source;
    Manifest a = _makeManifest("a");
    _addDependency(a, "b", "memory:b", ">=1.0.0");
    source.addRelease("memory:a", "1.0.0", a);
    Manifest b = _makeManifest("b");
    _addDependency(b, "a", "memory:a", ">=2.0.0");
    source.addRelease("memory:b", "1.0.0", b);
    Manifest root = _makeManifest("root");
    _addDependency(root, "a", "memory:a", ">=1.0.0");

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK(SLANG_FAILED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("No release tag satisfies")) >= 0);
}
