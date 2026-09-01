// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "core/slang-string-util.h"
#include "package-json.h"
#include "package-local.h"
#include "package-lock.h"
#include "package-path.h"
#include "package-report.h"
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

static SlangResult _runSlangc(const List<String>& arguments, ExecuteResult& outResult)
{
    String executablePath = Path::combine(
        Path::getParentDirectory(Path::getExecutablePath()),
        String("slangc") + Process::getExecutableSuffix());
    CommandLine commandLine;
    commandLine.setExecutableLocation(
        ExecutableLocation(ExecutableLocation::Type::Path, executablePath));
    for (const auto& argument : arguments)
        commandLine.addArg(argument);
    return ProcessUtil::execute(commandLine, outResult);
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

    SLANG_CHECK(SLANG_SUCCEEDED(
        parseVersionConstraint(UnownedStringSlice(">=1.2.0 !=1.3.0"), constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 2, 0)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 4, 0)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(1, 3, 0)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(1, 1, 9)));

    SLANG_CHECK(
        SLANG_SUCCEEDED(parseVersionConstraint(UnownedStringSlice("!=1.3.0"), constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 2, 0)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(1, 3, 0)));

    SLANG_CHECK(SLANG_SUCCEEDED(parseVersionConstraint(
        UnownedStringSlice(">=1.0.0 <1.3.0 || >=1.3.1 <2.0.0"),
        constraint,
        error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 2, 0)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 3, 1)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(1, 3, 0)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(2, 0, 0)));

    SLANG_CHECK(SLANG_SUCCEEDED(
        parseVersionConstraint(UnownedStringSlice("1.4.0 || 1.5.0"), constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 4, 0)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 5, 0)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(1, 6, 0)));

    SLANG_CHECK(SLANG_FAILED(parseVersionConstraint(UnownedStringSlice("||"), constraint, error)));
    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice(">=1.0.0 ||"), constraint, error)));
    SLANG_CHECK(
        SLANG_FAILED(parseVersionConstraint(UnownedStringSlice(">=1.0.0||"), constraint, error)));

    Dependency pinned;
    pinned.name = "noise";
    pinned.version = ">=1.0.0 <2.0.0";
    pinned.ref = "release";
    pinned.as = "1.4.0";
    SLANG_CHECK(SLANG_SUCCEEDED(parseDependencyConstraint(pinned, constraint, error)));
    SLANG_CHECK(constraint.matches(SemanticVersion(1, 4, 0)));
    SLANG_CHECK(!constraint.matches(SemanticVersion(9, 0, 0)));
}

SLANG_UNIT_TEST(PackageManifestJSON)
{
    const String manifestText =
        "{\n"
        "  // Package manifests allow comments.\n"
        "  \"schema_version\": 1,\n"
        "  \"name\": \"root\",\n"
        "  \"exports\": [\"src\"],\n"
        "  \"license_files\": [\"LICENSE\"],\n"
        "  \"dependencies\": {\n"
        "    \"noise\": {\n"
        "      \"git\": \"https://example.com/noise.git\",\n"
        "      \"version\": \">=1.2.0 <2.0.0\"\n"
        "    }\n"
        "  },\n"
        "  \"retractions\": [\n"
        "    {\"version\": \"1.1.0\", \"reason\": \"Broken release\"}\n"
        "  ],\n"
        "  \"workspace\": {\n"
        "    \"deps\": \"third-party\",\n"
        "    \"build\": \"out\",\n"
        "    \"excludes\": [\n"
        "      {\"package\": \"noise\", \"version\": \"1.3.0\", \"reason\": \"Workspace "
        "regression\"}\n"
        "    ]\n"
        "  },\n"
        "  \"host\": {\"executables\": [\"root-tool\"], \"default\": \"root-tool\"}\n"
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
    SLANG_CHECK(manifest.dependencies[0].ref.getLength() == 0);
    SLANG_CHECK(manifest.workspace.depsDirectory == "third-party");
    SLANG_CHECK(manifest.workspace.buildDirectory == "out");
    SLANG_CHECK(manifest.retractions.getCount() == 1);
    SLANG_CHECK(manifest.retractions[0].version == "1.1.0");
    SLANG_CHECK(manifest.retractions[0].reason == "Broken release");
    SLANG_CHECK(manifest.workspace.exclusions.getCount() == 1);
    SLANG_CHECK(manifest.workspace.exclusions[0].packageName == "noise");
    SLANG_CHECK(manifest.workspace.exclusions[0].version == "1.3.0");
    SLANG_CHECK(manifest.host.executables.getCount() == 1);
    SLANG_CHECK(manifest.host.executables[0] == "root-tool");
    SLANG_CHECK(manifest.host.defaultExecutable == "root-tool");
    SLANG_CHECK(manifest.workspace.bundle.modules);
    SLANG_CHECK(manifest.workspace.bundle.source);

    const String pinnedText = "{\n"
                              "  \"schema_version\": 1,\n"
                              "  \"name\": \"root\",\n"
                              "  \"exports\": [\"src\"],\n"
                              "  \"license_files\": [\"LICENSE\"],\n"
                              "  \"dependencies\": {\n"
                              "    \"noise\": {\n"
                              "      \"git\": \"https://example.com/noise.git\",\n"
                              "      \"version\": \">=1.0.0 <2.0.0\",\n"
                              "      \"ref\": \"release-1.4\",\n"
                              "      \"as\": \"1.4.0\"\n"
                              "    }\n"
                              "  }\n"
                              "}\n";
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(readManifestText("pinned.json", pinnedText, manifest, error)));
    SLANG_CHECK(manifest.dependencies[0].version == ">=1.0.0 <2.0.0");
    SLANG_CHECK(manifest.dependencies[0].ref == "release-1.4");
    SLANG_CHECK(manifest.dependencies[0].as == "1.4.0");

    const String refOnlyText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],"
        "\"license_files\":[\"LICENSE\"],\"dependencies\":{\"noise\":{"
        "\"git\":\"https://example.com/noise.git\",\"ref\":\"main\",\"as\":\"2.1.0\"}}}";
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(readManifestText("ref-only.json", refOnlyText, manifest, error)));
    SLANG_CHECK(manifest.dependencies[0].version.getLength() == 0);
    SLANG_CHECK(manifest.dependencies[0].ref == "main");
    SLANG_CHECK(manifest.dependencies[0].as == "2.1.0");

    const String contradictoryPinText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],"
        "\"license_files\":[\"LICENSE\"],\"dependencies\":{\"noise\":{"
        "\"git\":\"https://example.com/noise.git\",\"version\":\"<2.0.0\","
        "\"ref\":\"main\",\"as\":\"2.1.0\"}}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("contradictory-pin.json", contradictoryPinText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("does not satisfy")) >= 0);

    const String missingAsText = "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],"
                                 "\"license_files\":[\"LICENSE\"],\"dependencies\":{\"noise\":{"
                                 "\"git\":\"https://example.com/noise.git\",\"ref\":\"main\"}}}";
    SLANG_CHECK(SLANG_FAILED(readManifestText("missing-as.json", missingAsText, manifest, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("'ref' and 'as' together")) >= 0);

    const String asWithoutRefText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],"
        "\"license_files\":[\"LICENSE\"],\"dependencies\":{\"noise\":{"
        "\"git\":\"https://example.com/noise.git\",\"version\":\"2.1.0\","
        "\"as\":\"2.1.0\"}}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("as-without-ref.json", asWithoutRefText, manifest, error)));

    const String unsafeGitText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],"
        "\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{\"bad\":{\"git\":\"ext::sh -c bad\",\"version\":\"1.0.0\"}}}";
    SLANG_CHECK(SLANG_FAILED(readManifestText("unsafe-git.json", unsafeGitText, manifest, error)));

    const String unsafeExportText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\\n/etc\"],"
        "\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("unsafe-export.json", unsafeExportText, manifest, error)));

    const String missingLicenseFilesText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"dependencies\":{}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("missing-license-files.json", missingLicenseFilesText, manifest, error)));

    const String pathText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],"
        "\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{\"noise\":{\"path\":\"../noise\",\"as\":\"1.0.0\"}}}";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifestText("path.json", pathText, manifest, error)));
    SLANG_CHECK(manifest.dependencies[0].path == "../noise");
    SLANG_CHECK(manifest.dependencies[0].git.getLength() == 0);

    const String mixedSourceText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],"
        "\"dependencies\":{\"noise\":{\"git\":\"memory:noise\",\"path\":\"../noise\","
        "\"version\":\"1.0.0\"}}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("mixed-source.json", mixedSourceText, manifest, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("exactly one of 'git' or 'path'")) >= 0);

    const String versionedPathText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],"
        "\"dependencies\":{\"noise\":{\"path\":\"../noise\",\"version\":\"1.0.0\"}}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("versioned-path.json", versionedPathText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Path dependency must")) >= 0);

    const String absolutePathText = "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],"
                                    "\"license_files\":[\"LICENSE\"],"
                                    "\"dependencies\":{\"noise\":{\"path\":\"/tmp/noise\","
                                    "\"as\":\"1.0.0\"}}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("absolute-path.json", absolutePathText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("must be relative")) >= 0);

    const String selfVersionText = "{\"name\":\"root\",\"version\":\"1.0.0\",\"exports\":[\"src\"],"
                                   "\"license_files\":[\"LICENSE\"],\"dependencies\":{}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("self-version.json", selfVersionText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Unknown field")) >= 0);

    const String missingFormatVersionText =
        "{\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{}}";
    SLANG_CHECK(SLANG_FAILED(readManifestText(
        "missing-format-version.json",
        missingFormatVersionText,
        manifest,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("required")) >= 0);

    const String wrongFormatVersionText =
        "{\"schema_version\":2,\"name\":\"root\",\"exports\":[\"src\"],"
        "\"license_files\":[\"LICENSE\"],\"dependencies\":{}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("wrong-format-version.json", wrongFormatVersionText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("integer 1")) >= 0);

    const String unsafeWorkspaceText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"deps\":\"../deps\",\"build\":\"build\"}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("unsafe-workspace.json", unsafeWorkspaceText, manifest, error)));

    const String overlappingWorkspaceText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"deps\":\"out\",\"build\":\"out\"}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("overlapping-workspace.json", overlappingWorkspaceText, manifest, error)));

    const String implicitDepsOverlapText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"build\":\"deps\"}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("implicit-deps-overlap.json", implicitDepsOverlapText, manifest, error)));

    const String implicitBuildOverlapText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"deps\":\"build\"}}";
    SLANG_CHECK(SLANG_FAILED(readManifestText(
        "implicit-build-overlap.json",
        implicitBuildOverlapText,
        manifest,
        error)));

    const String nestedWorkspaceText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],"
        "\"dependencies\":{},\"workspace\":{\"deps\":\"state/deps\",\"build\":\"state\"}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("nested-workspace.json", nestedWorkspaceText, manifest, error)));

    const String disabledBundleText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"workspace\":{\"bundle\":{\"modules\":false,"
        "\"source\":true}}}";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readManifestText("disabled-bundle.json", disabledBundleText, manifest, error)));
    SLANG_CHECK(!manifest.workspace.bundle.modules);
    SLANG_CHECK(manifest.workspace.bundle.source);

    const String unknownBundleFieldText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"workspace\":{\"bundle\":{\"modules\":true,"
        "\"cache\":true}}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("unknown-bundle-field.json", unknownBundleFieldText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Unknown field")) >= 0);

    const String slangToolchainText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"tools\":{\"slang-toolchain\":{\"version\":\">=2026.8."
        "0\"}}}";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readManifestText("slang-toolchain.json", slangToolchainText, manifest, error)));
    SLANG_CHECK(manifest.slangToolchainConstraint == ">=2026.8.0");

    const String unknownToolText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"tools\":{\"dxc\":{\"version\":\">=1.0.0\"}}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("unknown-tool.json", unknownToolText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("slang-toolchain")) >= 0);

    const String invalidExecutableNameText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"host\":{\"executables\":[\"bin/root\"]}}";
    SLANG_CHECK(SLANG_FAILED(readManifestText(
        "invalid-executable-name.json",
        invalidExecutableNameText,
        manifest,
        error)));

    const String unknownHostFieldText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"host\":{\"executables\":[\"root\"],\"source\":\"main\"}"
        "}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("unknown-host-field.json", unknownHostFieldText, manifest, error)));

    const String missingDefaultText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"host\":{\"executables\":[\"one\",\"two\"]}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("missing-host-default.json", missingDefaultText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("default")) >= 0);

    const String implicitDefaultText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"host\":{\"executables\":[\"only-tool\"]}}";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readManifestText("implicit-host-default.json", implicitDefaultText, manifest, error)));
    SLANG_CHECK(manifest.host.defaultExecutable == "only-tool");

    const String legacyExecutableText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"executable\":{\"name\":\"root\"}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("legacy-executable.json", legacyExecutableText, manifest, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Unknown field")) >= 0);

    const String invalidRetractionText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"retractions\":[{\"version\":\"v1.0.0\","
        "\"reason\":\"bad\"}]}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("invalid-retraction.json", invalidRetractionText, manifest, error)));

    const String invalidExclusionText =
        "{\"schema_version\":1,\"name\":\"root\",\"exports\":[\"src\"],\"license_files\":["
        "\"LICENSE\"],\"dependencies\":{},\"workspace\":{\"excludes\":[{\"package\":\"bad/name\","
        "\"version\":\"1.0.0\",\"reason\":\"bad\"}]}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("invalid-exclusion.json", invalidExclusionText, manifest, error)));
}

SLANG_UNIT_TEST(PackageLockRejectsUnknownFields)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String path = Path::combine(temp.path, "slang-package-lock.json");
    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(path, "{\"lock_version\":1,\"packages\":{}}")));
    SLANG_CHECK(SLANG_FAILED(readLockFile(path, lock, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Unknown field")) >= 0);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(path, "{\"packages\":{}}")));
    SLANG_CHECK(SLANG_FAILED(readLockFile(path, lock, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("required")) >= 0);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(path, "{\"schema_version\":2,\"packages\":{}}")));
    SLANG_CHECK(SLANG_FAILED(readLockFile(path, lock, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("integer 1")) >= 0);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(path, "{\"schema_version\":1,\"packages\":{}}")));
    SLANG_CHECK(SLANG_SUCCEEDED(readLockFile(path, lock, error)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(
        path,
        "{\"schema_version\":1,\"packages\":{},\"tools\":{\"slang-toolchain\":{\"version\":"
        "\"2026.8.1\"}}}")));
    SLANG_CHECK(SLANG_FAILED(readLockFile(path, lock, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Unknown field")) >= 0);
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
    package.as = "1.2.0";
    package.enabled = false;
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
    SLANG_CHECK(!roundTrip[1].enabled);

    String workspaceText;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(path, workspaceText)));
    SLANG_CHECK(workspaceText.getUnownedSlice().indexOf(UnownedStringSlice("\"enabled\"")) >= 0);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(
        path,
        "{\"schema_version\":1,\"overrides\":{\"noise\":{\"path\":\"../noise\","
        "\"as\":\"1.2.0\"}}}")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readLocalPackages(path, roundTrip, error)));
    SLANG_CHECK(roundTrip[0].enabled);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(
        path,
        "{\"schema_version\":3,\"overrides\":{\"noise\":{\"path\":\"../noise\"}}}")));
    SLANG_CHECK(SLANG_FAILED(readLocalPackages(path, roundTrip, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Unsupported")) >= 0);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(
        path,
        "{\"schema_version\":1,\"overrides\":{\"noise\":{\"path\":\"/absolute/noise\"}}}")));
    SLANG_CHECK(SLANG_FAILED(readLocalPackages(path, roundTrip, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(
        path,
        "{\"schema_version\":1,\"edits\":{\"noise\":{\"unknown\":\"bad\"}}}")));
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
    SemanticVersion installedToolchain;
    String installedToolchainText;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        getInstalledSlangToolchainVersion(installedToolchain, installedToolchainText, error)));
    SLANG_CHECK(manifest.slangToolchainConstraint == String(">=") + installedToolchainText);

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

SLANG_UNIT_TEST(PackageToolSlangToolchain)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    SemanticVersion installedToolchain;
    String installedToolchainText;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        getInstalledSlangToolchainVersion(installedToolchain, installedToolchainText, error)));

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    PackageTool::LockFile lock;
    String lockPath = Path::combine(temp.path, "slang-package-lock.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readLockFile(lockPath, lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 0);
    String lockText;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(lockPath, lockText)));
    SLANG_CHECK(lockText.getUnownedSlice().indexOf(UnownedStringSlice("\"tools\"")) < 0);

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    root.slangToolchainConstraint = ">=2027.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("slang-toolchain")) >= 0);

    root.slangToolchainConstraint = String(">=") + installedToolchainText;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    const char* fetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    const char* statusArguments[] = {"slang-package", "status"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));

    root.slangToolchainConstraint = ">=2027.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("slang-toolchain")) >= 0);
    root.slangToolchainConstraint = String(">=") + installedToolchainText;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    String vendorRoot = Path::combine(temp.path, "vendor/noise");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(vendorRoot));
    Manifest vendor;
    vendor.name = "noise";
    vendor.exports.add("src");
    vendor.licenseFiles.add("LICENSE");
    vendor.slangToolchainConstraint = ">=2027.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(vendorRoot, "slang-package.json"), vendor, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(vendorRoot, "LICENSE"), "Noise license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(vendorRoot, "src/noise.slang"), "module noise;\n")));

    Dependency pathDep;
    pathDep.name = "noise";
    pathDep.path = "vendor/noise";
    pathDep.as = "1.0.0";
    root.dependencies.add(pathDep);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("noise")) >= 0);
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("slang-toolchain")) >= 0);
}

SLANG_UNIT_TEST(PackageToolBuild)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));
    Manifest manifest;
    String manifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(manifestPath, manifest, error)));
    manifest.workspace.buildDirectory = "out";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(manifestPath, manifest, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/acme/noise.slang"),
        "module noise;\n"
        "__include \"noise/helper\";\n"
        "public int getNoise() { return helper(); }\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/acme/noise/helper.slang"),
        "implementing noise;\n"
        "int helper() { return 1; }\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/main.slang"),
        "module main;\n"
        "import acme.noise;\n"
        "public int getValue() { return getNoise(); }\n")));

    const char* buildArguments[] = {"slang-package", "build"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(buildArguments), buildArguments, error)));
    SLANG_CHECK(
        File::exists(Path::combine(temp.path, "out/bundle/modules/acme/noise.slang-module")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "out/bundle/modules/main.slang-module")));
    SLANG_CHECK(!File::exists(
        Path::combine(temp.path, "out/bundle/modules/acme/noise/helper.slang-module")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "out/bundle/source/acme/noise.slang")));
    SLANG_CHECK(
        File::exists(Path::combine(temp.path, "out/bundle/source/acme/noise/helper.slang")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "out/bundle/source/main.slang")));
    String provenance;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(
        Path::combine(temp.path, "out/bundle/modules/provenance.json"),
        provenance)));
    SLANG_CHECK(provenance.getUnownedSlice().indexOf(UnownedStringSlice("slang-modules")) >= 0);
    SLANG_CHECK(provenance.getUnownedSlice().indexOf(UnownedStringSlice("slangc")) >= 0);
    String docsIndex;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "out/docs/index.md"), docsIndex)));
    String unlinkedWorkspace = String("- ") + manifest.name + "\n";
    SLANG_CHECK(docsIndex.getUnownedSlice().indexOf(unlinkedWorkspace.getUnownedSlice()) >= 0);
    String linkedWorkspace = String("[") + manifest.name + "](#" + manifest.name + ")";
    SLANG_CHECK(docsIndex.getUnownedSlice().indexOf(linkedWorkspace.getUnownedSlice()) < 0);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/main.slang"),
        "module main;\n"
        "int broken() { return missingValue; }\n")));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(buildArguments), buildArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("undefined identifier")) >= 0);
}

SLANG_UNIT_TEST(PackageToolBundleFlags)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));
    Manifest manifest;
    String manifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(manifestPath, manifest, error)));
    manifest.workspace.bundle.modules = false;
    manifest.workspace.bundle.source = false;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(manifestPath, manifest, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/library.slang"),
        "module library;\n"
        "public int getValue() { return 1; }\n")));

    const char* buildArguments[] = {"slang-package", "build"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(buildArguments), buildArguments, error)));
    SLANG_CHECK(
        !File::exists(Path::combine(temp.path, "build/bundle/modules/library.slang-module")));
    SLANG_CHECK(!File::exists(Path::combine(temp.path, "build/bundle/modules/provenance.json")));
    SLANG_CHECK(!File::exists(Path::combine(temp.path, "build/bundle/source/library.slang")));
}

SLANG_UNIT_TEST(PackageToolUpdateRejectsBundleCaseConflict)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    String leftRoot = Path::combine(temp.path, "vendor/left");
    String rightRoot = Path::combine(temp.path, "vendor/right");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(leftRoot));
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(rightRoot));
    Manifest left;
    left.name = "left";
    left.exports.add("src");
    left.licenseFiles.add("LICENSE");
    Manifest right;
    right.name = "right";
    right.exports.add("src");
    right.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(leftRoot, "slang-package.json"), left, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(rightRoot, "slang-package.json"), right, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(leftRoot, "LICENSE"), "Left license\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(rightRoot, "LICENSE"), "Right license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(leftRoot, "src/Util.slang"),
        "module Util;\n"
        "public int leftValue() { return 1; }\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(rightRoot, "src/util.slang"),
        "module util;\n"
        "public int rightValue() { return 2; }\n")));

    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    Manifest root;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency leftDep;
    leftDep.name = "left";
    leftDep.path = "vendor/left";
    leftDep.as = "1.0.0";
    Dependency rightDep;
    rightDep.name = "right";
    rightDep.path = "vendor/right";
    rightDep.as = "1.0.0";
    root.dependencies.add(leftDep);
    root.dependencies.add(rightDep);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/main.slang"),
        "module main;\n"
        "public int getValue() { return 0; }\n")));

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("case-insensitive")) >= 0);
    SLANG_CHECK(!File::exists(Path::combine(temp.path, "slang-package-lock.json")));
}

SLANG_UNIT_TEST(PackageToolRun)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));
    Manifest manifest;
    String manifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(manifestPath, manifest, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/library.slang"),
        "module library;\n"
        "public int getValue() { return 0; }\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/package-run-test.slang"),
        "module package_run_test;\n"
        "import library;\n"
        "export __extern_cpp int main(int argc, Ptr<NativeString> argv)\n"
        "{\n"
        "    return getValue() + (argc == 2 ? 0 : 1);\n"
        "}\n")));

    const char* stableRunArguments[] = {"slang-package", "run", "argument"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(stableRunArguments),
        stableRunArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("experimental")) >= 0);

    const char* runArguments[] = {"slang-package", "--experimental", "run", "argument"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(runArguments), runArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("does not configure")) >= 0);

    manifest.host.executables.add("package-run-test");
    manifest.host.defaultExecutable = "package-run-test";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(manifestPath, manifest, error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(runArguments), runArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("has not been built")) >= 0);

    const char* stableBuildArguments[] = {"slang-package", "build"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(stableBuildArguments),
        stableBuildArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("experimental")) >= 0);

    const char* buildArguments[] = {"slang-package", "--experimental", "build"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(buildArguments), buildArguments, error)));
    String executablePath =
        Path::combine(temp.path, String("build/package-run-test") + Process::getExecutableSuffix());
    SLANG_CHECK(File::exists(executablePath));
    SLANG_CHECK(File::exists(
        Path::combine(temp.path, "build/bundle/modules/package-run-test.slang-module")));

    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::remove(Path::combine(temp.path, "src/package-run-test.slang"))));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(runArguments), runArguments, error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(buildArguments), buildArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("does not export")) >= 0);
}

SLANG_UNIT_TEST(PackageToolMultipleHostExecutables)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));
    Manifest manifest;
    String manifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(manifestPath, manifest, error)));
    manifest.host.executables.add("alpha");
    manifest.host.executables.add("beta");
    manifest.host.defaultExecutable = "alpha";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(manifestPath, manifest, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/alpha.slang"),
        "module alpha;\n"
        "export __extern_cpp int main(int argc, Ptr<NativeString> argv)\n"
        "{\n"
        "    return argc == 1 ? 0 : 1;\n"
        "}\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/beta.slang"),
        "module beta;\n"
        "export __extern_cpp int main(int argc, Ptr<NativeString> argv)\n"
        "{\n"
        "    return argc == 2 ? 0 : 1;\n"
        "}\n")));

    const char* buildArguments[] = {"slang-package", "--experimental", "build"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(buildArguments), buildArguments, error)));
    SLANG_CHECK(File::exists(
        Path::combine(temp.path, String("build/alpha") + Process::getExecutableSuffix())));
    SLANG_CHECK(File::exists(
        Path::combine(temp.path, String("build/beta") + Process::getExecutableSuffix())));

    const char* defaultRun[] = {"slang-package", "--experimental", "run"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(defaultRun), defaultRun, error)));
    const char* namedRun[] = {"slang-package", "--experimental", "run", "beta", "arg"};
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(executeInDirectory(temp.path, SLANG_COUNT_OF(namedRun), namedRun, error)));
}

SLANG_UNIT_TEST(PackageToolExecutableRequiresWorkspaceSource)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    String dependencyRoot = Path::combine(temp.path, "vendor/tool");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(dependencyRoot));
    Manifest dependency;
    dependency.name = "tool";
    dependency.exports.add("src");
    dependency.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(dependencyRoot, "slang-package.json"), dependency, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(dependencyRoot, "LICENSE"), "Tool license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(dependencyRoot, "src/root-tool.slang"),
        "module root_tool;\n"
        "public int dependencyMain() { return 0; }\n")));

    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    Manifest root;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    root.host.executables.add("root-tool");
    root.host.defaultExecutable = "root-tool";
    Dependency toolDependency;
    toolDependency.name = "tool";
    toolDependency.path = "vendor/tool";
    toolDependency.as = "1.0.0";
    root.dependencies.add(toolDependency);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    const char* buildArguments[] = {"slang-package", "--experimental", "build"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(buildArguments), buildArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("root-tool.slang")) >= 0);
}

SLANG_UNIT_TEST(PackageToolTest)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));

    const char* testArguments[] = {"slang-package", "test"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(testArguments), testArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("not implemented")) >= 0);
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("slang-test")) >= 0);
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

SLANG_UNIT_TEST(PackageToolUpdateRequiresConfirmation)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    const char* statusArguments[] = {"slang-package", "status"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));

    List<LocalPackage> localPackages;
    LocalPackage localPackage;
    localPackage.name = "noise";
    localPackage.path = "../noise";
    localPackage.as = "1.0.0";
    localPackages.add(localPackage);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeProjectLocalPackages(temp.path, localPackages, error)));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("local package registrations")) >= 0);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::remove(Path::combine(temp.path, "slang-workspace.json"))));

    const char* updateArguments[] = {"slang-package", "update"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("--yes")) >= 0);
    SLANG_CHECK(!File::exists(Path::combine(temp.path, "slang-package-lock.json")));

    const char* confirmedUpdateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(confirmedUpdateArguments),
        confirmedUpdateArguments,
        error)));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "slang-package-lock.json")));
}

SLANG_UNIT_TEST(PackageToolDependencyCommandsAndInitialFetch)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    String bRoot = Path::combine(temp.path, "vendor/b");
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

    String aRoot = Path::combine(temp.path, "vendor/a");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(aRoot));
    Manifest a;
    a.name = "a";
    a.exports.add("src");
    a.licenseFiles.add("LICENSE");
    Dependency bDependency;
    bDependency.name = "b";
    bDependency.path = "../b";
    bDependency.as = "1.0.0";
    a.dependencies.add(bDependency);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(aRoot, "slang-package.json"), a, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(Path::combine(aRoot, "LICENSE"), "A license\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(aRoot, "src/a.slang"), "module a;\n")));

    const char* addArguments[] = {
        "slang-package",
        "dependency",
        "add",
        "a",
        "--path",
        "vendor/a",
        "--as",
        "1.0.0",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(addArguments), addArguments, error)));
    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    SLANG_CHECK(root.dependencies.getCount() == 1);
    SLANG_CHECK(root.dependencies[0].name == "a");
    SLANG_CHECK(root.dependencies[0].path == "vendor/a");
    SLANG_CHECK(root.dependencies[0].as == "1.0.0");
    SLANG_CHECK(!File::exists(Path::combine(temp.path, ".slang-package.json.validate.tmp")));

    const char* replaceArguments[] = {
        "slang-package",
        "dependency",
        "add",
        "a",
        "--path",
        "vendor/a",
        "--as",
        "1.1.0",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(replaceArguments), replaceArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    SLANG_CHECK(root.dependencies.getCount() == 1);
    SLANG_CHECK(root.dependencies[0].as == "1.1.0");

    const char* addGitArguments[] = {
        "slang-package",
        "dependency",
        "add",
        "remote",
        "--git",
        "https://example.com/remote.git",
        "--version",
        ">=1.0.0",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(addGitArguments), addGitArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    SLANG_CHECK(root.dependencies.getCount() == 2);
    const char* removeGitArguments[] = {"slang-package", "dependency", "remove", "remote"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(removeGitArguments),
        removeGitArguments,
        error)));

    String manifestBeforeInvalidAdd;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::readAllText(rootManifestPath, manifestBeforeInvalidAdd)));
    const char* invalidSelectorArguments[] = {
        "slang-package",
        "dependency",
        "add",
        "bad",
        "--git",
        "https://example.com/bad.git",
        "--path",
        "vendor/a",
    };
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(invalidSelectorArguments),
        invalidSelectorArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("exactly one")) >= 0);
    const char* missingValueArguments[] = {
        "slang-package",
        "dependency",
        "add",
        "bad",
        "--git",
    };
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(missingValueArguments),
        missingValueArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Missing value")) >= 0);
    const char* unknownOptionArguments[] = {
        "slang-package",
        "dependency",
        "add",
        "bad",
        "--unknown",
        "value",
    };
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(unknownOptionArguments),
        unknownOptionArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Unknown")) >= 0);
    const char* invalidNameArguments[] = {
        "slang-package",
        "dependency",
        "add",
        "bad/name",
        "--path",
        "vendor/a",
        "--as",
        "1.0.0",
    };
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(invalidNameArguments),
        invalidNameArguments,
        error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("Invalid dependency name")) >= 0);
    String manifestAfterInvalidAdd;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::readAllText(rootManifestPath, manifestAfterInvalidAdd)));
    SLANG_CHECK(manifestAfterInvalidAdd == manifestBeforeInvalidAdd);

    const char* listArguments[] = {"slang-package", "dependency", "list"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(listArguments), listArguments, error)));

    const char* unconfirmedFetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(unconfirmedFetchArguments),
        unconfirmedFetchArguments,
        error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("--yes")) >= 0);
    SLANG_CHECK(!File::exists(Path::combine(temp.path, "slang-package-lock.json")));
    const char* fetchArguments[] = {"slang-package", "fetch", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "slang-package-lock.json")));
    const char* treeArguments[] = {"slang-package", "tree"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(treeArguments), treeArguments, error)));
    const char* whyArguments[] = {"slang-package", "why", "b"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(whyArguments), whyArguments, error)));

    const char* removeArguments[] = {"slang-package", "dependency", "remove", "a"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(removeArguments), removeArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    SLANG_CHECK(root.dependencies.getCount() == 0);
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(removeArguments), removeArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("does not declare")) >= 0);
    const char* statusArguments[] = {"slang-package", "status"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("unreachable")) >= 0);
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
    locked.version = "1.0.0";
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

SLANG_UNIT_TEST(PackageToolFetchRejectsWorkspaceExclusion)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency dependency;
    dependency.name = "noise";
    dependency.git = "memory:noise";
    dependency.version = ">=1.0.0";
    root.dependencies.add(dependency);
    Exclusion exclusion;
    exclusion.packageName = "noise";
    exclusion.version = "1.0.0";
    exclusion.reason = "Known regression";
    root.workspace.exclusions.add(exclusion);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    LockedPackage locked;
    locked.name = "noise";
    locked.git = dependency.git;
    locked.ref = "v1.0.0";
    locked.version = "1.0.0";
    locked.commit = "0000000000000000000000000000000000000000";
    locked.exports.add("src");
    PackageTool::LockFile lock;
    lock.packages.add(locked);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));

    const char* fetchArguments[] = {"slang-package", "fetch"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Known regression")) >= 0);
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("package update")) >= 0);
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
    pathDep.as = "1.0.0";
    root.dependencies.add(pathDep);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("package-tool state under .slang")) >=
        0);

    LockedPackage locked;
    locked.name = "evil";
    locked.path = ".slang/evil";
    locked.version = "1.0.0";
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
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(temp.path, "docs/guide.md"), "# Root guide\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(temp.path, "docs/ignored.txt"), "Not docs\n")));

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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(bRoot, "src/b.slang"),
        "module b;\n"
        "import c;\n"
        "public int bValue() { return cValue(); }\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(bRoot, "docs/reference/api.md"), "# B API\n")));

    String cRoot = Path::combine(temp.path, "vendor/c");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(cRoot));
    Manifest c;
    c.name = "c";
    c.exports.add("src");
    c.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(cRoot, "slang-package.json"), c, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(Path::combine(cRoot, "LICENSE"), "C license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(cRoot, "src/c.slang"),
        "module c;\n"
        "public int cValue() { return 1; }\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(cRoot, "docs/reference.md"), "# C reference\n")));

    Manifest a;
    a.name = "a";
    a.exports.add("src");
    a.licenseFiles.add("LICENSE");
    Dependency bPath;
    bPath.name = "b";
    bPath.path = "vendor/b";
    bPath.as = "1.0.0";
    a.dependencies.add(bPath);
    Dependency cPath;
    cPath.name = "c";
    cPath.path = "../c";
    cPath.as = "1.0.0";
    a.dependencies.add(cPath);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeManifest(Path::combine(aRoot, "slang-package.json"), a, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(Path::combine(aRoot, "LICENSE"), "A license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(aRoot, "src/a.slang"),
        "module a;\n"
        "import b;\n"
        "import c;\n"
        "public int aValue() { return bValue() + cValue(); }\n")));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(aRoot, "docs/readme.md"), "# A readme\n")));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency bGit;
    bGit.name = "b";
    bGit.git = "memory:b";
    bGit.version = ">=1.0.0";
    root.dependencies.add(bGit);
    Dependency aPath;
    aPath.name = "a";
    aPath.path = "vendor/a";
    aPath.as = "1.0.0";
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

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    PackageTool::LockFile lock;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));
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

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        Path::combine(temp.path, "src/main.slang"),
        "module main;\n"
        "import a;\n"
        "public int useA() { return aValue(); }\n")));
    const char* buildArguments[] = {"slang-package", "build"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(buildArguments), buildArguments, error)));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "build/bundle/modules/a.slang-module")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "build/bundle/modules/b.slang-module")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "build/bundle/modules/c.slang-module")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "build/bundle/modules/main.slang-module")));

    String shippedConsumerPath = Path::combine(temp.path, "shipped/consumer.slang");
    String shippedOutputPath = Path::combine(temp.path, "shipped/consumer.slang-module");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(
        shippedConsumerPath,
        "module consumer;\n"
        "import a;\n"
        "public int consume() { return aValue(); }\n")));
    List<String> slangcArguments;
    slangcArguments.add(shippedConsumerPath);
    slangcArguments.add("-I");
    slangcArguments.add(Path::combine(temp.path, "build/bundle/modules"));
    slangcArguments.add("-o");
    slangcArguments.add(shippedOutputPath);
    ExecuteResult slangcResult;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runSlangc(slangcArguments, slangcResult)));
    if (slangcResult.resultCode != 0)
        getTestReporter()->message(TestMessageType::Info, slangcResult.standardError.getBuffer());
    SLANG_CHECK(slangcResult.resultCode == 0);
    SLANG_CHECK(File::exists(shippedOutputPath));
    SLANG_CHECK(
        File::exists(Path::combine(Path::combine(temp.path, "build/docs", root.name), "guide.md")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "build/docs/a/readme.md")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "build/docs/b/reference/api.md")));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "build/docs/c/reference.md")));
    SLANG_CHECK(!File::exists(
        Path::combine(Path::combine(temp.path, "build/docs", root.name), "ignored.txt")));
    String docsIndex;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "build/docs/index.md"), docsIndex)));
    SLANG_CHECK(docsIndex.getUnownedSlice().indexOf(UnownedStringSlice("[a](#a)")) >= 0);
    SLANG_CHECK(docsIndex.getUnownedSlice().indexOf(UnownedStringSlice("[b](#b)")) >= 0);
    SLANG_CHECK(docsIndex.getUnownedSlice().indexOf(UnownedStringSlice("[c](#c)")) >= 0);
    SLANG_CHECK(
        docsIndex.getUnownedSlice().indexOf(UnownedStringSlice("[readme.md](a/readme.md)")) >= 0);
    SLANG_CHECK(
        docsIndex.getUnownedSlice().indexOf(
            UnownedStringSlice("[reference/api.md](b/reference/api.md)")) >= 0);
    SLANG_CHECK(
        docsIndex.getUnownedSlice().indexOf(UnownedStringSlice("[reference.md](c/reference.md)")) >=
        0);
    String rootGuideLink = String("[guide.md](") + root.name + "/guide.md)";
    SLANG_CHECK(docsIndex.getUnownedSlice().indexOf(rootGuideLink.getUnownedSlice()) >= 0);
    const char* docsArguments[] = {"slang-package", "docs"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(docsArguments), docsArguments, error)));

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
    conflictingB.as = "1.0.0";
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
    dependency.version = ">=1.0.0";
    root.dependencies.add(dependency);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    LockedPackage locked;
    locked.name = "noise";
    locked.git = dependency.git;
    locked.ref = "v1.0.0";
    locked.version = "1.0.0";
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
        "2.0.0",
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(helperOverrideArguments),
        helperOverrideArguments,
        error)));

    root.dependencies[0].version = ">=5.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));
    const char* incompatibleLocalUpdateArguments[] = {
        "slang-package",
        "update",
        "--from-local",
        "--dry-run",
    };
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(incompatibleLocalUpdateArguments),
        incompatibleLocalUpdateArguments,
        error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("No package selection satisfies")) >= 0);
    root.dependencies[0].version = ">=1.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* validateArguments[] = {"slang-package", "validate"};
    SLANG_CHECK(SLANG_FAILED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(validateArguments),
        validateArguments,
        error)));

    String lockBeforeDryRun;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "slang-package-lock.json"), lockBeforeDryRun)));
    const char* dryRunArguments[] = {"slang-package", "update", "--from-local", "--dry-run"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(dryRunArguments), dryRunArguments, error)));
    String lockAfterDryRun;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "slang-package-lock.json"), lockAfterDryRun)));
    SLANG_CHECK(lockAfterDryRun == lockBeforeDryRun);

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 2);
    for (const auto& package : lock.packages)
    {
        SLANG_CHECK(package.path.getLength() != 0);
        SLANG_CHECK(package.ref.getLength() == 0);
        SLANG_CHECK(package.version.getLength() != 0);
        SLANG_CHECK(package.commit.getLength() == 0);
    }
    SLANG_CHECK(lock.packages[0].name == "helper");
    SLANG_CHECK(lock.packages[1].name == "noise");
    SLANG_CHECK(lock.packages[1].dependencies.getCount() == 2);
    SLANG_CHECK(lock.packages[1].dependencies[0].name == "helper");
    SLANG_CHECK(lock.packages[1].dependencies[1].name == "noise");

    const char* disableNoiseArguments[] = {"slang-package", "override", "disable", "noise"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(disableNoiseArguments),
        disableNoiseArguments,
        error)));
    List<LocalPackage> toggledPackages;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readProjectLocalPackages(temp.path, toggledPackages, error)));
    Index toggledNoiseIndex = findLocalPackageIndex(toggledPackages, "noise");
    SLANG_CHECK_ABORT(toggledNoiseIndex >= 0);
    SLANG_CHECK(!toggledPackages[toggledNoiseIndex].enabled);
    const char* enableNoiseArguments[] = {"slang-package", "override", "enable", "noise"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(enableNoiseArguments),
        enableNoiseArguments,
        error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readProjectLocalPackages(temp.path, toggledPackages, error)));
    toggledNoiseIndex = findLocalPackageIndex(toggledPackages, "noise");
    SLANG_CHECK_ABORT(toggledNoiseIndex >= 0);
    SLANG_CHECK(toggledPackages[toggledNoiseIndex].enabled);
    const char* listOverrideArguments[] = {"slang-package", "override", "list"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(listOverrideArguments),
        listOverrideArguments,
        error)));

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
        "1.0.0",
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
    SlangResult fetchResult =
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error);
    if (SLANG_FAILED(fetchResult))
        getTestReporter()->message(TestMessageType::Info, error.getBuffer());
    SLANG_CHECK(SLANG_SUCCEEDED(fetchResult));
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

SLANG_UNIT_TEST(PackageAppendErrorAdviceUsesNewline)
{
    String error = "Cannot read JSON file: /tmp/pkg/slang-package.json";
    appendErrorAdvice(error, "Run 'slang package fetch' if packages are missing.");
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(
            UnownedStringSlice("slang-package.json\nRun 'slang package fetch'")) >= 0);
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice(".json Run")) < 0);
}

SLANG_UNIT_TEST(PackageToolUpdateDryRun)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    auto writePathPackage = [&](const String& directory, const String& name) -> SlangResult
    {
        SLANG_RETURN_ON_FAIL(Path::createDirectoryRecursive(directory) ? SLANG_OK : SLANG_FAIL);
        Manifest package;
        package.name = name;
        package.exports.add("src");
        package.licenseFiles.add("LICENSE");
        SLANG_RETURN_ON_FAIL(
            writeManifest(Path::combine(directory, "slang-package.json"), package, error));
        SLANG_RETURN_ON_FAIL(_writeFile(Path::combine(directory, "LICENSE"), name + " license\n"));
        return _writeFile(
            Path::combine(directory, "src", name + ".slang"),
            String("module ") + name + ";\n");
    };

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writePathPackage(Path::combine(temp.path, "vendor/a"), "a")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writePathPackage(Path::combine(temp.path, "vendor/b"), "b")));

    Manifest root;
    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency a;
    a.name = "a";
    a.path = "vendor/a";
    a.as = "1.0.0";
    root.dependencies.add(a);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update", "--yes"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    PackageTool::LockFile lock;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 1);
    SLANG_CHECK(lock.packages[0].name == "a");

    Dependency b;
    b.name = "b";
    b.path = "vendor/b";
    b.as = "1.0.0";
    root.dependencies.add(b);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    String lockBeforeDryRun;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "slang-package-lock.json"), lockBeforeDryRun)));
    const char* dryRunArguments[] = {"slang-package", "update", "--dry-run"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(dryRunArguments), dryRunArguments, error)));
    String lockAfterDryRun;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::readAllText(Path::combine(temp.path, "slang-package-lock.json"), lockAfterDryRun)));
    SLANG_CHECK(lockAfterDryRun == lockBeforeDryRun);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readLockFile(Path::combine(temp.path, "slang-package-lock.json"), lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 2);
    const char* statusArguments[] = {"slang-package", "status"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(statusArguments), statusArguments, error)));
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

SLANG_UNIT_TEST(PackageCommandsValidateDependencyModuleLayout)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));
    String error;
    const char* initArguments[] = {"slang-package", "init"};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(Path::combine(temp.path, "LICENSE"), "Root license\n")));

    String dependencyRoot = Path::combine(temp.path, "vendor/noise");
    Manifest noise;
    noise.name = "noise";
    noise.exports.add("src");
    noise.licenseFiles.add("LICENSE");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(dependencyRoot));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        writeManifest(Path::combine(dependencyRoot, "slang-package.json"), noise, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_writeFile(Path::combine(dependencyRoot, "LICENSE"), "Noise license\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _writeFile(Path::combine(dependencyRoot, "src/noise.slang"), "module noise;\n")));
    String companionPath = Path::combine(dependencyRoot, "src/noise/helper.slang");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_writeFile(companionPath, "module helper;\n")));

    String rootManifestPath = Path::combine(temp.path, "slang-package.json");
    Manifest root;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(readManifest(rootManifestPath, root, error)));
    Dependency dependency;
    dependency.name = "noise";
    dependency.path = "vendor/noise";
    dependency.as = "1.0.0";
    root.dependencies.add(dependency);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeManifest(rootManifestPath, root, error)));

    const char* updateArguments[] = {"slang-package", "update", "--minimal", "--yes"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(updateArguments), updateArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Companion")) >= 0);
    SLANG_CHECK(!File::exists(Path::combine(temp.path, "slang-package-lock.json")));

    const char* skipUpdateArguments[] = {
        "slang-package",
        "update",
        "--minimal",
        "--skip-validate",
        "--yes",
    };
    error = String();
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(skipUpdateArguments),
        skipUpdateArguments,
        error)));
    SLANG_CHECK(File::exists(Path::combine(temp.path, "slang-package-lock.json")));

    const char* fetchArguments[] = {"slang-package", "fetch"};
    error = String();
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("Companion")) >= 0);

    const char* skipFetchArguments[] = {"slang-package", "fetch", "--skip-validate"};
    error = String();
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeInDirectory(
        temp.path,
        SLANG_COUNT_OF(skipFetchArguments),
        skipFetchArguments,
        error)));
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
    locked.ref = "v1.0.0";
    locked.version = "1.0.0";
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
        release.candidate.ref = String("v") + version;
        release.candidate.commit = release.candidate.ref;
        SLANG_RELEASE_ASSERT(SLANG_SUCCEEDED(
            SemanticVersion::parse(version.getUnownedSlice(), release.candidate.version)));
        releases.add(release);
    }

    void addRef(
        const String& git,
        const String& ref,
        const String& version,
        const Manifest& manifest)
    {
        InMemoryRelease release;
        release.git = git;
        release.manifest = manifest;
        release.candidate.ref = ref;
        release.candidate.commit = String("commit-") + ref;
        SLANG_RELEASE_ASSERT(SLANG_SUCCEEDED(
            SemanticVersion::parse(version.getUnownedSlice(), release.candidate.version)));
        releases.add(release);
    }

    virtual SlangResult resolveReference(
        const String&,
        const String& git,
        const String& ref,
        TagCandidate& outCandidate,
        String& outError) override
    {
        for (const auto& release : releases)
        {
            if (release.git == git && release.candidate.ref == ref)
            {
                outCandidate = release.candidate;
                return SLANG_OK;
            }
        }
        outError = String("Missing in-memory ref for ") + git + "@" + ref;
        return SLANG_FAIL;
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
            if (release.git == git && release.candidate.ref == candidate.ref)
            {
                outManifest.manifest = release.manifest;
                outManifest.sourceRoot = release.sourceRoot;
                outManifest.lockRoot = Path::combine("deps", outManifest.manifest.name);
                return SLANG_OK;
            }
        }
        outError = String("Missing in-memory manifest for ") + git + "@" + candidate.ref;
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
    SLANG_CHECK(a && a->ref == "v1.0.0");
    SLANG_CHECK(b && b->ref == "v1.4.0");
}

SLANG_UNIT_TEST(PackageResolverSlangToolchain)
{
    InMemoryPackageSource source;
    source.addRelease("memory:noise", "1.0.0", _makeManifest("noise"));

    Manifest root = _makeManifest("root");
    _addDependency(root, "noise", "memory:noise", ">=1.0.0");
    root.slangToolchainConstraint = ">=2027.0.0";

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK(SLANG_FAILED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("slang-toolchain")) >= 0);

    SemanticVersion installed;
    String installedText;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(getInstalledSlangToolchainVersion(installed, installedText, error)));
    root.slangToolchainConstraint = String(">=") + installedText;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(_findLockedPackage(lock, "noise") != nullptr);

    Manifest dep = _makeManifest("noise");
    dep.slangToolchainConstraint = ">=2027.0.0";
    source.releases.clear();
    source.addRelease("memory:noise", "1.0.0", dep);
    SLANG_CHECK(SLANG_FAILED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("noise")) >= 0);
}

SLANG_UNIT_TEST(PackageResolverNotEqual)
{
    InMemoryPackageSource source;
    source.addRelease("memory:b", "1.2.0", _makeManifest("b"));
    source.addRelease("memory:b", "1.3.0", _makeManifest("b"));
    source.addRelease("memory:b", "1.4.0", _makeManifest("b"));

    Manifest a = _makeManifest("a");
    _addDependency(a, "b", "memory:b", ">=1.0.0 !=1.4.0");
    source.addRelease("memory:a", "1.0.0", a);

    Manifest root = _makeManifest("root");
    _addDependency(root, "a", "memory:a", ">=1.0.0");
    _addDependency(root, "b", "memory:b", ">=1.0.0");

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    const LockedPackage* b = _findLockedPackage(lock, "b");
    SLANG_CHECK(b && b->ref == "v1.3.0");
}

SLANG_UNIT_TEST(PackageResolverDisjunction)
{
    InMemoryPackageSource source;
    source.addRelease("memory:b", "1.2.0", _makeManifest("b"));
    source.addRelease("memory:b", "1.3.0", _makeManifest("b"));
    source.addRelease("memory:b", "1.4.0", _makeManifest("b"));

    Manifest root = _makeManifest("root");
    _addDependency(root, "b", "memory:b", ">=1.0.0 <1.3.0 || >=1.4.0");

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    const LockedPackage* b = _findLockedPackage(lock, "b");
    SLANG_CHECK(b && b->ref == "v1.4.0");

    root.dependencies[0].version = ">=1.0.0 <1.1.0 || >=9.0.0";
    SLANG_CHECK(SLANG_FAILED(resolveDependenciesWithSource(root, source, lock, error)));
}

SLANG_UNIT_TEST(PackageResolverSlangToolchainNotEqual)
{
    InMemoryPackageSource source;
    source.addRelease("memory:noise", "1.0.0", _makeManifest("noise"));

    Manifest root = _makeManifest("root");
    _addDependency(root, "noise", "memory:noise", ">=1.0.0");

    SemanticVersion installed;
    String installedText;
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(getInstalledSlangToolchainVersion(installed, installedText, error)));
    root.slangToolchainConstraint = String("!=") + installedText;

    PackageTool::LockFile lock;
    SLANG_CHECK(SLANG_FAILED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(error.getUnownedSlice().indexOf(UnownedStringSlice("slang-toolchain")) >= 0);

    root.slangToolchainConstraint = String(">=2027.0.0 || >=") + installedText;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(_findLockedPackage(lock, "noise") != nullptr);
}

SLANG_UNIT_TEST(PackageResolverPinnedRefUsesClaimedVersion)
{
    InMemoryPackageSource source;
    source.addRelease("memory:noise", "1.0.0", _makeManifest("noise"));
    source.addRef("memory:noise", "main", "1.4.0", _makeManifest("noise"));

    Manifest root = _makeManifest("root");
    Dependency dependency;
    dependency.name = "noise";
    dependency.git = "memory:noise";
    dependency.version = ">=1.0.0 <2.0.0";
    dependency.ref = "main";
    dependency.as = "1.4.0";
    root.dependencies.add(dependency);

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    const LockedPackage* noise = _findLockedPackage(lock, "noise");
    SLANG_CHECK_ABORT(noise);
    SLANG_CHECK(noise->ref == "main");
    SLANG_CHECK(noise->version == "1.4.0");
    SLANG_CHECK(noise->commit == "commit-main");
}

SLANG_UNIT_TEST(PackageResolverUsesLatestReleaseRetractions)
{
    InMemoryPackageSource source;
    source.addRelease("memory:noise", "1.0.0", _makeManifest("noise"));
    source.addRelease("memory:noise", "1.1.0", _makeManifest("noise"));
    Manifest latest = _makeManifest("noise");
    Retraction retraction;
    retraction.version = "1.1.0";
    retraction.reason = "Known regression";
    latest.retractions.add(retraction);
    source.addRelease("memory:noise", "1.2.0", latest);

    Manifest root = _makeManifest("root");
    _addDependency(root, "noise", "memory:noise", ">=1.0.0 <1.2.0");

    PackageTool::LockFile lock;
    List<String> warnings;
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(resolveDependenciesWithSource(".", root, source, lock, error, &warnings)));
    const LockedPackage* noise = _findLockedPackage(lock, "noise");
    SLANG_CHECK_ABORT(noise);
    SLANG_CHECK(noise->ref == "v1.0.0");
    SLANG_CHECK(warnings.getCount() == 1);
    SLANG_CHECK(warnings[0].getUnownedSlice().indexOf(UnownedStringSlice("retracts")) >= 0);
}

SLANG_UNIT_TEST(PackageResolveReportFormat)
{
    Manifest root = _makeManifest("video-preview");
    String error;

    LockedPackage previousEncoding;
    previousEncoding.name = "color-encoding";
    previousEncoding.git = "memory:color-encoding";
    previousEncoding.ref = "v1.0.0";
    previousEncoding.version = "1.0.0";
    previousEncoding.commit = "aaa";
    PackageTool::LockFile previous;
    previous.packages.add(previousEncoding);

    LockedPackage nextEncoding;
    nextEncoding.name = "color-encoding";
    nextEncoding.git = "memory:color-encoding";
    nextEncoding.ref = "v1.1.0";
    nextEncoding.version = "1.1.0";
    nextEncoding.commit = "bbb";
    LockedPackage nextConvert;
    nextConvert.name = "color-convert";
    nextConvert.git = "memory:color-convert";
    nextConvert.ref = "v1.1.0";
    nextConvert.version = "1.1.0";
    nextConvert.commit = "ccc";
    PackageTool::LockFile next;
    next.packages.add(nextConvert);
    next.packages.add(nextEncoding);

    ResolveConstraintNote encodingConstraint;
    encodingConstraint.ownerName = "video-preview";
    encodingConstraint.text = ">=1.0.0 <2.0.0";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(parseVersionConstraint(
        encodingConstraint.text.getUnownedSlice(),
        encodingConstraint.constraint,
        error)));
    ResolveSkipNote skip;
    skip.version = "1.0.0";
    skip.reason = "retracted — Truncated Rec. 709 luma weights";
    ResolvePackageExplanation encoding;
    encoding.name = "color-encoding";
    encoding.version = "1.1.0";
    encoding.ref = "v1.1.0";
    encoding.selectionKind = ResolveSelectionKind::HighestRelease;
    encoding.constraints.add(encodingConstraint);
    encoding.skips.add(skip);
    ResolvePackageExplanation convert;
    convert.name = "color-convert";
    convert.version = "1.1.0";
    convert.ref = "v1.1.0";
    convert.selectionKind = ResolveSelectionKind::HighestRelease;
    convert.constraints.add(encodingConstraint);
    convert.constraints[0].text = ">=1.0.0 <2.0.0";
    ResolveReport report;
    report.rootPackageName = "video-preview";
    report.packages.add(convert);
    report.packages.add(encoding);

    String detailed = formatResolveReport(root, &previous, next, report, false, false);
    SLANG_CHECK(
        detailed.getUnownedSlice().indexOf(UnownedStringSlice("Resolving dependencies...")) >= 0);
    SLANG_CHECK(
        detailed.getUnownedSlice().indexOf(
            UnownedStringSlice("upgraded color-encoding 1.0.0 => 1.1.0")) >= 0);
    SLANG_CHECK(
        detailed.getUnownedSlice().indexOf(UnownedStringSlice("skipped 1.0.0: retracted")) >= 0);
    SLANG_CHECK(
        detailed.getUnownedSlice().indexOf(UnownedStringSlice("added color-convert 1.1.0")) >= 0);
    SLANG_CHECK(
        detailed.getUnownedSlice().indexOf(
            UnownedStringSlice("Updated 2 packages: 1 upgraded, 1 added; 0 unchanged.")) >= 0);

    String minimal = formatResolveReport(root, &previous, next, report, false, true);
    SLANG_CHECK(
        minimal.getUnownedSlice().indexOf(UnownedStringSlice("Resolving dependencies...")) < 0);
    SLANG_CHECK(minimal.getUnownedSlice().indexOf(UnownedStringSlice("selected highest")) < 0);
    SLANG_CHECK(
        minimal.getUnownedSlice().indexOf(
            UnownedStringSlice("upgraded color-encoding 1.0.0 => 1.1.0")) >= 0);
    SLANG_CHECK(
        minimal.getUnownedSlice().indexOf(UnownedStringSlice("added color-convert 1.1.0")) >= 0);
}

SLANG_UNIT_TEST(PackageResolverReportRecordsSkips)
{
    InMemoryPackageSource source;
    source.addRelease("memory:noise", "1.0.0", _makeManifest("noise"));
    source.addRelease("memory:noise", "1.1.0", _makeManifest("noise"));

    Manifest root = _makeManifest("root");
    _addDependency(root, "noise", "memory:noise", ">=1.0.0");
    Exclusion exclusion;
    exclusion.packageName = "noise";
    exclusion.version = "1.1.0";
    exclusion.reason = "Workspace regression";
    root.workspace.exclusions.add(exclusion);

    PackageTool::LockFile lock;
    List<String> warnings;
    ResolveReport report;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        resolveDependenciesWithSource(".", root, source, lock, error, &warnings, &report)));
    const LockedPackage* noise = _findLockedPackage(lock, "noise");
    SLANG_CHECK_ABORT(noise);
    SLANG_CHECK(noise->ref == "v1.0.0");
    SLANG_CHECK(report.packages.getCount() == 1);
    SLANG_CHECK(report.packages[0].skips.getCount() == 1);
    SLANG_CHECK(report.packages[0].skips[0].version == "1.1.0");
    SLANG_CHECK(
        report.packages[0].skips[0].reason.getUnownedSlice().indexOf(
            UnownedStringSlice("workspace excludes")) >= 0);
    SLANG_CHECK(report.packages[0].constraints.getCount() == 1);
    SLANG_CHECK(report.packages[0].constraints[0].ownerName == "root");
}

SLANG_UNIT_TEST(PackageResolverAppliesWorkspaceExclusions)
{
    InMemoryPackageSource source;
    source.addRelease("memory:noise", "1.0.0", _makeManifest("noise"));
    source.addRelease("memory:noise", "1.1.0", _makeManifest("noise"));

    Manifest root = _makeManifest("root");
    _addDependency(root, "noise", "memory:noise", ">=1.0.0");
    Exclusion exclusion;
    exclusion.packageName = "noise";
    exclusion.version = "1.1.0";
    exclusion.reason = "Workspace regression";
    root.workspace.exclusions.add(exclusion);

    PackageTool::LockFile lock;
    List<String> warnings;
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(resolveDependenciesWithSource(".", root, source, lock, error, &warnings)));
    const LockedPackage* noise = _findLockedPackage(lock, "noise");
    SLANG_CHECK_ABORT(noise);
    SLANG_CHECK(noise->ref == "v1.0.0");
    SLANG_CHECK(warnings.getCount() == 1);
    SLANG_CHECK(warnings[0].getUnownedSlice().indexOf(UnownedStringSlice("excludes")) >= 0);
}

SLANG_UNIT_TEST(PackageResolverWarnsUnadoptedDependencyExcludes)
{
    InMemoryPackageSource source;
    Manifest display = _makeManifest("display");
    Exclusion exclusion;
    exclusion.packageName = "noise";
    exclusion.version = "1.1.0";
    exclusion.reason = "Broken gradients";
    display.workspace.exclusions.add(exclusion);
    _addDependency(display, "noise", "memory:noise", ">=1.0.0");
    source.addRelease("memory:display", "1.0.0", display);
    source.addRelease("memory:noise", "1.0.0", _makeManifest("noise"));
    source.addRelease("memory:noise", "1.1.0", _makeManifest("noise"));

    Manifest root = _makeManifest("root");
    _addDependency(root, "display", "memory:display", ">=1.0.0");

    PackageTool::LockFile lock;
    List<String> warnings;
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(resolveDependenciesWithSource(".", root, source, lock, error, &warnings)));
    const LockedPackage* noise = _findLockedPackage(lock, "noise");
    SLANG_CHECK_ABORT(noise);
    SLANG_CHECK(noise->ref == "v1.1.0");
    SLANG_CHECK(warnings.getCount() == 1);
    SLANG_CHECK(warnings[0].getUnownedSlice().indexOf(UnownedStringSlice("display")) >= 0);
    SLANG_CHECK(warnings[0].getUnownedSlice().indexOf(UnownedStringSlice("does not exclude")) >= 0);

    root.workspace.exclusions.add(exclusion);
    warnings.clear();
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(resolveDependenciesWithSource(".", root, source, lock, error, &warnings)));
    noise = _findLockedPackage(lock, "noise");
    SLANG_CHECK_ABORT(noise);
    SLANG_CHECK(noise->ref == "v1.0.0");
    for (const auto& warning : warnings)
        SLANG_CHECK(warning.getUnownedSlice().indexOf(UnownedStringSlice("does not exclude")) < 0);

    Manifest wrapper = _makeManifest("wrapper");
    _addDependency(wrapper, "display", "memory:display", ">=1.0.0");
    source.addRelease("memory:wrapper", "1.0.0", wrapper);
    Manifest indirectRoot = _makeManifest("root");
    _addDependency(indirectRoot, "wrapper", "memory:wrapper", ">=1.0.0");
    warnings.clear();
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        resolveDependenciesWithSource(".", indirectRoot, source, lock, error, &warnings)));
    SLANG_CHECK(warnings.getCount() == 1);
    SLANG_CHECK(warnings[0].getUnownedSlice().indexOf(UnownedStringSlice("display")) >= 0);
    SLANG_CHECK(warnings[0].getUnownedSlice().indexOf(UnownedStringSlice("does not exclude")) >= 0);
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
    pPath.as = "1.0.0";
    root.dependencies.add(pPath);
    _addDependency(root, "b", "memory:b", "<1.5.0");

    PackageTool::LockFile lock;
    List<String> warnings;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        resolveDependenciesWithSource(temp.path, root, source, lock, error, &warnings)));
    const LockedPackage* lockedP = _findLockedPackage(lock, "p");
    const LockedPackage* lockedB = _findLockedPackage(lock, "b");
    SLANG_CHECK(lockedP && lockedP->path == "vendor/p");
    SLANG_CHECK(lockedB && lockedB->ref == "v1.4.0");
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
    qPath.as = "1.0.0";
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
    SLANG_CHECK(lockedQ->version == "1.0.0");
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
    SLANG_CHECK(lockedA->ref == "v1.0.0");
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
    SLANG_CHECK(lockedA->ref == "v1.0.0");
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
        error.getUnownedSlice().indexOf(UnownedStringSlice("No package selection satisfies")) >= 0);
}
