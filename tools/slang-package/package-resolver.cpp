// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-resolver.h"

#include "core/slang-io.h"
#include "package-git.h"
#include "package-json.h"
#include "package-local.h"
#include "package-path.h"
#include "package-report.h"

namespace Slang
{
namespace PackageTool
{

struct GitRequirement
{
    String owner;
    String git;
    String ref;
    String as;
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
    ResolveSelectionKind selectionKind = ResolveSelectionKind::HighestRelease;
    List<ResolveConstraintNote> constraintNotes;
    List<ResolveSkipNote> skips;
    LockedPackage locked;
    ResolvedManifest resolvedManifest;
};

static String _rootOwnerKey()
{
    return "<root>";
}

static String _gitOwnerKey(const String& packageName, const String& commit)
{
    return String("git:") + packageName + "@" + commit;
}

static String _pathOwnerKey(const String& canonicalPath)
{
    return String("path:") + canonicalPath;
}

static String _localOwnerKey(const String& packageName, const String& path)
{
    return String("local:") + packageName + ":" + path;
}

static String _candidateOwnerKey(const String& packageName, const String& ref)
{
    return String("candidate:") + packageName + "@" + ref;
}

class GitPackageResolverSource : public IPackageResolverSource
{
public:
    String projectRoot;
    String cacheRoot;
    String depsDirectory;

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

    virtual SlangResult resolveReference(
        const String&,
        const String& git,
        const String& ref,
        TagCandidate& outCandidate,
        String& outError) override
    {
        return PackageTool::resolveReference(git, ref, outCandidate, outError);
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
        String sourceName = git + "@" + candidate.ref + ":slang-package.json";
        SLANG_RETURN_ON_FAIL(
            readManifestText(sourceName, manifestText, outManifest.manifest, outError));
        outManifest.ownerKey = _gitOwnerKey(packageName, candidate.commit);
        outManifest.lockRoot = Path::combine(depsDirectory, packageName);
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
        gitSource.depsDirectory = depsDirectory;
        return gitSource.initialize(outError);
    }

    String depsDirectory;

    virtual SlangResult listReleaseTags(
        const String& packageName,
        const String& git,
        List<TagCandidate>& outCandidates,
        String& outError) override
    {
        Index localIndex = findActiveLocalPackageIndex(*localPackages, packageName);
        if (localIndex < 0)
            return gitSource.listReleaseTags(packageName, git, outCandidates, outError);

        const LocalPackage& localPackage = (*localPackages)[localIndex];
        if (isEditedLocalPackage(localPackage))
        {
            String localRoot;
            SLANG_RETURN_ON_FAIL(
                getLocalPackageRoot(projectRoot, localPackage, localRoot, outError));
            String headCommit;
            SLANG_RETURN_ON_FAIL(getRepositoryHeadCommit(localRoot, headCommit, outError));
            List<TagCandidate> releaseCandidates;
            SLANG_RETURN_ON_FAIL(
                gitSource.listReleaseTags(packageName, git, releaseCandidates, outError));
            for (const auto& release : releaseCandidates)
            {
                if (release.commit != headCommit)
                    continue;
                TagCandidate candidate = release;
                candidate.path = localPackage.path;
                candidate.isEdit = true;
                outCandidates.clear();
                outCandidates.add(candidate);
                return SLANG_OK;
            }
            outError = String("Edited package HEAD is not a published release tag: ") + packageName;
            return SLANG_FAIL;
        }
        TagCandidate candidate;
        candidate.path = localPackage.path;
        SLANG_RETURN_ON_FAIL(parseExactVersion(localPackage.as, candidate.version, outError));
        outCandidates.clear();
        outCandidates.add(candidate);
        return SLANG_OK;
    }

    virtual SlangResult resolveReference(
        const String& packageName,
        const String& git,
        const String& ref,
        TagCandidate& outCandidate,
        String& outError) override
    {
        Index localIndex = findActiveLocalPackageIndex(*localPackages, packageName);
        if (localIndex < 0)
            return gitSource.resolveReference(packageName, git, ref, outCandidate, outError);

        const LocalPackage& localPackage = (*localPackages)[localIndex];
        if (isEditedLocalPackage(localPackage))
        {
            SLANG_RETURN_ON_FAIL(
                gitSource.resolveReference(packageName, git, ref, outCandidate, outError));
            String localRoot;
            SLANG_RETURN_ON_FAIL(
                getLocalPackageRoot(projectRoot, localPackage, localRoot, outError));
            String headCommit;
            SLANG_RETURN_ON_FAIL(getRepositoryHeadCommit(localRoot, headCommit, outError));
            if (headCommit != outCandidate.commit)
            {
                outError =
                    String("Edited package HEAD does not match pinned Git ref: ") + packageName;
                return SLANG_FAIL;
            }
            outCandidate.path = localPackage.path;
            outCandidate.isEdit = true;
            return SLANG_OK;
        }
        SemanticVersion version;
        SLANG_RETURN_ON_FAIL(parseExactVersion(localPackage.as, version, outError));
        outCandidate = TagCandidate();
        outCandidate.path = localPackage.path;
        outCandidate.ref = ref;
        outCandidate.version = version;
        return SLANG_OK;
    }

    virtual SlangResult loadManifest(
        const String& packageName,
        const String& git,
        const TagCandidate& candidate,
        ResolvedManifest& outManifest,
        String& outError) override
    {
        if (candidate.isEdit)
            return gitSource.loadManifest(packageName, git, candidate, outManifest, outError);
        if (!candidate.path.getLength())
            return gitSource.loadManifest(packageName, git, candidate, outManifest, outError);

        Index localIndex = findActiveLocalPackageIndex(*localPackages, packageName);
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
        outManifest.ownerKey = _localOwnerKey(packageName, candidate.path);
        outManifest.lockRoot = (*localPackages)[localIndex].path;
        return SLANG_OK;
    }
};

/// Resolve the workspace package's dependency graph to one lock entry per package name.
///
/// Consider this example: the workspace package depends on `b` from Git (`>=1.0.0`) and on `a` by
/// relative path, and `a` also depends on `b` by path. Name identity is unique, so there is one
/// `b`. The path edge wins, Git constraints that only the Git pin contributed must disappear, and
/// transitives that existed only because of that pin must be pruned. Path packages are selected
/// immediately; Git packages are searched by release tag or resolved from one pinned ref.
/// `ownerKey` records which selected representation added each Git requirement so a later path
/// selection can retract it. Every candidate has one effective semantic version, including paths
/// and local overrides, so all incoming version constraints use the same matching path.
class Resolver
{
public:
    static const Index kMaxPackageCount = 256;
    static const Index kMaxCandidateAttempts = 4096;

    IPackageResolverSource* source = nullptr;
    String projectRoot;
    List<String>* warnings = nullptr;
    ResolveReport* report = nullptr;
    const Manifest* rootManifest = nullptr;
    List<ResolutionPackage> packages;
    Index candidateAttemptCount = 0;

    SlangResult resolve(const Manifest& rootManifest, LockFile& outLock, String& outError)
    {
        this->rootManifest = &rootManifest;
        ResolvedManifest root;
        root.manifest = rootManifest;
        root.ownerKey = _rootOwnerKey();
        root.sourceRoot = projectRoot;
        root.lockRoot = ".";
        for (const auto& dependency : rootManifest.dependencies)
            SLANG_RETURN_ON_FAIL(addDependency(dependency, root, outError));

        SLANG_RETURN_ON_FAIL(search(outError));
        outLock = LockFile();
        List<bool> reachable;
        getReachablePackages(reachable);
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (!reachable[i])
                continue;
            if (!packages[i].selected)
            {
                outError = String("Internal resolver error: reachable package '") +
                           packages[i].name + "' is unexpectedly unselected.";
                return SLANG_FAIL;
            }
            outLock.packages.add(packages[i].locked);
        }
        outLock.packages.sort([](const LockedPackage& left, const LockedPackage& right)
                              { return left.name < right.name; });
        List<ToolchainConstraint> toolchainConstraints;
        addSlangToolchainConstraint(rootManifest, toolchainConstraints);
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (!reachable[i])
                continue;
            addSlangToolchainConstraint(
                packages[i].resolvedManifest.manifest,
                toolchainConstraints);
        }
        SLANG_RETURN_ON_FAIL(selectSlangToolchain(toolchainConstraints, outError));
        if (report)
        {
            publishReport();
            report->toolchainConstraints = toolchainConstraints;
            if (toolchainConstraints.getCount())
            {
                SemanticVersion installed;
                String installedText;
                String toolchainError;
                if (SLANG_SUCCEEDED(getInstalledSlangToolchainVersion(
                        installed,
                        installedText,
                        toolchainError)))
                    report->installedToolchain = installedText;
            }
        }
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (!reachable[i])
                continue;
            addUnadoptedWorkspaceExclusionWarnings(
                rootManifest,
                packages[i].name,
                packages[i].resolvedManifest.manifest,
                warnings);
        }
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

    bool getPinnedIdentity(
        const ResolutionPackage& package,
        String& outRef,
        String& outAs,
        SemanticVersion& outVersion) const
    {
        for (const auto& requirement : package.gitRequirements)
        {
            if (!requirement.ref.getLength())
                continue;
            outRef = requirement.ref;
            outAs = requirement.as;
            String error;
            SLANG_RELEASE_ASSERT(SLANG_SUCCEEDED(parseExactVersion(outAs, outVersion, error)));
            return true;
        }
        return false;
    }

    const Exclusion* findExclusion(const String& packageName, const SemanticVersion& version) const
    {
        for (const auto& exclusion : rootManifest->workspace.exclusions)
        {
            if (exclusion.packageName == packageName &&
                matchesVersionPolicy(exclusion.version, version))
            {
                return &exclusion;
            }
        }
        return nullptr;
    }

    static const Retraction* findRetraction(
        const List<Retraction>& retractions,
        const SemanticVersion& version)
    {
        for (const auto& retraction : retractions)
        {
            if (matchesVersionPolicy(retraction.version, version))
                return &retraction;
        }
        return nullptr;
    }

    /// Read publisher retractions from the highest release, independently of the workspace's
    /// version constraint. Local edit and override candidates are explicit developer choices and
    /// do not participate in remote retraction discovery.
    SlangResult loadPublisherRetractions(
        const ResolutionPackage& package,
        const List<TagCandidate>& candidates,
        List<Retraction>& outRetractions,
        String& outError)
    {
        outRetractions.clear();
        const TagCandidate* highestRelease = nullptr;
        for (const auto& candidate : candidates)
        {
            if (!candidate.path.getLength())
            {
                highestRelease = &candidate;
                break;
            }
        }
        if (!highestRelease)
            return SLANG_OK;

        ResolvedManifest latestManifest;
        SLANG_RETURN_ON_FAIL(
            loadCandidateManifest(package, *highestRelease, latestManifest, outError));
        outRetractions = latestManifest.manifest.retractions;
        return SLANG_OK;
    }

    void removeGitRequirementsFromOwner(const String& owner)
    {
        // Path selection of `owner`'s package retracts every Git constraint that representation
        // added, including constraints on other package names.
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
        if (owner == _rootOwnerKey())
            return true;
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (reachable[i] && packages[i].selected &&
                packages[i].resolvedManifest.ownerKey == owner)
                return true;
        }
        return false;
    }

    /// Drop Git constraints whose contributing representation is no longer reachable, and unselect
    /// path packages whose declaring owner disappeared. Repeat until the reachable set is stable;
    /// each iteration only removes requirements or selections, so it cannot cycle.
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

    String ownerVersionFor(const ResolvedManifest& declaringManifest) const
    {
        if (declaringManifest.ownerKey == _rootOwnerKey())
            return String();
        Index index = findPackage(declaringManifest.manifest.name);
        if (index < 0 || !packages[index].selected)
            return String();
        return packages[index].locked.version;
    }

    static String gitConstraintText(const Dependency& dependency)
    {
        String text = dependency.version;
        if (dependency.ref.getLength())
        {
            String pin = String("ref ") + dependency.ref + " as " + dependency.as;
            if (text.getLength())
                text = text + ", " + pin;
            else
                text = pin;
        }
        return text;
    }

    void addConstraintNote(
        ResolutionPackage& package,
        const ResolvedManifest& declaringManifest,
        const String& text,
        const VersionConstraint& constraint)
    {
        ResolveConstraintNote note;
        note.ownerName = declaringManifest.manifest.name;
        note.ownerVersion = ownerVersionFor(declaringManifest);
        note.text = text;
        note.constraint = constraint;
        package.constraintNotes.add(note);
    }

    void publishReport()
    {
        if (!report)
            return;
        *report = ResolveReport();
        report->rootPackageName = rootManifest->name;
        List<bool> reachable;
        getReachablePackages(reachable);
        for (Index i = 0; i < packages.getCount(); ++i)
        {
            if (!reachable[i])
                continue;
            const ResolutionPackage& package = packages[i];
            ResolvePackageExplanation explanation;
            explanation.name = package.name;
            explanation.version = package.locked.version;
            explanation.git = package.locked.git;
            explanation.ref = package.locked.ref;
            explanation.path = package.locked.path;
            explanation.selectionKind = package.selectionKind;
            explanation.constraints = package.constraintNotes;
            explanation.skips = package.skips;
            report->packages.add(explanation);
        }
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
            if (Path::isAbsolute(gitRelativeRoot) || pathStartsWithParentComponent(gitRelativeRoot))
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
            String canonicalDeclaringRoot;
            if (SLANG_FAILED(
                    Path::getCanonical(declaringManifest.sourceRoot, canonicalDeclaringRoot)))
            {
                outError =
                    String("Cannot canonicalize package root: ") + declaringManifest.sourceRoot;
                return SLANG_FAIL;
            }
            SLANG_RETURN_ON_FAIL(validatePathDoesNotEscapeIntoToolState(
                projectRoot,
                canonicalDeclaringRoot,
                outCanonicalPath,
                dependency.name,
                outError));
            SLANG_RETURN_ON_FAIL(readManifest(
                Path::combine(outCanonicalPath, "slang-package.json"),
                outManifest.manifest,
                outError));
            outManifest.sourceRoot = outCanonicalPath;
            if (isCanonicalPathWithin(canonicalDeclaringRoot, outCanonicalPath))
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
                outError =
                    String("Path dependency must be on the same filesystem as the workspace: ") +
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
        outManifest.ownerKey = _pathOwnerKey(outCanonicalPath);
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
        SemanticVersion pathVersion;
        SLANG_RETURN_ON_FAIL(parseExactVersion(dependency.as, pathVersion, outError));
        if (package.canonicalPath == canonicalPath && package.locked.path.getLength())
        {
            if (package.locked.version != dependency.as)
            {
                outError = String("Package '") + dependency.name +
                           "' is required from one path with different 'as' versions.";
                return SLANG_FAIL;
            }
            VersionConstraint asConstraint;
            String asError;
            SLANG_RETURN_ON_FAIL(parseVersionConstraint(dependency.as, asConstraint, asError));
            addConstraintNote(
                package,
                declaringManifest,
                String("as ") + dependency.as,
                asConstraint);
            return SLANG_OK;
        }
        if (!matchesAll(package, pathVersion))
        {
            outError = String("Path dependency '") + dependency.name + "' provides version " +
                       dependency.as + ", which conflicts with a Git version constraint.";
            return SLANG_FAIL;
        }
        String pinnedRef;
        String pinnedAs;
        SemanticVersion pinnedVersion;
        if (getPinnedIdentity(package, pinnedRef, pinnedAs, pinnedVersion) &&
            pinnedVersion != pathVersion)
        {
            outError = String("Path dependency '") + dependency.name + "' provides version " +
                       dependency.as + ", which conflicts with pinned Git version " + pinnedAs +
                       ".";
            return SLANG_FAIL;
        }
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
        package.selectionKind = ResolveSelectionKind::Path;
        package.resolvedManifest = pathManifest;
        package.locked = LockedPackage();
        package.locked.name = package.name;
        package.locked.path = pathManifest.lockRoot;
        package.locked.version = dependency.as;
        package.locked.exports = pathManifest.manifest.exports;
        package.locked.dependencies = pathManifest.manifest.dependencies;
        VersionConstraint asConstraint;
        String asError;
        SLANG_RETURN_ON_FAIL(parseVersionConstraint(dependency.as, asConstraint, asError));
        addConstraintNote(package, declaringManifest, String("as ") + dependency.as, asConstraint);
        pruneUnreachableGitRequirements();
        for (const auto& child : pathManifest.manifest.dependencies)
            SLANG_RETURN_ON_FAIL(addDependency(child, pathManifest, outError));
        return SLANG_OK;
    }

    /// Record one Git edge after checking that its source and optional pin agree with every other
    /// Git edge for the package. Keeping shadowed edges here lets the solver restore them if the
    /// path edge that currently wins later becomes unreachable.
    SlangResult addGitRequirement(
        ResolutionPackage& package,
        const Dependency& dependency,
        const ResolvedManifest& declaringManifest,
        const VersionConstraint& constraint,
        String& outError)
    {
        if (package.git.getLength() && package.git != dependency.git)
        {
            outError =
                String("Package '") + dependency.name + "' is required from more than one Git URL.";
            return SLANG_FAIL;
        }
        if (dependency.ref.getLength())
        {
            for (const auto& existing : package.gitRequirements)
            {
                if (existing.ref.getLength() &&
                    (existing.ref != dependency.ref || existing.as != dependency.as))
                {
                    outError = String("Package '") + dependency.name +
                               "' is pinned to more than one Git ref or 'as' version.";
                    return SLANG_FAIL;
                }
            }
        }

        package.git = dependency.git;
        GitRequirement requirement;
        requirement.owner = declaringManifest.ownerKey;
        requirement.git = dependency.git;
        requirement.ref = dependency.ref;
        requirement.as = dependency.as;
        requirement.constraint = constraint;
        package.gitRequirements.add(requirement);
        addConstraintNote(package, declaringManifest, gitConstraintText(dependency), constraint);
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
        if (dependency.version.getLength())
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
            packages.add(package);
            index = packages.getCount() - 1;
        }

        ResolutionPackage& package = packages[index];
        if (package.canonicalPath.getLength())
        {
            addWarning(
                String("Path dependency '") + dependency.name + "' shadows a Git dependency from " +
                dependency.git + ".");
            SemanticVersion selectedVersion;
            SLANG_RETURN_ON_FAIL(
                parseExactVersion(package.locked.version, selectedVersion, outError));
            if (!constraint.matches(selectedVersion))
            {
                outError = String("Path dependency '") + dependency.name + "' provides version " +
                           package.locked.version + ", which conflicts with a Git constraint.";
                return SLANG_FAIL;
            }
            if (dependency.ref.getLength() && dependency.as != package.locked.version)
            {
                outError = String("Path dependency '") + dependency.name + "' provides version " +
                           package.locked.version + ", which conflicts with pinned Git version " +
                           dependency.as + ".";
                return SLANG_FAIL;
            }
            SLANG_RETURN_ON_FAIL(
                addGitRequirement(package, dependency, declaringManifest, constraint, outError));
            return SLANG_OK;
        }
        SLANG_RETURN_ON_FAIL(
            addGitRequirement(package, dependency, declaringManifest, constraint, outError));
        if (package.selected)
        {
            SemanticVersion selectedVersion;
            SLANG_RETURN_ON_FAIL(
                parseExactVersion(package.locked.version, selectedVersion, outError));
            if (!constraint.matches(selectedVersion))
            {
                outError = String("Selected version of package '") + dependency.name +
                           "' conflicts with a transitive constraint.";
                return SLANG_FAIL;
            }
            if (dependency.ref.getLength() &&
                (package.locked.ref != dependency.ref || package.locked.version != dependency.as))
            {
                outError = String("Selected package '") + dependency.name +
                           "' conflicts with a pinned Git ref.";
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
            outManifest.ownerKey = _candidateOwnerKey(package.name, candidate.ref);
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
        List<Retraction> retractions;
        String pinnedRef;
        String pinnedAs;
        SemanticVersion pinnedVersion;
        if (getPinnedIdentity(unresolved, pinnedRef, pinnedAs, pinnedVersion))
        {
            TagCandidate candidate;
            SLANG_RETURN_ON_FAIL(source->resolveReference(
                unresolved.name,
                unresolved.git,
                pinnedRef,
                candidate,
                outError));
            candidate.version = pinnedVersion;
            candidates.add(candidate);
            if (!candidate.path.getLength())
            {
                List<TagCandidate> releaseCandidates;
                SLANG_RETURN_ON_FAIL(source->listReleaseTags(
                    unresolved.name,
                    unresolved.git,
                    releaseCandidates,
                    outError));
                SLANG_RETURN_ON_FAIL(
                    loadPublisherRetractions(unresolved, releaseCandidates, retractions, outError));
            }
        }
        else
        {
            SLANG_RETURN_ON_FAIL(
                source->listReleaseTags(unresolved.name, unresolved.git, candidates, outError));
            SLANG_RETURN_ON_FAIL(
                loadPublisherRetractions(unresolved, candidates, retractions, outError));
        }
        String lastCandidateError;
        for (const auto& candidate : candidates)
        {
            if (!matchesAll(unresolved, candidate.version))
                continue;
            if (!candidate.path.getLength())
            {
                if (const Exclusion* exclusion = findExclusion(unresolved.name, candidate.version))
                {
                    ResolveSkipNote skip;
                    skip.version = formatExactVersion(candidate.version);
                    skip.reason = String("workspace excludes this release — ") + exclusion->reason;
                    packages[unresolvedIndex].skips.add(skip);
                    addWarning(
                        String("Workspace excludes package '") + unresolved.name + "' release " +
                        candidate.ref + ": " + exclusion->reason);
                    continue;
                }
                if (const Retraction* retraction = findRetraction(retractions, candidate.version))
                {
                    ResolveSkipNote skip;
                    skip.version = formatExactVersion(candidate.version);
                    skip.reason = String("retracted — ") + retraction->reason;
                    packages[unresolvedIndex].skips.add(skip);
                    addWarning(
                        String("Package '") + unresolved.name + "' retracts release " +
                        candidate.ref + ": " + retraction->reason);
                    continue;
                }
            }
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
            if (getPinnedIdentity(selected, pinnedRef, pinnedAs, pinnedVersion))
                selected.selectionKind = ResolveSelectionKind::PinnedRef;
            else if (candidate.path.getLength() && candidate.isEdit)
                selected.selectionKind = ResolveSelectionKind::Edit;
            else if (candidate.path.getLength())
                selected.selectionKind = ResolveSelectionKind::Override;
            else
                selected.selectionKind = ResolveSelectionKind::HighestRelease;
            selected.locked.name = selected.name;
            selected.locked.git = selected.git;
            selected.locked.version = formatExactVersion(candidate.version);
            if (candidate.path.getLength() && !candidate.isEdit)
            {
                addWarning(
                    String("Local path for package '") + selected.name +
                    "' shadows a Git dependency from " + selected.git + ".");
                selected.locked.path = candidate.path;
            }
            else
            {
                selected.locked.ref = candidate.ref;
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

        outError =
            String("No package selection satisfies all constraints for '") + unresolved.name + "'.";
        if (lastCandidateError.getLength() != 0)
        {
            appendErrorAdvice(
                outError,
                String("Last candidate failed because: ") + lastCandidateError);
        }
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
    List<String>* outWarnings,
    ResolveReport* outReport)
{
    Resolver resolver;
    resolver.source = &source;
    resolver.projectRoot = projectRoot;
    resolver.warnings = outWarnings;
    resolver.report = outReport;
    return resolver.resolve(manifest, outLock, outError);
}

SlangResult resolveDependencies(
    const String& projectRoot,
    const Manifest& manifest,
    LockFile& outLock,
    String& outError,
    List<String>* outWarnings,
    ResolveReport* outReport)
{
    GitPackageResolverSource source;
    source.projectRoot = projectRoot;
    source.depsDirectory = getWorkspaceDepsDirectory(manifest);
    SLANG_RETURN_ON_FAIL(source.initialize(outError));
    Resolver resolver;
    resolver.source = &source;
    resolver.projectRoot = projectRoot;
    resolver.warnings = outWarnings;
    resolver.report = outReport;
    return resolver.resolve(manifest, outLock, outError);
}

SlangResult resolveDependenciesFromLocalPackages(
    const String& projectRoot,
    const Manifest& manifest,
    const List<LocalPackage>& localPackages,
    LockFile& outLock,
    String& outError,
    List<String>* outWarnings,
    ResolveReport* outReport)
{
    LocalPackageResolverSource source;
    source.projectRoot = projectRoot;
    source.depsDirectory = getWorkspaceDepsDirectory(manifest);
    source.localPackages = &localPackages;
    SLANG_RETURN_ON_FAIL(source.initialize(outError));
    Resolver resolver;
    resolver.source = &source;
    resolver.projectRoot = projectRoot;
    resolver.warnings = outWarnings;
    resolver.report = outReport;
    return resolver.resolve(manifest, outLock, outError);
}

} // namespace PackageTool
} // namespace Slang
