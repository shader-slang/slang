// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_RESOLVER_H
#define SLANG_PACKAGE_RESOLVER_H

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

/// A parsed manifest together with the directory used to resolve its path dependencies and the
/// app-relative directory that will contain the package after fetch.
struct ResolvedManifest
{
    Manifest manifest;
    String ownerKey;
    String sourceRoot;
    String lockRoot;
    String gitRepositoryPath;
    String gitRevision;
    String gitRelativeRoot;
};

/// Supplies release candidates and their manifests to the dependency solver.
///
/// Production resolution uses Git, while tests and local overrides can provide the same semantic
/// inputs without invoking an external process.
class IPackageResolverSource
{
public:
    virtual ~IPackageResolverSource() {}

    virtual SlangResult listReleaseTags(
        const String& packageName,
        const String& git,
        List<TagCandidate>& outCandidates,
        String& outError) = 0;

    virtual SlangResult loadManifest(
        const String& packageName,
        const String& git,
        const TagCandidate& candidate,
        ResolvedManifest& outManifest,
        String& outError) = 0;
};

/// Resolve dependencies using an explicitly supplied package source.
SlangResult resolveDependenciesWithSource(
    const Manifest& manifest,
    IPackageResolverSource& source,
    LockFile& outLock,
    String& outError);

/// Resolve dependencies using an explicit source and app root.
SlangResult resolveDependenciesWithSource(
    const String& projectRoot,
    const Manifest& manifest,
    IPackageResolverSource& source,
    LockFile& outLock,
    String& outError,
    List<String>* outWarnings = nullptr);

/// Resolve dependencies from Git repositories, using a cache under `projectRoot`.
SlangResult resolveDependencies(
    const String& projectRoot,
    const Manifest& manifest,
    LockFile& outLock,
    String& outError,
    List<String>* outWarnings = nullptr);

/// Resolve dependencies using registered local manifests and Git for the remaining packages.
SlangResult resolveDependenciesFromLocalPackages(
    const String& projectRoot,
    const Manifest& manifest,
    const List<LocalPackage>& localPackages,
    LockFile& outLock,
    String& outError,
    List<String>* outWarnings = nullptr);

} // namespace PackageTool
} // namespace Slang

#endif
