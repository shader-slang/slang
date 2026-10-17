#pragma once

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

bool isValidPackageName(const String& name);

SlangResult readManifest(const String& path, Manifest& outManifest, String& outError);
SlangResult readManifestText(
    const String& sourceName,
    const String& text,
    Manifest& outManifest,
    String& outError);
SlangResult writeManifest(const String& path, const Manifest& manifest, String& outError);

/// Read a package index from a local path or an `http`/`https` URL.
///
/// A relative path is resolved from the current directory. `SLANG_PACKAGE_INDEX` is the location
/// `dependency add` reads when `--git` is omitted. The file is JSON with `"schema_version": 1`
/// and a `packages` object that maps a package name to one Git URL.
SlangResult readPackageIndex(
    const String& location,
    List<RepositoryLocation>& outPackages,
    String& outError);

SlangResult readLockFile(const String& path, LockFile& outLock, String& outError);
SlangResult writeLockFile(const String& path, const LockFile& lock, String& outError);

SlangResult readLocalPackages(
    const String& path,
    List<LocalPackage>& outPackages,
    String& outError);
SlangResult writeLocalPackages(
    const String& path,
    const List<LocalPackage>& packages,
    String& outError);

} // namespace PackageTool
} // namespace Slang
