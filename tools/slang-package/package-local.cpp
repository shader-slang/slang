// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-local.h"

#include "core/slang-io.h"
#include "package-json.h"

namespace Slang
{
namespace PackageTool
{

static const char* const kManifestName = "slang-package.json";
static const char* const kWorkspaceName = "slang-workspace.json";

Index findLocalPackageIndex(const List<LocalPackage>& packages, const String& name)
{
    for (Index i = 0; i < packages.getCount(); ++i)
    {
        if (packages[i].name == name)
            return i;
    }
    return -1;
}

Index findActiveLocalPackageIndex(const List<LocalPackage>& packages, const String& name)
{
    Index index = findLocalPackageIndex(packages, name);
    return index >= 0 && isActiveLocalPackage(packages[index]) ? index : -1;
}

SlangResult readProjectLocalPackages(
    const String& projectRoot,
    List<LocalPackage>& outPackages,
    String& outError)
{
    String path = Path::combine(projectRoot, kWorkspaceName);
    if (!File::exists(path))
    {
        outPackages.clear();
        return SLANG_OK;
    }
    SLANG_RETURN_ON_FAIL(readLocalPackages(path, outPackages, outError));
    Manifest manifest;
    SLANG_RETURN_ON_FAIL(
        readManifest(Path::combine(projectRoot, kManifestName), manifest, outError));
    String depsDirectory = getWorkspaceDepsDirectory(manifest);
    for (auto& package : outPackages)
    {
        if (isEditedLocalPackage(package))
            package.path = Path::combine(depsDirectory, package.name);
    }
    return SLANG_OK;
}

SlangResult writeProjectLocalPackages(
    const String& projectRoot,
    const List<LocalPackage>& packages,
    String& outError)
{
    return writeLocalPackages(Path::combine(projectRoot, kWorkspaceName), packages, outError);
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
