// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "core/slang-io.h"
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
                                "  \"version\": \"0.1.0\",\n"
                                "  \"exports\": [\"src\"],\n"
                                "  \"license_files\": [\"LICENSE\"],\n"
                                "  \"dependencies\": {\n"
                                "    \"noise\": {\n"
                                "      \"git\": \"https://example.com/noise.git\",\n"
                                "      \"version\": \">=1.2.0 <2.0.0\"\n"
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
    SLANG_CHECK(manifest.licenseFiles.getCount() == 1);
    SLANG_CHECK(manifest.licenseFiles[0] == "LICENSE");
    SLANG_CHECK(manifest.dependencies.getCount() == 1);
    SLANG_CHECK(manifest.dependencies[0].name == "noise");
    SLANG_CHECK(manifest.dependencies[0].version == ">=1.2.0 <2.0.0");
    SLANG_CHECK(manifest.dependencies[0].tag.getLength() == 0);

    const String taggedText = "{\n"
                              "  \"name\": \"root\",\n"
                              "  \"version\": \"0.1.0\",\n"
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
        "{\"name\":\"root\",\"version\":\"0.1.0\",\"exports\":[\"src\"],"
        "\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{\"bad\":{\"git\":\"ext::sh -c bad\",\"version\":\"1.0.0\"}}}";
    SLANG_CHECK(SLANG_FAILED(readManifestText("unsafe-git.json", unsafeGitText, manifest, error)));

    const String unsafeExportText =
        "{\"name\":\"root\",\"version\":\"0.1.0\",\"exports\":[\"src\\n/etc\"],"
        "\"license_files\":[\"LICENSE\"],"
        "\"dependencies\":{}}";
    SLANG_CHECK(
        SLANG_FAILED(readManifestText("unsafe-export.json", unsafeExportText, manifest, error)));

    const String missingLicenseFilesText =
        "{\"name\":\"root\",\"version\":\"0.1.0\",\"exports\":[\"src\"],\"dependencies\":{}}";
    SLANG_CHECK(SLANG_FAILED(
        readManifestText("missing-license-files.json", missingLicenseFilesText, manifest, error)));
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
    SLANG_CHECK(File::exists(Path::combine(temp.path, "LICENSE")));

    Manifest manifest;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        readManifest(Path::combine(temp.path, "slang-package.json"), manifest, error)));
    SLANG_CHECK(manifest.name == Path::getFileName(temp.path));
    SLANG_CHECK(manifest.licenseFiles.getCount() == 1);
    SLANG_CHECK(manifest.licenseFiles[0] == "LICENSE");

    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));

    const String invalidRoot = Path::combine(temp.path, "invalid package");
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(invalidRoot));
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(invalidRoot, SLANG_COUNT_OF(initArguments), initArguments, error)));
    SLANG_CHECK(!File::exists(Path::combine(invalidRoot, "slang-package.json")));
}

SLANG_UNIT_TEST(PackageToolLockedFetchUsesJSONLockName)
{
    TemporaryDirectory temp;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_makeTemporaryDirectory(temp)));

    const char* initArguments[] = {"slang-package", "init"};
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(initArguments), initArguments, error)));

    const char* fetchArguments[] = {"slang-package", "fetch", "--locked"};
    SLANG_CHECK(SLANG_FAILED(
        executeInDirectory(temp.path, SLANG_COUNT_OF(fetchArguments), fetchArguments, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("slang-package-lock.json")) >= 0);
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

    String packageRoot = Path::combine(Path::combine(temp.path, ".slang", "packages"), "b");
    Manifest package;
    package.name = "b";
    package.version = "1.0.0";
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
    TagCandidate candidate;
    Manifest manifest;
};

class InMemoryPackageSource : public IPackageResolverSource
{
public:
    List<InMemoryRelease> releases;

    void addRelease(const String& git, const Manifest& manifest)
    {
        InMemoryRelease release;
        release.git = git;
        release.manifest = manifest;
        release.candidate.tag = String("v") + manifest.version;
        release.candidate.commit = release.candidate.tag;
        SLANG_RELEASE_ASSERT(SLANG_SUCCEEDED(
            SemanticVersion::parse(manifest.version.getUnownedSlice(), release.candidate.version)));
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
        Manifest& outManifest,
        String& outError) override
    {
        for (const auto& release : releases)
        {
            if (release.git == git && release.candidate.tag == candidate.tag)
            {
                outManifest = release.manifest;
                return SLANG_OK;
            }
        }
        outError = String("Missing in-memory manifest for ") + git + "@" + candidate.tag;
        return SLANG_FAIL;
    }
};

static Manifest _makeManifest(const char* name, const char* version)
{
    Manifest manifest;
    manifest.name = name;
    manifest.version = version;
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
    source.addRelease("memory:b", _makeManifest("b", "1.2.0"));
    source.addRelease("memory:b", _makeManifest("b", "1.4.0"));

    Manifest a1 = _makeManifest("a", "1.0.0");
    _addDependency(a1, "b", "memory:b", ">=1.2.0");
    source.addRelease("memory:a", a1);
    Manifest a2 = _makeManifest("a", "2.0.0");
    _addDependency(a2, "b", "memory:b", ">=9.0.0");
    source.addRelease("memory:a", a2);

    Manifest root = _makeManifest("root", "0.1.0");
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

SLANG_UNIT_TEST(PackageResolverCompatibleSelfCycle)
{
    InMemoryPackageSource source;
    Manifest a = _makeManifest("a", "1.0.0");
    _addDependency(a, "a", "memory:a", ">=1.0.0");
    source.addRelease("memory:a", a);
    Manifest root = _makeManifest("root", "0.1.0");
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
    Manifest a = _makeManifest("a", "1.0.0");
    _addDependency(a, "b", "memory:b", ">=1.0.0");
    source.addRelease("memory:a", a);
    Manifest b = _makeManifest("b", "1.0.0");
    _addDependency(b, "a", "memory:a", ">=1.0.0");
    source.addRelease("memory:b", b);
    Manifest root = _makeManifest("root", "0.1.0");
    _addDependency(root, "a", "memory:a", ">=1.0.0");

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(lock.packages.getCount() == 2);
}

SLANG_UNIT_TEST(PackageResolverCycleBacktracksEarlierSelection)
{
    InMemoryPackageSource source;
    source.addRelease("memory:a", _makeManifest("a", "1.0.0"));
    source.addRelease("memory:a", _makeManifest("a", "2.0.0"));
    Manifest b = _makeManifest("b", "1.0.0");
    _addDependency(b, "a", "memory:a", "<2.0.0");
    source.addRelease("memory:b", b);
    Manifest root = _makeManifest("root", "0.1.0");
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
    Manifest a = _makeManifest("a", "1.0.0");
    _addDependency(a, "b", "memory:b", ">=1.0.0");
    source.addRelease("memory:a", a);
    Manifest b = _makeManifest("b", "1.0.0");
    _addDependency(b, "a", "memory:a", ">=2.0.0");
    source.addRelease("memory:b", b);
    Manifest root = _makeManifest("root", "0.1.0");
    _addDependency(root, "a", "memory:a", ">=1.0.0");

    PackageTool::LockFile lock;
    String error;
    SLANG_CHECK(SLANG_FAILED(resolveDependenciesWithSource(root, source, lock, error)));
    SLANG_CHECK(
        error.getUnownedSlice().indexOf(UnownedStringSlice("No release tag satisfies")) >= 0);
}
