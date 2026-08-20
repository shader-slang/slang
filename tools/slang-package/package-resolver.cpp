// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-resolver.h"

#include "core/slang-io.h"
#include "package-git.h"
#include "package-json.h"

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

class Resolver
{
public:
    static const Index kMaxPackageCount = 256;
    static const Index kMaxResolutionDepth = 64;
    static const Index kMaxCandidateAttempts = 4096;

    String projectRoot;
    String cacheRoot;
    List<ResolutionPackage> packages;
    Index candidateAttemptCount = 0;

    SlangResult resolve(const Manifest& rootManifest, LockFile& outLock, String& outError)
    {
        cacheRoot = Path::combine(projectRoot, ".slang", "cache");
        if (!Path::createDirectoryRecursive(cacheRoot))
        {
            outError = String("Cannot create package cache directory: ") + cacheRoot;
            return SLANG_FAIL;
        }
        for (const auto& dependency : rootManifest.dependencies)
            SLANG_RETURN_ON_FAIL(addDependency(dependency, outError));

        SLANG_RETURN_ON_FAIL(search(0, outError));
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
            if (SLANG_FAILED(parseReleaseTag(package.locked.tag, selectedVersion)))
            {
                outError =
                    String("Selected package has an invalid release tag: ") + package.locked.tag;
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
        String repositoryPath = Path::combine(cacheRoot, package.name);
        SLANG_RETURN_ON_FAIL(ensureRepository(projectRoot, package.git, repositoryPath, outError));

        String manifestText;
        SLANG_RETURN_ON_FAIL(readFileAtRevision(
            repositoryPath,
            candidate.commit,
            "slang-package.json",
            manifestText,
            outError));
        String sourceName = package.git + "@" + candidate.tag + ":slang-package.json";
        SLANG_RETURN_ON_FAIL(readManifestText(sourceName, manifestText, outManifest, outError));
        if (outManifest.name != package.name)
        {
            outError = String("Package name '") + outManifest.name +
                       "' does not match dependency name '" + package.name + "'.";
            return SLANG_FAIL;
        }
        SemanticVersion manifestVersion;
        SLANG_RETURN_ON_FAIL(
            SemanticVersion::parse(outManifest.version.getUnownedSlice(), manifestVersion));
        if (manifestVersion != candidate.version)
        {
            outError = String("Manifest version for package '") + package.name +
                       "' does not match tag " + candidate.tag + ".";
            return SLANG_FAIL;
        }
        return SLANG_OK;
    }

    SlangResult search(Index depth, String& outError)
    {
        if (depth > kMaxResolutionDepth)
        {
            outError = "Dependency graph exceeds the resolution depth limit.";
            return SLANG_FAIL;
        }
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
        SLANG_RETURN_ON_FAIL(listReleaseTags(unresolved.git, candidates, outError));
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
            selected.locked.tag = candidate.tag;
            selected.locked.commit = candidate.commit;
            selected.locked.exports = manifest.exports;

            bool dependencyConflict = false;
            for (const auto& dependency : manifest.dependencies)
            {
                if (SLANG_FAILED(addDependency(dependency, candidateError)))
                {
                    dependencyConflict = true;
                    break;
                }
            }
            if (!dependencyConflict && SLANG_SUCCEEDED(search(depth + 1, candidateError)))
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

SlangResult resolveDependencies(
    const String& projectRoot,
    const Manifest& manifest,
    LockFile& outLock,
    String& outError)
{
    Resolver resolver;
    resolver.projectRoot = projectRoot;
    return resolver.resolve(manifest, outLock, outError);
}

} // namespace PackageTool
} // namespace Slang
