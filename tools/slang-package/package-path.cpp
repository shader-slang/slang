// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-path.h"

#include "core/slang-io.h"

namespace Slang
{
namespace PackageTool
{

bool isCanonicalPathWithin(const String& canonicalRoot, const String& canonicalPath)
{
    if (canonicalPath == canonicalRoot)
        return true;
    UnownedStringSlice root = canonicalRoot.getUnownedSlice();
    UnownedStringSlice path = canonicalPath.getUnownedSlice();
    return path.startsWith(root) && path.getLength() > root.getLength() &&
           Path::isDelimiter(path[root.getLength()]);
}

bool pathStartsWithParentComponent(const String& path)
{
    List<UnownedStringSlice> components;
    Path::split(path.getUnownedSlice(), components);
    return components.getCount() != 0 && components[0] == "..";
}

SlangResult validatePathDoesNotEscapeIntoToolState(
    const String& projectRoot,
    const String& canonicalDeclaringRoot,
    const String& canonicalPath,
    const String& dependencyName,
    String& outError)
{
    String canonicalStateRoot;
    if (SLANG_FAILED(Path::getCanonical(Path::combine(projectRoot, ".slang"), canonicalStateRoot)))
        return SLANG_OK;
    bool declaringIsInsideToolState =
        isCanonicalPathWithin(canonicalStateRoot, canonicalDeclaringRoot);
    bool pathIsInsideToolState = isCanonicalPathWithin(canonicalStateRoot, canonicalPath);
    bool pathIsInsideDeclaringPackage =
        isCanonicalPathWithin(canonicalDeclaringRoot, canonicalPath);
    if (pathIsInsideToolState && !(declaringIsInsideToolState && pathIsInsideDeclaringPackage))
    {
        outError =
            String("Path dependency cannot use package-tool state under .slang: ") + dependencyName;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult validateLockedPathDependency(
    const String& projectRoot,
    const String& declaringRoot,
    const String& declaringPackageName,
    const Dependency& dependency,
    const LockedPackage& lockedPackage,
    String& outError,
    List<String>* outWarnings)
{
    if (!dependency.path.getLength())
        return SLANG_OK;
    String canonicalExpectedPath;
    String canonicalLockedPath;
    if (SLANG_FAILED(Path::getCanonical(
            Path::combine(declaringRoot, dependency.path),
            canonicalExpectedPath)) ||
        SLANG_FAILED(Path::getCanonical(
            Path::combine(projectRoot, lockedPackage.path),
            canonicalLockedPath)) ||
        canonicalExpectedPath != canonicalLockedPath)
    {
        outError = String("Locked path does not match dependency '") + dependency.name +
                   "'. Run 'slang package update'.";
        return SLANG_FAIL;
    }
    String canonicalDeclaringRoot;
    if (SLANG_FAILED(Path::getCanonical(declaringRoot, canonicalDeclaringRoot)))
    {
        outError = String("Cannot canonicalize package root: ") + declaringRoot;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(validatePathDoesNotEscapeIntoToolState(
        projectRoot,
        canonicalDeclaringRoot,
        canonicalExpectedPath,
        dependency.name,
        outError));
    if (outWarnings && !isCanonicalPathWithin(canonicalDeclaringRoot, canonicalExpectedPath))
    {
        String warning = String("Path dependency '") + dependency.name + "' escapes package '" +
                         declaringPackageName + "': " + dependency.path;
        if (!outWarnings->contains(warning))
            outWarnings->add(warning);
    }
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
