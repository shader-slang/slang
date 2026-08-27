// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-docs.h"

#include "core/slang-io.h"
#include "package-json.h"
#include "package-local.h"
#include "package-lock.h"
#include "package-path.h"
#include "package-validate.h"

#include <stdio.h>

namespace Slang
{
namespace PackageTool
{

static const char* const kManifestName = "slang-package.json";
static const char* const kLockName = "slang-package-lock.json";
static const Index kMaxDocumentationFileCount = 16384;
static const Index kMaxDocumentationDirectoryCount = 4096;

struct DocumentationEntry
{
    Path::Type type;
    String name;
};

class DocumentationEntryCollector : public Path::Visitor
{
public:
    List<DocumentationEntry> entries;

    virtual void accept(Path::Type type, const UnownedStringSlice& filename) override
    {
        DocumentationEntry entry;
        entry.type = type;
        entry.name = filename;
        entries.add(entry);
    }
};

/// Copy Markdown files recursively while refusing directory links that leave the package's docs
/// tree.
static SlangResult _copyMarkdownFilesRec(
    const String& sourceRoot,
    const String& canonicalSourceRoot,
    const String& relativeDirectory,
    const String& destinationRoot,
    const String& canonicalDestinationRoot,
    List<String>& ioVisitedDirectories,
    Index& ioFileCount,
    String& outError)
{
    String sourceDirectory =
        relativeDirectory.getLength() ? Path::combine(sourceRoot, relativeDirectory) : sourceRoot;
    String canonicalDirectory;
    if (SLANG_FAILED(Path::getCanonical(sourceDirectory, canonicalDirectory)) ||
        !isCanonicalPathWithin(canonicalSourceRoot, canonicalDirectory))
    {
        outError = String("Documentation directory escapes its package: ") + sourceDirectory;
        return SLANG_FAIL;
    }
    if (ioVisitedDirectories.contains(canonicalDirectory))
        return SLANG_OK;
    if (ioVisitedDirectories.getCount() >= kMaxDocumentationDirectoryCount)
    {
        outError = String("Package documentation exceeds the directory limit: ") + sourceRoot;
        return SLANG_FAIL;
    }
    ioVisitedDirectories.add(canonicalDirectory);

    DocumentationEntryCollector collector;
    if (SLANG_FAILED(Path::find(sourceDirectory, nullptr, &collector)))
    {
        outError = String("Cannot enumerate documentation directory: ") + sourceDirectory;
        return SLANG_FAIL;
    }
    collector.entries.sort([](const DocumentationEntry& left, const DocumentationEntry& right)
                           { return left.name < right.name; });
    for (const auto& entry : collector.entries)
    {
        String relativePath = relativeDirectory.getLength()
                                  ? Path::combine(relativeDirectory, entry.name)
                                  : entry.name;
        if (entry.type == Path::Type::Directory)
        {
            SLANG_RETURN_ON_FAIL(_copyMarkdownFilesRec(
                sourceRoot,
                canonicalSourceRoot,
                relativePath,
                destinationRoot,
                canonicalDestinationRoot,
                ioVisitedDirectories,
                ioFileCount,
                outError));
            continue;
        }
        if (entry.type != Path::Type::File ||
            !relativePath.getUnownedSlice().endsWithCaseInsensitive(".md"))
        {
            continue;
        }
        if (ioFileCount >= kMaxDocumentationFileCount)
        {
            outError = String("Package documentation exceeds the file limit: ") + sourceRoot;
            return SLANG_FAIL;
        }

        String sourcePath = Path::combine(sourceRoot, relativePath);
        String canonicalSourcePath;
        if (SLANG_FAILED(Path::getCanonical(sourcePath, canonicalSourcePath)) ||
            !isCanonicalPathWithin(canonicalSourceRoot, canonicalSourcePath))
        {
            outError = String("Documentation file escapes its package: ") + sourcePath;
            return SLANG_FAIL;
        }
        String destinationPath = Path::combine(destinationRoot, relativePath);
        if (!Path::createDirectoryRecursive(Path::getParentDirectory(destinationPath)))
        {
            outError =
                String("Cannot create documentation output directory for: ") + destinationPath;
            return SLANG_FAIL;
        }
        String canonicalDestinationDirectory;
        if (SLANG_FAILED(Path::getCanonical(
                Path::getParentDirectory(destinationPath),
                canonicalDestinationDirectory)) ||
            !isCanonicalPathWithin(canonicalDestinationRoot, canonicalDestinationDirectory))
        {
            outError =
                String("Documentation output directory escapes build/docs: ") + destinationPath;
            return SLANG_FAIL;
        }
        if (File::exists(destinationPath))
        {
            String canonicalDestinationPath;
            if (SLANG_FAILED(Path::getCanonical(destinationPath, canonicalDestinationPath)) ||
                !isCanonicalPathWithin(canonicalDestinationRoot, canonicalDestinationPath))
            {
                outError =
                    String("Documentation output file escapes build/docs: ") + destinationPath;
                return SLANG_FAIL;
            }
        }
        List<unsigned char> contents;
        if (SLANG_FAILED(File::readAllBytes(sourcePath, contents)) ||
            SLANG_FAILED(
                File::writeAllBytes(destinationPath, contents.getBuffer(), contents.getCount())))
        {
            outError = String("Cannot copy documentation file: ") + sourcePath;
            return SLANG_FAIL;
        }
        ++ioFileCount;
    }
    return SLANG_OK;
}

SlangResult buildDocumentation(const String& projectRoot, String& outError)
{
    List<String> warnings;
    SLANG_RETURN_ON_FAIL(validateProject(
        projectRoot,
        outError,
        &warnings,
        nullptr,
        ProjectValidationMode::SourceAndDependencies));
    for (const auto& warning : warnings)
        fprintf(stderr, "slang-package: warning: %s\n", warning.getBuffer());

    Manifest manifest;
    SLANG_RETURN_ON_FAIL(
        readManifest(Path::combine(projectRoot, kManifestName), manifest, outError));
    List<String> packageNames;
    List<String> packageRoots;
    packageNames.add(manifest.name);
    packageRoots.add(projectRoot);

    String lockPath = Path::combine(projectRoot, kLockName);
    if (File::exists(lockPath))
    {
        LockFile lock;
        List<LocalPackage> localPackages;
        SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
        SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
        for (const auto& package : lock.packages)
        {
            String packageRoot;
            SLANG_RETURN_ON_FAIL(getLockedPackageRoot(
                projectRoot,
                getWorkspaceDepsDirectory(manifest),
                package,
                localPackages,
                packageRoot,
                outError));
            packageNames.add(package.name);
            packageRoots.add(packageRoot);
        }
    }

    String destinationRoot =
        Path::combine(projectRoot, getWorkspaceBuildDirectory(manifest), "docs");
    if (!Path::createDirectoryRecursive(destinationRoot))
    {
        outError = String("Cannot create documentation output directory: ") + destinationRoot;
        return SLANG_FAIL;
    }
    String canonicalDestinationRoot;
    if (SLANG_FAILED(Path::getCanonical(destinationRoot, canonicalDestinationRoot)))
    {
        outError = String("Cannot canonicalize documentation output directory: ") + destinationRoot;
        return SLANG_FAIL;
    }

    Index copiedFileCount = 0;
    for (Index i = 0; i < packageRoots.getCount(); ++i)
    {
        String sourceRoot = Path::combine(packageRoots[i], "docs");
        SlangPathType sourceType;
        if (SLANG_FAILED(Path::getPathType(sourceRoot, &sourceType)) ||
            sourceType != SLANG_PATH_TYPE_DIRECTORY)
        {
            continue;
        }
        String canonicalSourceRoot;
        if (SLANG_FAILED(Path::getCanonical(sourceRoot, canonicalSourceRoot)))
        {
            outError = String("Cannot canonicalize documentation directory: ") + sourceRoot;
            return SLANG_FAIL;
        }
        if (isCanonicalPathWithin(canonicalSourceRoot, canonicalDestinationRoot))
        {
            outError = "Workspace build/docs must not be inside a package documentation directory.";
            return SLANG_FAIL;
        }

        List<String> visitedDirectories;
        String packageDestination = Path::combine(destinationRoot, packageNames[i]);
        if (!Path::createDirectoryRecursive(packageDestination))
        {
            outError = String("Cannot create package documentation output directory: ") +
                       packageDestination;
            return SLANG_FAIL;
        }
        String canonicalPackageDestination;
        if (SLANG_FAILED(Path::getCanonical(packageDestination, canonicalPackageDestination)) ||
            !isCanonicalPathWithin(canonicalDestinationRoot, canonicalPackageDestination))
        {
            outError =
                String("Package documentation output escapes build/docs: ") + packageDestination;
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(_copyMarkdownFilesRec(
            sourceRoot,
            canonicalSourceRoot,
            String(),
            packageDestination,
            canonicalPackageDestination,
            visitedDirectories,
            copiedFileCount,
            outError));
    }
    fprintf(stdout, "Copied %lld documentation file(s).\n", (long long)copiedFileCount);
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
