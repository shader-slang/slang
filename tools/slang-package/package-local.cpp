// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-local.h"

#include "core/slang-io.h"
#include "package-json.h"

namespace Slang
{
namespace PackageTool
{

static const char* const kManifestName = "slang-package.json";

Index findLocalPackageIndex(const List<LocalPackage>& packages, const String& name)
{
    for (Index i = 0; i < packages.getCount(); ++i)
    {
        if (packages[i].name == name)
            return i;
    }
    return -1;
}

SlangResult readProjectLocalPackages(
    const String& projectRoot,
    List<LocalPackage>& outPackages,
    String& outError)
{
    String path = Path::combine(projectRoot, ".slang", "overrides.json");
    if (!File::exists(path))
    {
        outPackages.clear();
        return SLANG_OK;
    }
    return readLocalPackages(path, outPackages, outError);
}

SlangResult writeProjectLocalPackages(
    const String& projectRoot,
    const List<LocalPackage>& packages,
    String& outError)
{
    String slangDirectory = Path::combine(projectRoot, ".slang");
    if (!Path::createDirectoryRecursive(slangDirectory))
    {
        outError = String("Cannot create package state directory: ") + slangDirectory;
        return SLANG_FAIL;
    }
    return writeLocalPackages(Path::combine(slangDirectory, "overrides.json"), packages, outError);
}

SlangResult getLocalPackageRoot(
    const String& projectRoot,
    const LocalPackage& package,
    String& outRoot,
    String& outError)
{
    String path = Path::combine(projectRoot, package.path);
    SlangPathType type;
    if (SLANG_FAILED(Path::getPathType(path, &type)) || type != SLANG_PATH_TYPE_DIRECTORY ||
        SLANG_FAILED(Path::getCanonical(path, outRoot)))
    {
        outError = String("Registered local package directory does not exist: ") + path;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult readLocalPackageManifest(
    const String& projectRoot,
    const LocalPackage& package,
    Manifest& outManifest,
    String& outError)
{
    String root;
    SLANG_RETURN_ON_FAIL(getLocalPackageRoot(projectRoot, package, root, outError));
    SLANG_RETURN_ON_FAIL(readManifest(Path::combine(root, kManifestName), outManifest, outError));
    if (outManifest.name != package.name)
    {
        outError = String("Registered local package '") + package.name + "' has manifest name '" +
                   outManifest.name + "'.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
