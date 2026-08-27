// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-bundle.h"

#include "compiler-core/slang-json-parser.h"
#include "core/slang-dictionary.h"
#include "core/slang-io.h"

namespace Slang
{
namespace PackageTool
{

#ifndef SLANG_PACKAGE_COMPILER_VERSION
#define SLANG_PACKAGE_COMPILER_VERSION "unknown"
#endif

SlangResult resetDirectory(const String& path, String& outError)
{
    SlangPathType pathType;
    if (SLANG_SUCCEEDED(Path::getPathType(path, &pathType)))
    {
        if (SLANG_FAILED(Path::removeNonEmpty(path)))
        {
            outError = String("Cannot replace bundle directory: ") + path;
            return SLANG_FAIL;
        }
    }
    if (!Path::createDirectoryRecursive(path))
    {
        outError = String("Cannot create bundle directory: ") + path;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult writeModuleProvenance(
    const String& modulesRoot,
    const String& slangcPath,
    String& outError)
{
    String canonicalSlangcPath = slangcPath;
    Path::getCanonical(slangcPath, canonicalSlangcPath);

    JSONWriter writer(JSONWriter::IndentationStyle::Allman);
    writer.startObject(SourceLoc());
    writer.addUnquotedKey(UnownedStringSlice("schema_version"), SourceLoc());
    writer.addIntegerValue(kSchemaVersion, SourceLoc());
    writer.addUnquotedKey(UnownedStringSlice("kind"), SourceLoc());
    writer.addStringValue(UnownedStringSlice("slang-modules"), SourceLoc());
    writer.addUnquotedKey(UnownedStringSlice("compiler"), SourceLoc());
    writer.startObject(SourceLoc());
    writer.addUnquotedKey(UnownedStringSlice("name"), SourceLoc());
    writer.addStringValue(UnownedStringSlice("slangc"), SourceLoc());
    writer.addUnquotedKey(UnownedStringSlice("version"), SourceLoc());
    writer.addStringValue(UnownedStringSlice(SLANG_PACKAGE_COMPILER_VERSION), SourceLoc());
    writer.addUnquotedKey(UnownedStringSlice("path"), SourceLoc());
    writer.addStringValue(canonicalSlangcPath.getUnownedSlice(), SourceLoc());
    writer.endObject(SourceLoc());
    writer.endObject(SourceLoc());
    writer.getBuilder() << "\n";

    String provenancePath = Path::combine(modulesRoot, "provenance.json");
    if (SLANG_FAILED(File::writeAllText(provenancePath, writer.getBuilder())))
    {
        outError = String("Cannot write module provenance: ") + provenancePath;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult copyBundleSource(
    const String& sourceRoot,
    const List<ExportedSourceFile>& sourceFiles,
    String& outError)
{
    Dictionary<String, String> occupied;
    Dictionary<String, String> occupiedPackage;
    for (const auto& file : sourceFiles)
    {
        String folded = file.relativePath.toLower();
        if (const String* existingPath = occupied.tryGetValue(folded))
        {
            const String* existingPackage = occupiedPackage.tryGetValue(folded);
            String otherPackage = existingPackage ? *existingPackage : String();
            if (*existingPath == file.relativePath)
            {
                outError = String("Bundle source path '") + file.relativePath +
                           "' is produced by both package '" + otherPackage + "' and package '" +
                           file.packageName + "'.";
            }
            else
            {
                outError = String("Bundle source path '") + file.relativePath +
                           "' conflicts with '" + *existingPath +
                           "' on a case-insensitive filesystem.";
            }
            return SLANG_FAIL;
        }
        occupied.add(folded, file.relativePath);
        occupiedPackage.add(folded, file.packageName);
    }

    SLANG_RETURN_ON_FAIL(resetDirectory(sourceRoot, outError));
    for (const auto& file : sourceFiles)
    {
        String destinationPath = Path::combine(sourceRoot, file.relativePath);
        if (!Path::createDirectoryRecursive(Path::getParentDirectory(destinationPath)))
        {
            outError = String("Cannot create bundle source directory for: ") + destinationPath;
            return SLANG_FAIL;
        }
        List<unsigned char> contents;
        if (SLANG_FAILED(File::readAllBytes(file.sourcePath, contents)) ||
            SLANG_FAILED(File::writeAllBytes(
                destinationPath,
                contents.getBuffer(),
                contents.getCount())))
        {
            outError = String("Cannot copy bundle source to: ") + destinationPath;
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
