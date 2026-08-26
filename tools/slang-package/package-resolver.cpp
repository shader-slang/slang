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

struct GitRequirement
{
    String owner;
    String git;
    VersionConstraint constraint;
};

struct ResolutionPackage
{
    String name;
    String git;
    String canonicalPath;
    String pathOwner;
    List<GitRequirement> gitRequirements;
    bool selected = false;
    LockedPackage locked;
    ResolvedManifest resolvedManifest;
};

static bool _isPathWithin(const String& root, const String& path)
{
    String relative = Path::getRelativePath(root, path);
    if (Path::isAbsolute(relative))
        return false;
    List<UnownedStringSlice> components;
    Path::split(relative.getUnownedSlice(), components);
    return components.getCount() == 0 || components[0] != "..";
}

static bool _pathStartsWithParent(const String& path)
{
    List<UnownedStringSlice> components;
    Path::split(path.getUnownedSlice(), components);
    return components.getCount() != 0 && components[0] == "..";
}

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
        ResolvedManifest& outManifest,
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
        SLANG_RETURN_ON_FAIL(
            readManifestText(sourceName, manifestText, outManifest.manifest, outError));
        outManifest.ownerKey = String("git:") + packageName + "@" + candidate.commit;
        outManifest.lockRoot = Path::combine(".slang", "packages", packageName);
        outManifest.gitRepositoryPath = repositoryPath;
        outManifest.gitRevision = candidate.commit;
        return SLANG_OK;
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

        TagCandidate candidate;
        candidate.path = (*localPackages)[localIndex].path;
        outCandidates.clear();
        outCandidates.add(candidate);
        return SLANG_OK;
    }

    virtual SlangResult loadManifest(
        const String& packageName,
        const String& git,
        const TagCandidate& candidate,
        ResolvedManifest& outManifest,
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
        SLANG_RETURN_ON_FAIL(readLocalPackageManifest(
            projectRoot,
            (*localPackages)[localIndex],
            outManifest.manifest,
            outError));
        SLANG_RETURN_ON_FAIL(getLocalPackageRoot(
            projectRoot,
            (*localPackages)[localIndex],
            outManifest.sourceRoot,
            outError));
        outManifest.ownerKey = String("local:") + packageName + ":" + candidate.path;
        outManifest.lockRoot = (*localPackages)[localIndex].path;
        return SLANG_OK;
    }
};

class Resolver
{
public:
    static const Index kMaxPackageCount = 256;
    static const Index kMaxCandidateAttempts = 4096;

    IPackageResolverSource* source = nullptr;
    String projectRoot;
    List<String>* warnings = nullptr;
    const Manifest* rootManifest = nullptr;
    List<ResolutionPackage> packages;
    Index candidateAttemptCount = 0;

    SlangResult resolve(const Manifest& rootManifest, LockFile& outLock, String& outError)
    {
        this->rootManifest = &rootManifest;
        ResolvedManifest root;
        root.manifest = rootManifest;
        root.ownerKey = "<root>";
        root.sourceRoot = projectRoot;
        root.lockRoot = ".";
        for (const auto& dependency : rootManifest.dependencies)
            SLANG_RETURN_ON_FAIL(addDependency(dependency, root, outError));

        SLANG_RETURN_ON_FAIL(search(outError));
        outLock = LockFile();
        List<bool> reachable;
        reachable.setCount(packages.getCount());
        for (auto& value : reachable)
            value = false;
        List<Index> pending;
        for (const auto& dependency : rootManifest.dependencies)
        {
            Index index = findPackage(dependency.name);
            SLANG_RELEASE_ASSERT(index >= 0);
            if (!reachable[index])
            {
                reachable[index] = true;
                pending.add(index);
            }
        }
        for (Index pendingIndex = 0; pendingIndex < pending.getCount(); ++pendingIndex)
        {
            const auto& package = packages[pending[pendingIndex]];
            SLANG_RELEASE_ASSERT(package.selected);
            for (const auto& dependency : package.locked.dependencies)
            {
                Index index = findPackage(dependency.name);
                SLANG_RELEASE_ASSERT(index >= 0);
                if (!reachable[index])
                {
                    reachable[index] = true;
                    pending.add(index);
                }
            }
        }
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (!reachable[i])
                continue;
            const auto& package = packages[i];
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
        for (const auto& requirement : package.gitRequirements)
        {
            if (!requirement.constraint.matches(version))
                return false;
        }
        return true;
    }

    void removeGitRequirementsFromOwner(const String& owner)
    {
        for (auto& package : packages)
        {
            String oldGit = package.git;
            for (Index i = package.gitRequirements.getCount() - 1; i >= 0; --i)
            {
                if (package.gitRequirements[i].owner == owner)
                    package.gitRequirements.removeAt(i);
            }
            package.git =
                package.gitRequirements.getCount() ? package.gitRequirements[0].git : String();
            if (!package.canonicalPath.getLength() && package.selected && oldGit != package.git)
            {
                package.selected = false;
                package.locked = LockedPackage();
                package.resolvedManifest = ResolvedManifest();
            }
        }
    }

    bool isOwnerActive(const String& owner, const List<bool>& reachable) const
    {
        if (owner == "<root>")
            return true;
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (reachable[i] && packages[i].selected &&
                packages[i].resolvedManifest.ownerKey == owner)
                return true;
        }
        return false;
    }

    void pruneUnreachableGitRequirements()
    {
        for (;;)
        {
            List<bool> reachable;
            getReachablePackages(reachable);
            bool changed = false;
            for (auto& package : packages)
            {
                if (package.canonicalPath.getLength() &&
                    !isOwnerActive(package.pathOwner, reachable))
                {
                    String removedOwner = package.resolvedManifest.ownerKey;
                    removeGitRequirementsFromOwner(removedOwner);
                    package.canonicalPath = String();
                    package.pathOwner = String();
                    package.selected = false;
                    package.locked = LockedPackage();
                    package.resolvedManifest = ResolvedManifest();
                    changed = true;
                }
                String oldGit = package.git;
                for (Index i = package.gitRequirements.getCount() - 1; i >= 0; --i)
                {
                    const String& owner = package.gitRequirements[i].owner;
                    if (!isOwnerActive(owner, reachable))
                    {
                        package.gitRequirements.removeAt(i);
                        changed = true;
                    }
                }
                package.git =
                    package.gitRequirements.getCount() ? package.gitRequirements[0].git : String();
                if (!package.canonicalPath.getLength() && package.selected && oldGit != package.git)
                {
                    package.selected = false;
                    package.locked = LockedPackage();
                    package.resolvedManifest = ResolvedManifest();
                    changed = true;
                }
            }
            if (!changed)
                return;
        }
    }

    void getReachablePackages(List<bool>& outReachable) const
    {
        outReachable.setCount(packages.getCount());
        for (auto& value : outReachable)
            value = false;
        List<Index> pending;
        for (const auto& dependency : rootManifest->dependencies)
        {
            Index index = findPackage(dependency.name);
            if (index >= 0 && !outReachable[index])
            {
                outReachable[index] = true;
                pending.add(index);
            }
        }
        for (Index pendingIndex = 0; pendingIndex < pending.getCount(); ++pendingIndex)
        {
            const auto& package = packages[pending[pendingIndex]];
            if (!package.selected)
                continue;
            for (const auto& dependency : package.locked.dependencies)
            {
                Index index = findPackage(dependency.name);
                if (index >= 0 && !outReachable[index])
                {
                    outReachable[index] = true;
                    pending.add(index);
                }
            }
        }
    }

    void addWarning(const String& warning)
    {
        if (!warnings || warnings->contains(warning))
            return;
        warnings->add(warning);
    }

    SlangResult loadPathManifest(
        const Dependency& dependency,
        const ResolvedManifest& declaringManifest,
        String& outCanonicalPath,
        ResolvedManifest& outManifest,
        String& outError)
    {
        if (declaringManifest.gitRevision.getLength())
        {
            String gitRelativeRoot =
                Path::simplify(Path::combine(declaringManifest.gitRelativeRoot, dependency.path));
            if (Path::isAbsolute(gitRelativeRoot) || _pathStartsWithParent(gitRelativeRoot))
            {
                outError = String("Path dependency '") + dependency.name +
                           "' escapes its Git package checkout: " + dependency.path;
                return SLANG_FAIL;
            }
            String manifestPath = gitRelativeRoot.getLength()
                                      ? Path::combine(gitRelativeRoot, "slang-package.json")
                                      : "slang-package.json";
            String manifestText;
            SLANG_RETURN_ON_FAIL(readFileAtRevision(
                declaringManifest.gitRepositoryPath,
                declaringManifest.gitRevision,
                manifestPath,
                manifestText,
                outError));
            String sourceName = declaringManifest.gitRepositoryPath + "@" +
                                declaringManifest.gitRevision + ":" + manifestPath;
            SLANG_RETURN_ON_FAIL(
                readManifestText(sourceName, manifestText, outManifest.manifest, outError));
            outCanonicalPath = declaringManifest.gitRepositoryPath + "@" +
                               declaringManifest.gitRevision + ":" + gitRelativeRoot;
            outManifest.lockRoot =
                Path::simplify(Path::combine(declaringManifest.lockRoot, dependency.path));
            outManifest.gitRepositoryPath = declaringManifest.gitRepositoryPath;
            outManifest.gitRevision = declaringManifest.gitRevision;
            outManifest.gitRelativeRoot = gitRelativeRoot;
        }
        else
        {
            String sourcePath = Path::combine(declaringManifest.sourceRoot, dependency.path);
            SlangPathType type;
            if (SLANG_FAILED(Path::getPathType(sourcePath, &type)) ||
                type != SLANG_PATH_TYPE_DIRECTORY ||
                SLANG_FAILED(Path::getCanonical(sourcePath, outCanonicalPath)))
            {
                outError = String("Path dependency directory does not exist: ") + dependency.name +
                           " (" + dependency.path + ")";
                return SLANG_FAIL;
            }
            String canonicalStateRoot;
            String canonicalDeclaringRoot;
            if (SLANG_SUCCEEDED(
                    Path::getCanonical(Path::combine(projectRoot, ".slang"), canonicalStateRoot)) &&
                SLANG_SUCCEEDED(
                    Path::getCanonical(declaringManifest.sourceRoot, canonicalDeclaringRoot)) &&
                _isPathWithin(canonicalStateRoot, outCanonicalPath) &&
                !(_isPathWithin(canonicalStateRoot, canonicalDeclaringRoot) &&
                  _isPathWithin(canonicalDeclaringRoot, outCanonicalPath)))
            {
                outError = String("Path dependency cannot use package-tool state under .slang: ") +
                           dependency.name;
                return SLANG_FAIL;
            }
            SLANG_RETURN_ON_FAIL(readManifest(
                Path::combine(outCanonicalPath, "slang-package.json"),
                outManifest.manifest,
                outError));
            outManifest.sourceRoot = outCanonicalPath;
            if (_isPathWithin(declaringManifest.sourceRoot, outCanonicalPath))
            {
                String relative =
                    Path::getRelativePath(declaringManifest.sourceRoot, outCanonicalPath);
                outManifest.lockRoot =
                    Path::simplify(Path::combine(declaringManifest.lockRoot, relative));
            }
            else
            {
                outManifest.lockRoot = Path::getRelativePath(projectRoot, outCanonicalPath);
                addWarning(
                    String("Path dependency '") + dependency.name + "' escapes package '" +
                    declaringManifest.manifest.name + "': " + dependency.path);
            }
            if (Path::isAbsolute(outManifest.lockRoot))
            {
                outError = String("Path dependency must be on the same filesystem as the app: ") +
                           dependency.name;
                return SLANG_FAIL;
            }
        }
        if (outManifest.manifest.name != dependency.name)
        {
            outError = String("Package name '") + outManifest.manifest.name +
                       "' does not match dependency name '" + dependency.name + "'.";
            return SLANG_FAIL;
        }
        outManifest.ownerKey = String("path:") + outCanonicalPath;
        return SLANG_OK;
    }

    SlangResult selectPathDependency(
        ResolutionPackage& package,
        const Dependency& dependency,
        const ResolvedManifest& declaringManifest,
        String& outError)
    {
        String canonicalPath;
        ResolvedManifest pathManifest;
        SLANG_RETURN_ON_FAIL(
            loadPathManifest(dependency, declaringManifest, canonicalPath, pathManifest, outError));
        if (package.canonicalPath.getLength() && package.canonicalPath != canonicalPath)
        {
            outError =
                String("Package '") + dependency.name + "' is required from more than one path.";
            return SLANG_FAIL;
        }
        if (package.canonicalPath == canonicalPath && package.locked.path.getLength())
            return SLANG_OK;
        if (package.git.getLength())
        {
            addWarning(
                String("Path dependency '") + dependency.name + "' shadows a Git dependency from " +
                package.git + ".");
        }
        if (package.selected)
        {
            String removedOwner = package.resolvedManifest.ownerKey;
            removeGitRequirementsFromOwner(removedOwner);
        }
        package.canonicalPath = canonicalPath;
        package.pathOwner = declaringManifest.ownerKey;
        package.selected = true;
        package.resolvedManifest = pathManifest;
        package.locked = LockedPackage();
        package.locked.name = package.name;
        package.locked.path = pathManifest.lockRoot;
        package.locked.exports = pathManifest.manifest.exports;
        package.locked.dependencies = pathManifest.manifest.dependencies;
        pruneUnreachableGitRequirements();
        for (const auto& child : pathManifest.manifest.dependencies)
            SLANG_RETURN_ON_FAIL(addDependency(child, pathManifest, outError));
        return SLANG_OK;
    }

    SlangResult addDependency(
        const Dependency& dependency,
        const ResolvedManifest& declaringManifest,
        String& outError)
    {
        Index index = findPackage(dependency.name);
        if (dependency.path.getLength())
        {
            if (index < 0)
            {
                if (packages.getCount() >= kMaxPackageCount)
                {
                    outError = "Dependency graph exceeds the package limit.";
                    return SLANG_FAIL;
                }
                ResolutionPackage package;
                package.name = dependency.name;
                packages.add(package);
                index = packages.getCount() - 1;
            }
            return selectPathDependency(packages[index], dependency, declaringManifest, outError);
        }

        VersionConstraint constraint;
        SLANG_RETURN_ON_FAIL(parseDependencyConstraint(dependency, constraint, outError));

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
            GitRequirement requirement;
            requirement.owner = declaringManifest.ownerKey;
            requirement.git = dependency.git;
            requirement.constraint = constraint;
            package.gitRequirements.add(requirement);
            packages.add(package);
            return SLANG_OK;
        }

        ResolutionPackage& package = packages[index];
        if (package.canonicalPath.getLength())
        {
            addWarning(
                String("Path dependency '") + dependency.name + "' shadows a Git dependency from " +
                dependency.git + ".");
            return SLANG_OK;
        }
        if (package.git.getLength() && package.git != dependency.git)
        {
            outError =
                String("Package '") + dependency.name + "' is required from more than one Git URL.";
            return SLANG_FAIL;
        }
        package.git = dependency.git;
        GitRequirement requirement;
        requirement.owner = declaringManifest.ownerKey;
        requirement.git = dependency.git;
        requirement.constraint = constraint;
        package.gitRequirements.add(requirement);
        if (package.selected)
        {
            if (package.locked.path.getLength())
                return SLANG_OK;
            SemanticVersion selectedVersion;
            SlangResult versionResult = parseReleaseTag(package.locked.tag, selectedVersion);
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
        ResolvedManifest& outManifest,
        String& outError)
    {
        SLANG_RETURN_ON_FAIL(
            source->loadManifest(package.name, package.git, candidate, outManifest, outError));
        if (outManifest.manifest.name != package.name)
        {
            outError = String("Package name '") + outManifest.manifest.name +
                       "' does not match dependency name '" + package.name + "'.";
            return SLANG_FAIL;
        }
        if (!outManifest.ownerKey.getLength())
            outManifest.ownerKey = String("candidate:") + package.name + "@" + candidate.tag;
        return SLANG_OK;
    }

    SlangResult search(String& outError)
    {
        List<bool> reachable;
        getReachablePackages(reachable);
        Index unresolvedIndex = -1;
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (reachable[i] && !packages[i].selected)
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
            if (!candidate.path.getLength() && !matchesAll(unresolved, candidate.version))
                continue;
            if (++candidateAttemptCount > kMaxCandidateAttempts)
            {
                outError = "Dependency resolution exceeds the candidate-attempt limit.";
                return SLANG_FAIL;
            }

            List<ResolutionPackage> snapshot = packages;
            ResolvedManifest manifest;
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
                addWarning(
                    String("Local path for package '") + selected.name +
                    "' shadows a Git dependency from " + selected.git + ".");
                selected.locked.path = candidate.path;
            }
            else
            {
                selected.locked.tag = candidate.tag;
                selected.locked.commit = candidate.commit;
            }
            selected.resolvedManifest = manifest;
            selected.locked.exports = manifest.manifest.exports;
            selected.locked.dependencies = manifest.manifest.dependencies;

            bool dependencyConflict = false;
            for (const auto& dependency : manifest.manifest.dependencies)
            {
                if (SLANG_FAILED(addDependency(dependency, manifest, candidateError)))
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
    resolver.projectRoot = ".";
    return resolver.resolve(manifest, outLock, outError);
}

SlangResult resolveDependenciesWithSource(
    const String& projectRoot,
    const Manifest& manifest,
    IPackageResolverSource& source,
    LockFile& outLock,
    String& outError,
    List<String>* outWarnings)
{
    Resolver resolver;
    resolver.source = &source;
    resolver.projectRoot = projectRoot;
    resolver.warnings = outWarnings;
    return resolver.resolve(manifest, outLock, outError);
}

SlangResult resolveDependencies(
    const String& projectRoot,
    const Manifest& manifest,
    LockFile& outLock,
    String& outError,
    List<String>* outWarnings)
{
    GitPackageResolverSource source;
    source.projectRoot = projectRoot;
    SLANG_RETURN_ON_FAIL(source.initialize(outError));
    Resolver resolver;
    resolver.source = &source;
    resolver.projectRoot = projectRoot;
    resolver.warnings = outWarnings;
    return resolver.resolve(manifest, outLock, outError);
}

SlangResult resolveDependenciesFromLocalPackages(
    const String& projectRoot,
    const Manifest& manifest,
    const List<LocalPackage>& localPackages,
    LockFile& outLock,
    String& outError,
    List<String>* outWarnings)
{
    LocalPackageResolverSource source;
    source.projectRoot = projectRoot;
    source.localPackages = &localPackages;
    SLANG_RETURN_ON_FAIL(source.initialize(outError));
    Resolver resolver;
    resolver.source = &source;
    resolver.projectRoot = projectRoot;
    resolver.warnings = outWarnings;
    return resolver.resolve(manifest, outLock, outError);
}

} // namespace PackageTool
} // namespace Slang
