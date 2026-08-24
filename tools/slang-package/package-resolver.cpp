// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-resolver.h"

#include "core/slang-io.h"
#include "package-git.h"
#include "package-json.h"
#include "package-local.h"

namespace Slang
{
namespace PackageTool
{

struct ResolutionPackage
{
    String name;
    String git;
    List<VersionConstraint> constraints;
    bool selected = false;
    LockedPackage locked;
};

class GitPackageResolverSource : public IPackageResolverSource
{
public:
    String projectRoot;
    String cacheRoot;

    SlangResult initialize(String& outError)
    {
        cacheRoot = Path::combine(projectRoot, ".slang", "cache");
        if (!Path::createDirectoryRecursive(cacheRoot))
        {
            outError = String("Cannot create package cache directory: ") + cacheRoot;
            return SLANG_FAIL;
        }
        return SLANG_OK;
    }

    virtual SlangResult listReleaseTags(
        const String&,
        const String& git,
        List<TagCandidate>& outCandidates,
        String& outError) override
    {
        return PackageTool::listReleaseTags(git, outCandidates, outError);
    }

    virtual SlangResult loadManifest(
        const String& packageName,
        const String& git,
        const TagCandidate& candidate,
        Manifest& outManifest,
        String& outError) override
    {
        String repositoryPath = Path::combine(cacheRoot, packageName);
        SLANG_RETURN_ON_FAIL(ensureRepository(projectRoot, git, repositoryPath, outError));

        String manifestText;
        SLANG_RETURN_ON_FAIL(readFileAtRevision(
            repositoryPath,
            candidate.commit,
            "slang-package.json",
            manifestText,
            outError));
        String sourceName = git + "@" + candidate.tag + ":slang-package.json";
        return readManifestText(sourceName, manifestText, outManifest, outError);
    }
};

class LocalPackageResolverSource : public IPackageResolverSource
{
public:
    String projectRoot;
    const List<LocalPackage>* localPackages = nullptr;
    GitPackageResolverSource gitSource;

    SlangResult initialize(String& outError)
    {
        gitSource.projectRoot = projectRoot;
        return gitSource.initialize(outError);
    }

    virtual SlangResult listReleaseTags(
        const String& packageName,
        const String& git,
        List<TagCandidate>& outCandidates,
        String& outError) override
    {
        Index localIndex = findLocalPackageIndex(*localPackages, packageName);
        if (localIndex < 0)
            return gitSource.listReleaseTags(packageName, git, outCandidates, outError);

        Manifest manifest;
        SLANG_RETURN_ON_FAIL(readLocalPackageManifest(
            projectRoot,
            (*localPackages)[localIndex],
            manifest,
            outError));
        TagCandidate candidate;
        if (SLANG_FAILED(
                SemanticVersion::parse(manifest.version.getUnownedSlice(), candidate.version)))
        {
            outError = String("Registered local package has an invalid version: ") + packageName;
            return SLANG_FAIL;
        }
        candidate.tag = String("v") + manifest.version;
        candidate.path = (*localPackages)[localIndex].path;
        outCandidates.clear();
        outCandidates.add(candidate);
        return SLANG_OK;
    }

    virtual SlangResult loadManifest(
        const String& packageName,
        const String& git,
        const TagCandidate& candidate,
        Manifest& outManifest,
        String& outError) override
    {
        if (!candidate.path.getLength())
            return gitSource.loadManifest(packageName, git, candidate, outManifest, outError);

        Index localIndex = findLocalPackageIndex(*localPackages, packageName);
        if (localIndex < 0 || (*localPackages)[localIndex].path != candidate.path)
        {
            outError =
                String("Local package registration changed during resolution: ") + packageName;
            return SLANG_FAIL;
        }
        return readLocalPackageManifest(
            projectRoot,
            (*localPackages)[localIndex],
            outManifest,
            outError);
    }
};

class Resolver
{
public:
    static const Index kMaxPackageCount = 256;
    static const Index kMaxCandidateAttempts = 4096;

    IPackageResolverSource* source = nullptr;
    List<ResolutionPackage> packages;
    Index candidateAttemptCount = 0;

    SlangResult resolve(const Manifest& rootManifest, LockFile& outLock, String& outError)
    {
        for (const auto& dependency : rootManifest.dependencies)
            SLANG_RETURN_ON_FAIL(addDependency(dependency, outError));

        SLANG_RETURN_ON_FAIL(search(outError));
        outLock = LockFile();
        for (const auto& package : packages)
        {
            SLANG_ASSERT(package.selected);
            outLock.packages.add(package.locked);
        }
        outLock.packages.sort([](const LockedPackage& left, const LockedPackage& right)
                              { return left.name < right.name; });
        return SLANG_OK;
    }

private:
    Index findPackage(const String& name) const
    {
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (packages[i].name == name)
                return i;
        }
        return -1;
    }

    bool matchesAll(const ResolutionPackage& package, const SemanticVersion& version) const
    {
        for (const auto& constraint : package.constraints)
        {
            if (!constraint.matches(version))
                return false;
        }
        return true;
    }

    SlangResult addDependency(const Dependency& dependency, String& outError)
    {
        VersionConstraint constraint;
        SLANG_RETURN_ON_FAIL(parseDependencyConstraint(dependency, constraint, outError));

        Index index = findPackage(dependency.name);
        if (index < 0)
        {
            if (packages.getCount() >= kMaxPackageCount)
            {
                outError = "Dependency graph exceeds the package limit.";
                return SLANG_FAIL;
            }
            ResolutionPackage package;
            package.name = dependency.name;
            package.git = dependency.git;
            package.constraints.add(constraint);
            packages.add(package);
            return SLANG_OK;
        }

        ResolutionPackage& package = packages[index];
        if (package.git != dependency.git)
        {
            outError =
                String("Package '") + dependency.name + "' is required from more than one Git URL.";
            return SLANG_FAIL;
        }
        package.constraints.add(constraint);
        if (package.selected)
        {
            SemanticVersion selectedVersion;
            SlangResult versionResult = package.locked.path.getLength()
                                            ? SemanticVersion::parse(
                                                  package.locked.version.getUnownedSlice(),
                                                  selectedVersion)
                                            : parseReleaseTag(package.locked.tag, selectedVersion);
            if (SLANG_FAILED(versionResult))
            {
                outError = String("Selected package has an invalid version: ") + package.name;
                return SLANG_FAIL;
            }
            if (!constraint.matches(selectedVersion))
            {
                outError = String("Selected version of package '") + dependency.name +
                           "' conflicts with a transitive constraint.";
                return SLANG_FAIL;
            }
        }
        return SLANG_OK;
    }

    SlangResult loadCandidateManifest(
        const ResolutionPackage& package,
        const TagCandidate& candidate,
        Manifest& outManifest,
        String& outError)
    {
        SLANG_RETURN_ON_FAIL(
            source->loadManifest(package.name, package.git, candidate, outManifest, outError));
        if (outManifest.name != package.name)
        {
            outError = String("Package name '") + outManifest.name +
                       "' does not match dependency name '" + package.name + "'.";
            return SLANG_FAIL;
        }
        SemanticVersion manifestVersion;
        if (SLANG_FAILED(
                SemanticVersion::parse(outManifest.version.getUnownedSlice(), manifestVersion)))
        {
            outError = String("Manifest for package '") + package.name +
                       "' has an invalid semantic version: " + outManifest.version;
            return SLANG_FAIL;
        }
        if (manifestVersion != candidate.version)
        {
            outError = String("Manifest version for package '") + package.name +
                       "' does not match tag " + candidate.tag + ".";
            return SLANG_FAIL;
        }
        return SLANG_OK;
    }

    SlangResult search(String& outError)
    {
        Index unresolvedIndex = -1;
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (!packages[i].selected)
            {
                unresolvedIndex = i;
                break;
            }
        }
        if (unresolvedIndex < 0)
            return SLANG_OK;

        ResolutionPackage unresolved = packages[unresolvedIndex];
        List<TagCandidate> candidates;
        SLANG_RETURN_ON_FAIL(
            source->listReleaseTags(unresolved.name, unresolved.git, candidates, outError));
        String lastCandidateError;
        for (const auto& candidate : candidates)
        {
            if (!matchesAll(unresolved, candidate.version))
                continue;
            if (++candidateAttemptCount > kMaxCandidateAttempts)
            {
                outError = "Dependency resolution exceeds the candidate-attempt limit.";
                return SLANG_FAIL;
            }

            List<ResolutionPackage> snapshot = packages;
            Manifest manifest;
            String candidateError;
            if (SLANG_FAILED(
                    loadCandidateManifest(unresolved, candidate, manifest, candidateError)))
            {
                lastCandidateError = candidateError;
                packages = snapshot;
                continue;
            }

            ResolutionPackage& selected = packages[unresolvedIndex];
            selected.selected = true;
            selected.locked.name = selected.name;
            selected.locked.git = selected.git;
            if (candidate.path.getLength())
            {
                selected.locked.version = manifest.version;
                selected.locked.path = candidate.path;
            }
            else
            {
                selected.locked.tag = candidate.tag;
                selected.locked.commit = candidate.commit;
            }
            selected.locked.exports = manifest.exports;
            selected.locked.dependencies = manifest.dependencies;

            bool dependencyConflict = false;
            for (const auto& dependency : manifest.dependencies)
            {
                if (SLANG_FAILED(addDependency(dependency, candidateError)))
                {
                    dependencyConflict = true;
                    break;
                }
            }
            if (!dependencyConflict && SLANG_SUCCEEDED(search(candidateError)))
                return SLANG_OK;

            lastCandidateError = candidateError;
            packages = snapshot;
        }

        outError = String("No release tag satisfies all constraints for package '") +
                   unresolved.name + "'.";
        if (lastCandidateError.getLength() != 0)
            outError = outError + " Last candidate failed because: " + lastCandidateError;
        return SLANG_FAIL;
    }
};

SlangResult resolveDependenciesWithSource(
    const Manifest& manifest,
    IPackageResolverSource& source,
    LockFile& outLock,
    String& outError)
{
    Resolver resolver;
    resolver.source = &source;
    return resolver.resolve(manifest, outLock, outError);
}

SlangResult resolveDependencies(
    const String& projectRoot,
    const Manifest& manifest,
    LockFile& outLock,
    String& outError)
{
    GitPackageResolverSource source;
    source.projectRoot = projectRoot;
    SLANG_RETURN_ON_FAIL(source.initialize(outError));
    return resolveDependenciesWithSource(manifest, source, outLock, outError);
}

SlangResult resolveDependenciesFromLocalPackages(
    const String& projectRoot,
    const Manifest& manifest,
    const List<LocalPackage>& localPackages,
    LockFile& outLock,
    String& outError)
{
    LocalPackageResolverSource source;
    source.projectRoot = projectRoot;
    source.localPackages = &localPackages;
    SLANG_RETURN_ON_FAIL(source.initialize(outError));
    return resolveDependenciesWithSource(manifest, source, outLock, outError);
}

} // namespace PackageTool
} // namespace Slang
