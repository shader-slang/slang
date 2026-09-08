// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-validate.h"

#include "compiler-core/slang-lexer.h"
#include "core/slang-io.h"
#include "package-git.h"
#include "package-json.h"
#include "package-local.h"
#include "package-lock.h"
#include "package-path.h"
#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

static const char* const kLicensePlaceholder =
    "Replace this file with the package license before publishing.\n";
static const Index kMaxSourceFileCount = 16384;
static const Index kMaxSourceDirectoryCount = 4096;

const char* getLicensePlaceholderText()
{
    return kLicensePlaceholder;
}

struct DirectoryEntry
{
    Path::Type type;
    String name;
};

class DirectoryEntryCollector : public Path::Visitor
{
public:
    List<DirectoryEntry> entries;

    virtual void accept(Path::Type type, const UnownedStringSlice& filename) override
    {
        DirectoryEntry entry;
        entry.type = type;
        entry.name = filename;
        entries.add(entry);
    }
};

struct SourceFileCollection
{
    String canonicalRoot;
    List<String> relativePaths;
    List<String> visitedDirectories;
};

/// Collect each `.slang` file under an export without following a directory outside that export.
static SlangResult _collectSourceFilesRec(
    const String& exportRoot,
    const String& relativeDirectory,
    SourceFileCollection& ioCollection,
    String& outError)
{
    String directory =
        relativeDirectory.getLength() ? Path::combine(exportRoot, relativeDirectory) : exportRoot;
    String canonicalDirectory;
    if (SLANG_FAILED(Path::getCanonical(directory, canonicalDirectory)))
    {
        outError = String("Cannot canonicalize source directory: ") + directory;
        return SLANG_FAIL;
    }
    if (!isCanonicalPathWithin(ioCollection.canonicalRoot, canonicalDirectory))
    {
        outError = String("Source directory escapes its package export: ") + directory;
        return SLANG_FAIL;
    }
    if (ioCollection.visitedDirectories.contains(canonicalDirectory))
        return SLANG_OK;
    if (ioCollection.visitedDirectories.getCount() >= kMaxSourceDirectoryCount)
    {
        outError = String("Package source tree exceeds the directory limit: ") + exportRoot;
        return SLANG_FAIL;
    }
    ioCollection.visitedDirectories.add(canonicalDirectory);

    DirectoryEntryCollector collector;
    if (SLANG_FAILED(Path::find(directory, nullptr, &collector)))
    {
        outError = String("Cannot enumerate source directory: ") + directory;
        return SLANG_FAIL;
    }
    collector.entries.sort([](const DirectoryEntry& left, const DirectoryEntry& right)
                           { return left.name < right.name; });
    for (const auto& entry : collector.entries)
    {
        String relativePath = relativeDirectory.getLength()
                                  ? Path::combine(relativeDirectory, entry.name)
                                  : entry.name;
        if (entry.type == Path::Type::Directory)
        {
            SLANG_RETURN_ON_FAIL(
                _collectSourceFilesRec(exportRoot, relativePath, ioCollection, outError));
        }
        else if (
            entry.type == Path::Type::File &&
            relativePath.getUnownedSlice().endsWithCaseInsensitive(".slang"))
        {
            String canonicalPath;
            if (SLANG_FAILED(
                    Path::getCanonical(Path::combine(exportRoot, relativePath), canonicalPath)) ||
                !isCanonicalPathWithin(ioCollection.canonicalRoot, canonicalPath))
            {
                outError = String("Source file escapes its package export: ") +
                           Path::combine(exportRoot, relativePath);
                return SLANG_FAIL;
            }
            if (ioCollection.relativePaths.getCount() >= kMaxSourceFileCount)
            {
                outError = String("Package source tree exceeds the file limit: ") + exportRoot;
                return SLANG_FAIL;
            }
            ioCollection.relativePaths.add(relativePath);
        }
    }
    return SLANG_OK;
}

static String _normalizePath(const String& path)
{
    StringBuilder result;
    for (auto c : path.getUnownedSlice())
        result.append(c == '\\' ? '/' : c);
    return result.produceString();
}

static String _canonicalModuleName(const String& name)
{
    String normalized = _normalizePath(name);
    Index separator = normalized.getUnownedSlice().lastIndexOf('/');
    UnownedStringSlice simpleName = separator < 0
                                        ? normalized.getUnownedSlice()
                                        : normalized.getUnownedSlice().tail(separator + 1);
    if (simpleName.endsWithCaseInsensitive(".slang"))
        simpleName = simpleName.head(simpleName.getLength() - 6);

    StringBuilder result;
    for (auto c : simpleName)
        result.append(c == '-' ? '_' : c);
    return result.produceString();
}

enum class ModuleHeaderKind
{
    Module,
    Implementing,
};

/// Read the required first `module` or `implementing` declaration from a Slang source file.
static SlangResult _readModuleHeader(
    const String& path,
    ModuleHeaderKind& outKind,
    String& outName,
    String& outError)
{
    String contents;
    if (SLANG_FAILED(File::readAllText(path, contents)))
    {
        outError = String("Cannot read Slang source file: ") + path;
        return SLANG_FAIL;
    }

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);
    DiagnosticSink sink(&sourceManager, Lexer::sourceLocationLexer);
    SourceFile* sourceFile =
        sourceManager.createSourceFileWithString(PathInfo::makePath(path), contents);
    SourceView* sourceView = sourceManager.createSourceView(sourceFile, nullptr, SourceLoc());
    NamePool namePool;
    Lexer lexer;
    lexer.initialize(sourceView, &sink, &namePool, sourceManager.getMemoryArena());
    TokenList tokens = lexer.lexAllSemanticTokens();
    TokenReader reader(tokens);

    Token declaration = reader.advanceToken();
    if (declaration.type != TokenType::Identifier)
    {
        outError =
            String("Slang package source must start with 'module' or 'implementing': ") + path;
        return SLANG_FAIL;
    }
    if (declaration.getContent() == "module")
        outKind = ModuleHeaderKind::Module;
    else if (declaration.getContent() == "implementing")
        outKind = ModuleHeaderKind::Implementing;
    else
    {
        outError =
            String("Slang package source must start with 'module' or 'implementing': ") + path;
        return SLANG_FAIL;
    }

    Token name = reader.advanceToken();
    if (name.type == TokenType::StringLiteral)
        outName = getFileNameTokenValue(name);
    else if (name.type == TokenType::Identifier)
        outName = name.getContent();
    else
    {
        outError = String("Module declaration must name its module: ") + path;
        return SLANG_FAIL;
    }
    if (Path::hasPath(outName))
    {
        outError = String("Module declaration must use a simple name: ") + path;
        return SLANG_FAIL;
    }
    if (reader.advanceToken().type != TokenType::Semicolon)
    {
        outError = String("Module declaration must use a simple name followed by ';': ") + path;
        return SLANG_FAIL;
    }
    outName = _canonicalModuleName(outName);
    return SLANG_OK;
}

struct ModuleLocation
{
    String canonicalImport;
    String importPath;
    String packageName;
    String path;
};

/// Copy the validated module index into the public build inventory.
static void _collectPrimaryModules(
    const List<ModuleLocation>& modules,
    List<PrimaryModule>& outPrimaryModules)
{
    outPrimaryModules.clear();
    for (const auto& module : modules)
    {
        PrimaryModule primary;
        primary.importPath = module.importPath;
        primary.packageName = module.packageName;
        primary.sourcePath = module.path;
        outPrimaryModules.add(primary);
    }
    outPrimaryModules.sort([](const PrimaryModule& left, const PrimaryModule& right)
                           { return left.importPath < right.importPath; });
}

static void _collectExportedSourceFiles(
    const List<ExportedSourceFile>& files,
    List<ExportedSourceFile>& outSourceFiles)
{
    outSourceFiles = files;
    outSourceFiles.sort([](const ExportedSourceFile& left, const ExportedSourceFile& right)
                        { return left.relativePath < right.relativePath; });
}

/// Find the primary module file whose same-named directory contains `relativePath`.
static String _findOwningModule(
    const String& relativePath,
    const List<String>& normalizedSourcePaths)
{
    String normalized = _normalizePath(relativePath);
    List<UnownedStringSlice> components;
    Path::split(normalized.getUnownedSlice(), components);
    String prefix;
    for (Index i = 0; i + 1 < components.getCount(); ++i)
    {
        String candidate = prefix.getLength()
                               ? Path::combine(prefix, String(components[i]) + ".slang")
                               : String(components[i]) + ".slang";
        candidate = _normalizePath(candidate);
        if (normalizedSourcePaths.contains(candidate))
            return candidate;
        prefix = prefix.getLength() ? Path::combine(prefix, components[i]) : String(components[i]);
    }
    return String();
}

static SlangResult _addModule(
    const String& packageName,
    const String& exportRoot,
    const String& relativePath,
    List<ModuleLocation>& ioModules,
    String& outError,
    bool validateSourceLayout)
{
    String normalizedImport = _normalizePath(Path::getPathWithoutExt(relativePath));
    StringBuilder canonicalImportBuilder;
    for (auto c : normalizedImport.getUnownedSlice())
        canonicalImportBuilder.append(c == '-' ? '_' : c);
    String canonicalImport = canonicalImportBuilder.produceString();
    String fullPath = Path::combine(exportRoot, relativePath);
    for (const auto& existing : ioModules)
    {
        if (validateSourceLayout &&
            existing.canonicalImport.getUnownedSlice().caseInsensitiveEquals(
                canonicalImport.getUnownedSlice()))
        {
            if (existing.canonicalImport == canonicalImport)
            {
                outError = String("Module '") + canonicalImport +
                           "' is exported by both package '" + existing.packageName + "' (" +
                           existing.path + ") and package '" + packageName + "' (" + fullPath +
                           ").";
            }
            else
            {
                outError = String("Module '") + canonicalImport + "' conflicts with module '" +
                           existing.canonicalImport +
                           "' on a case-insensitive filesystem; they are exported by package '" +
                           existing.packageName + "' (" + existing.path + ") and package '" +
                           packageName + "' (" + fullPath + ").";
            }
            return SLANG_FAIL;
        }
    }
    ModuleLocation location;
    location.canonicalImport = canonicalImport;
    location.importPath = normalizedImport;
    location.packageName = packageName;
    location.path = fullPath;
    ioModules.add(location);
    return SLANG_OK;
}

/// Validate declaration placement in one export and add each primary module to the flat index.
static SlangResult _validateExport(
    const String& packageName,
    const String& exportRoot,
    List<ModuleLocation>& ioModules,
    List<ExportedSourceFile>& ioSourceFiles,
    String& outError,
    bool validateSourceLayout)
{
    SlangPathType exportType;
    if (SLANG_FAILED(Path::getPathType(exportRoot, &exportType)) ||
        exportType != SLANG_PATH_TYPE_DIRECTORY)
    {
        outError = String("Package export is not a directory: ") + exportRoot;
        return SLANG_FAIL;
    }

    SourceFileCollection collection;
    if (SLANG_FAILED(Path::getCanonical(exportRoot, collection.canonicalRoot)))
    {
        outError = String("Cannot canonicalize package export: ") + exportRoot;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(_collectSourceFilesRec(exportRoot, String(), collection, outError));
    List<String> normalizedSourcePaths;
    for (const auto& relativePath : collection.relativePaths)
        normalizedSourcePaths.add(_normalizePath(relativePath));

    for (const auto& relativePath : collection.relativePaths)
    {
        String owner = _findOwningModule(relativePath, normalizedSourcePaths);
        bool isPrimary = owner.getLength() == 0;
        String fullPath = Path::combine(exportRoot, relativePath);
        if (validateSourceLayout)
        {
            String expectedName = _canonicalModuleName(isPrimary ? relativePath : owner);
            ModuleHeaderKind kind;
            String declaredName;
            SLANG_RETURN_ON_FAIL(_readModuleHeader(fullPath, kind, declaredName, outError));
            if (isPrimary && kind != ModuleHeaderKind::Module)
            {
                outError = String("Primary module file must start with 'module ") + expectedName +
                           ";': " + fullPath;
                return SLANG_FAIL;
            }
            if (!isPrimary && kind != ModuleHeaderKind::Implementing)
            {
                outError = String("Companion module file must start with 'implementing ") +
                           expectedName + ";': " + fullPath;
                return SLANG_FAIL;
            }
            if (declaredName != expectedName)
            {
                outError = String("Module declaration name '") + declaredName +
                           "' does not match expected name '" + expectedName + "': " + fullPath;
                return SLANG_FAIL;
            }
        }
        ExportedSourceFile sourceFile;
        sourceFile.relativePath = _normalizePath(relativePath);
        sourceFile.packageName = packageName;
        sourceFile.sourcePath = fullPath;
        ioSourceFiles.add(sourceFile);
        if (isPrimary)
        {
            SLANG_RETURN_ON_FAIL(_addModule(
                packageName,
                exportRoot,
                relativePath,
                ioModules,
                outError,
                validateSourceLayout));
        }
    }
    return SLANG_OK;
}

static SlangResult _validateLicenseFiles(
    const String& packageRoot,
    const Manifest& manifest,
    String& outError)
{
    if (manifest.licenseFiles.getCount() == 0)
    {
        outError = String("Package manifest must list at least one file in 'license_files': ") +
                   packageRoot;
        return SLANG_FAIL;
    }
    String canonicalPackageRoot;
    if (SLANG_FAILED(Path::getCanonical(packageRoot, canonicalPackageRoot)))
    {
        outError = String("Cannot canonicalize package root: ") + packageRoot;
        return SLANG_FAIL;
    }
    for (const auto& relativePath : manifest.licenseFiles)
    {
        String path = Path::combine(packageRoot, relativePath);
        SlangPathType type;
        if (SLANG_FAILED(Path::getPathType(path, &type)) || type != SLANG_PATH_TYPE_FILE)
        {
            outError = String("Package license file does not exist: ") + path;
            return SLANG_FAIL;
        }
        String canonicalPath;
        if (SLANG_FAILED(Path::getCanonical(path, canonicalPath)) ||
            !isCanonicalPathWithin(canonicalPackageRoot, canonicalPath))
        {
            outError = String("Package license file escapes its package: ") + path;
            return SLANG_FAIL;
        }
        String contents;
        if (SLANG_FAILED(File::readAllText(path, contents)) || contents.trim().getLength() == 0)
        {
            outError = String("Package license file is empty or unreadable: ") + path;
            return SLANG_FAIL;
        }
        if (contents.getUnownedSlice().indexOf(UnownedStringSlice(kLicensePlaceholder).trim()) >= 0)
        {
            outError =
                String("Replace the generated license placeholder before validating: ") + path;
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

static SlangResult _validatePackageTree(
    const String& packageRoot,
    const Manifest& manifest,
    List<ModuleLocation>& ioModules,
    List<ExportedSourceFile>& ioSourceFiles,
    String& outError,
    bool validateLicenses,
    bool validateSourceLayout)
{
    if (validateLicenses)
        SLANG_RETURN_ON_FAIL(_validateLicenseFiles(packageRoot, manifest, outError));
    if (manifest.exports.getCount() == 0)
    {
        outError =
            String("Package manifest must export at least one source directory: ") + packageRoot;
        return SLANG_FAIL;
    }
    String canonicalPackageRoot;
    if (SLANG_FAILED(Path::getCanonical(packageRoot, canonicalPackageRoot)))
    {
        outError = String("Cannot canonicalize package root: ") + packageRoot;
        return SLANG_FAIL;
    }
    for (const auto& relativeExport : manifest.exports)
    {
        String exportRoot = Path::combine(packageRoot, relativeExport);
        String canonicalExportRoot;
        if (SLANG_FAILED(Path::getCanonical(exportRoot, canonicalExportRoot)) ||
            !isCanonicalPathWithin(canonicalPackageRoot, canonicalExportRoot))
        {
            outError = String("Package export escapes its package: ") + exportRoot;
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(_validateExport(
            manifest.name,
            exportRoot,
            ioModules,
            ioSourceFiles,
            outError,
            validateSourceLayout));
    }
    return SLANG_OK;
}

/// Reject local dependency edges that would refer outside a shared copy of this package.
static SlangResult _validatePublishablePathDependencies(
    const String& packageRoot,
    const Manifest& manifest,
    String& outError)
{
    String canonicalPackageRoot;
    if (SLANG_FAILED(Path::getCanonical(packageRoot, canonicalPackageRoot)))
    {
        outError = String("Cannot canonicalize package root: ") + packageRoot;
        return SLANG_FAIL;
    }
    for (const auto& dependency : manifest.dependencies)
    {
        if (!dependency.path.getLength())
            continue;
        String path = Path::combine(packageRoot, dependency.path);
        String canonicalPath;
        if (SLANG_FAILED(Path::getCanonical(path, canonicalPath)))
        {
            outError = String("Path dependency does not exist: ") + dependency.name + " at " + path;
            return SLANG_FAIL;
        }
        if (!isCanonicalPathWithin(canonicalPackageRoot, canonicalPath))
        {
            outError = String("Path dependency '") + dependency.name +
                       "' escapes the package and cannot be published: " + dependency.path;
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

static SlangResult _readMaterializedManifest(
    const String& projectRoot,
    const String& depsDirectory,
    const LockedPackage& package,
    const List<LocalPackage>& localPackages,
    String& outPackageRoot,
    Manifest& outManifest,
    String& outError)
{
    Index localIndex = findActiveLocalPackageIndex(localPackages, package.name);
    if (localIndex >= 0)
    {
        if (package.path.getLength() && package.path != localPackages[localIndex].path)
        {
            outError = String("Locked path for package '") + package.name +
                       "' does not match slang-pkg-workspace.json.";
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(
            getLocalPackageRoot(projectRoot, localPackages[localIndex], outPackageRoot, outError));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(getLockedPackageRoot(
            projectRoot,
            depsDirectory,
            package,
            localPackages,
            outPackageRoot,
            outError));
    }
    if (SLANG_FAILED(
            readManifest(Path::combine(outPackageRoot, kManifestFileName), outManifest, outError)))
    {
        outError = String("Cannot validate materialized package manifest '") + package.name +
                   "'. Run 'slang package fetch'. " + outError;
        return SLANG_FAIL;
    }
    return validateLockedPackageManifest(package, outManifest, outError);
}

/// Return whether a Git checkout is already sitting at exactly `commit`.
///
/// Stage 1 of validation runs on every fetch, update, and validate. When `deps/NAME` already holds
/// the locked revision, its committed manifest is the one the legal graph needs, so reading it
/// there avoids the `git fetch` that `ensureRepository` performs against the tool-owned cache. A
/// clean workspace therefore validates without touching the network. During `update` the checkout
/// still holds the previous commit, so this returns false and the cache answers for the newly
/// selected revision instead.
static bool _isCheckoutAtCommit(const String& checkoutPath, const String& commit)
{
    if (!commit.getLength())
        return false;
    String headCommit;
    String error;
    if (SLANG_FAILED(getRepositoryHeadCommit(checkoutPath, headCommit, error)))
        return false;
    return headCommit == commit;
}

struct ResolvedPackageLoad
{
    Manifest manifest;
    String packageRoot;
    String gitRepositoryPath;
    String gitRevision;
    String gitRelativeRoot;
};

/// Load the manifest the legal graph needs for one lock row without requiring `deps/`.
///
/// Consider this example: `update` selects `noise@v1.1.0` and `noise` vendors `vendor/math` as a
/// path dependency. Stage 1 must check those identities before it clears search paths or clones
/// `deps/noise`. A Git pin is therefore read at its locked commit from whichever repository
/// already has that revision, preferring an existing `deps/NAME` checkout so a clean workspace
/// needs no network, and falling back to `.slang/cache` when the checkout holds an older commit.
/// Nested path packages of that Git tree are read out of the same repository with `git show`.
/// Overrides and ordinary path packages still use the real directory.
static SlangResult _loadResolvedPackage(
    const String& projectRoot,
    const String& depsDirectory,
    const LockedPackage& package,
    const List<LocalPackage>& localPackages,
    const ResolvedPackageLoad& parent,
    const Dependency& incoming,
    ResolvedPackageLoad& out,
    String& outError,
    bool allowRemoteGit)
{
    out = ResolvedPackageLoad();
    Index localIndex = findActiveLocalPackageIndex(localPackages, package.name);
    if (localIndex >= 0)
    {
        if (package.path.getLength() && package.path != localPackages[localIndex].path)
        {
            outError = String("Locked path for package '") + package.name +
                       "' does not match slang-pkg-workspace.json.";
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(
            getLocalPackageRoot(projectRoot, localPackages[localIndex], out.packageRoot, outError));
        if (SLANG_FAILED(readManifest(
                Path::combine(out.packageRoot, kManifestFileName),
                out.manifest,
                outError)))
        {
            outError =
                String("Cannot read local package manifest '") + package.name + "': " + outError;
            return SLANG_FAIL;
        }
        return validateLockedPackageManifest(package, out.manifest, outError);
    }

    if (incoming.path.getLength() && parent.gitRevision.getLength())
    {
        String gitRelativeRoot =
            Path::simplify(Path::combine(parent.gitRelativeRoot, incoming.path));
        if (Path::isAbsolute(gitRelativeRoot) || pathStartsWithParentComponent(gitRelativeRoot))
        {
            outError = String("Path dependency '") + package.name +
                       "' escapes its Git package checkout: " + incoming.path;
            return SLANG_FAIL;
        }
        String manifestPath = gitRelativeRoot.getLength()
                                  ? Path::combine(gitRelativeRoot, kManifestFileName)
                                  : kManifestFileName;
        String manifestText;
        SLANG_RETURN_ON_FAIL(readFileAtRevision(
            parent.gitRepositoryPath,
            parent.gitRevision,
            manifestPath,
            manifestText,
            outError));
        String sourceName =
            parent.gitRepositoryPath + "@" + parent.gitRevision + ":" + manifestPath;
        SLANG_RETURN_ON_FAIL(readManifestText(sourceName, manifestText, out.manifest, outError));
        out.packageRoot = Path::combine(projectRoot, package.path);
        out.gitRepositoryPath = parent.gitRepositoryPath;
        out.gitRevision = parent.gitRevision;
        out.gitRelativeRoot = gitRelativeRoot;
        return validateLockedPackageManifest(package, out.manifest, outError);
    }

    if (isPathOnlyLockedPackage(package) || !isGitBackedLockedPackage(package))
    {
        SLANG_RETURN_ON_FAIL(getLockedPackageRoot(
            projectRoot,
            depsDirectory,
            package,
            localPackages,
            out.packageRoot,
            outError));
        if (SLANG_FAILED(readManifest(
                Path::combine(out.packageRoot, kManifestFileName),
                out.manifest,
                outError)))
        {
            outError = String("Cannot validate materialized package manifest '") + package.name +
                       "'. Run 'slang package fetch'. " + outError;
            return SLANG_FAIL;
        }
        return validateLockedPackageManifest(package, out.manifest, outError);
    }

    // Read the committed manifest out of whichever Git source already has the locked revision,
    // rather than the file in the working tree, so local edits under `deps/` cannot change what
    // the legal graph sees.
    String depsRoot = Path::combine(projectRoot, depsDirectory, package.name);
    String cachePath = Path::combine(Path::combine(projectRoot, ".slang", "cache"), package.name);
    String gitError;
    String manifestText;
    String gitRepositoryPath;
    if (_isCheckoutAtCommit(depsRoot, package.commit) && SLANG_SUCCEEDED(readFileAtRevision(
                                                             depsRoot,
                                                             package.commit,
                                                             kManifestFileName,
                                                             manifestText,
                                                             gitError)))
    {
        gitRepositoryPath = depsRoot;
    }
    else if (
        allowRemoteGit && package.commit.getLength() &&
        SLANG_SUCCEEDED(ensureRepository(projectRoot, package.git, cachePath, gitError)) &&
        SLANG_SUCCEEDED(readFileAtRevision(
            cachePath,
            package.commit,
            kManifestFileName,
            manifestText,
            gitError)))
    {
        gitRepositoryPath = cachePath;
    }
    if (gitRepositoryPath.getLength())
    {
        String sourceName = package.git + "@" + package.ref + ":" + kManifestFileName;
        SLANG_RETURN_ON_FAIL(readManifestText(sourceName, manifestText, out.manifest, outError));
        out.packageRoot = depsRoot;
        out.gitRepositoryPath = gitRepositoryPath;
        out.gitRevision = package.commit;
        return validateLockedPackageManifest(package, out.manifest, outError);
    }

    // A lock can name an origin that cannot be cloned from here, such as the `memory:` URLs the
    // unit tests pin, while a materialized tree is still present on disk. Read that directory
    // when no Git source can supply the locked revision.
    if (SLANG_FAILED(getLockedPackageRoot(
            projectRoot,
            depsDirectory,
            package,
            localPackages,
            out.packageRoot,
            outError)))
    {
        if (gitError.getLength())
            outError = gitError;
        return SLANG_FAIL;
    }
    if (SLANG_FAILED(readManifest(
            Path::combine(out.packageRoot, kManifestFileName),
            out.manifest,
            outError)))
    {
        outError = gitError.getLength()
                       ? gitError
                       : String("Cannot validate materialized package manifest '") + package.name +
                             "'. Run 'slang package fetch'. " + outError;
        return SLANG_FAIL;
    }
    return validateLockedPackageManifest(package, out.manifest, outError);
}

SlangResult loadLockedPackageGraphManifest(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockedPackage& package,
    const List<LocalPackage>& localPackages,
    Manifest& outManifest,
    String& outError,
    bool allowRemoteGit)
{
    ResolvedPackageLoad parent;
    Dependency incoming;
    ResolvedPackageLoad loaded;
    SLANG_RETURN_ON_FAIL(_loadResolvedPackage(
        projectRoot,
        getWorkspaceDepsDirectory(rootManifest),
        package,
        localPackages,
        parent,
        incoming,
        loaded,
        outError,
        allowRemoteGit));
    outManifest = loaded.manifest;
    return SLANG_OK;
}

SlangResult validatePublishablePackage(
    const String& packageRoot,
    const Manifest& manifest,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(_validatePublishablePathDependencies(packageRoot, manifest, outError));
    List<ModuleLocation> modules;
    List<ExportedSourceFile> sourceFiles;
    return _validatePackageTree(packageRoot, manifest, modules, sourceFiles, outError, true, true);
}

SlangResult validateLegalResolvedProject(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
    List<String>* outWarnings,
    bool allowRemoteGit)
{
    SLANG_RETURN_ON_FAIL(validateLockedWorkspaceExclusions(rootManifest, lock, outError));
    List<ResolvedPackageLoad> loadedPackages;
    loadedPackages.setCount(lock.packages.getCount());
    List<bool> loaded;
    loaded.setCount(lock.packages.getCount());
    for (auto& value : loaded)
        value = false;
    for (const auto& localPackage : localPackages)
    {
        if (!isActiveLocalPackage(localPackage))
            continue;
        Index packageIndex = findLockedPackageIndex(lock, localPackage.name);
        if (packageIndex < 0)
        {
            if (isInPlaceLocalPackage(rootManifest, localPackage))
                continue;
            outError =
                String("Registered local package is not present in the lock: ") + localPackage.name;
            return SLANG_FAIL;
        }
        const LockedPackage& package = lock.packages[packageIndex];
        if (localPackage.as.getLength() && package.version != localPackage.as)
        {
            outError = String("Locked version for local override '") + package.name +
                       "' does not match slang-pkg-workspace.json. Run 'slang package update'.";
            return SLANG_FAIL;
        }
    }

    ResolvedPackageLoad workspaceLoad;
    workspaceLoad.packageRoot = projectRoot;

    List<bool> reachable;
    reachable.setCount(lock.packages.getCount());
    for (auto& value : reachable)
        value = false;
    List<Index> pending;
    for (const auto& dependency : rootManifest.dependencies)
    {
        Index index;
        SLANG_RETURN_ON_FAIL(validateLockedDependency(dependency, lock, index, outError));
        SLANG_RETURN_ON_FAIL(validateLockedPathDependency(
            projectRoot,
            projectRoot,
            rootManifest.name,
            dependency,
            lock.packages[index],
            outError,
            outWarnings));
        if (isTrustedLockSelection(dependency, lock.packages[index]) && !reachable[index])
        {
            reachable[index] = true;
            pending.add(index);
            if (!loaded[index])
            {
                SLANG_RETURN_ON_FAIL(_loadResolvedPackage(
                    projectRoot,
                    getWorkspaceDepsDirectory(rootManifest),
                    lock.packages[index],
                    localPackages,
                    workspaceLoad,
                    dependency,
                    loadedPackages[index],
                    outError,
                    allowRemoteGit));
                loaded[index] = true;
            }
        }
    }
    for (Index pendingIndex = 0; pendingIndex < pending.getCount(); ++pendingIndex)
    {
        Index index = pending[pendingIndex];
        const Manifest& manifest = loadedPackages[index].manifest;
        for (const auto& dependency : manifest.dependencies)
        {
            Index dependencyIndex;
            SLANG_RETURN_ON_FAIL(
                validateLockedDependency(dependency, lock, dependencyIndex, outError));
            SLANG_RETURN_ON_FAIL(validateLockedPathDependency(
                projectRoot,
                loadedPackages[index].packageRoot,
                manifest.name,
                dependency,
                lock.packages[dependencyIndex],
                outError,
                outWarnings));
            if (isTrustedLockSelection(dependency, lock.packages[dependencyIndex]) &&
                !reachable[dependencyIndex])
            {
                reachable[dependencyIndex] = true;
                pending.add(dependencyIndex);
                if (!loaded[dependencyIndex])
                {
                    SLANG_RETURN_ON_FAIL(_loadResolvedPackage(
                        projectRoot,
                        getWorkspaceDepsDirectory(rootManifest),
                        lock.packages[dependencyIndex],
                        localPackages,
                        loadedPackages[index],
                        dependency,
                        loadedPackages[dependencyIndex],
                        outError,
                        allowRemoteGit));
                    loaded[dependencyIndex] = true;
                }
            }
        }
    }
    SLANG_RETURN_ON_FAIL(requireAllLockPackagesTrusted(lock, reachable, outError));
    List<ToolchainConstraint> toolchainConstraints;
    addSlangToolchainConstraint(rootManifest, toolchainConstraints);
    for (Index i = 0; i < lock.packages.getCount(); ++i)
    {
        if (loaded[i])
            addSlangToolchainConstraint(loadedPackages[i].manifest, toolchainConstraints);
        if (loaded[i] && outWarnings)
        {
            addUnadoptedWorkspaceExclusionWarnings(
                rootManifest,
                lock.packages[i].name,
                loadedPackages[i].manifest,
                outWarnings);
        }
    }
    return selectSlangToolchain(toolchainConstraints, outError);
}

SlangResult validateBuildableResolvedProject(
    const String& projectRoot,
    const Manifest& rootManifest,
    const LockFile& lock,
    const List<LocalPackage>& localPackages,
    String& outError,
    List<String>* outWarnings,
    List<PrimaryModule>* outPrimaryModules,
    List<ExportedSourceFile>* outSourceFiles,
    bool skipSourceValidation,
    bool assumeLegalGraph)
{
    if (!assumeLegalGraph)
    {
        SLANG_RETURN_ON_FAIL(validateLegalResolvedProject(
            projectRoot,
            rootManifest,
            lock,
            localPackages,
            outError,
            outWarnings));
    }

    List<ModuleLocation> modules;
    List<ExportedSourceFile> sourceFiles;
    SLANG_RETURN_ON_FAIL(_validatePackageTree(
        projectRoot,
        rootManifest,
        modules,
        sourceFiles,
        outError,
        false,
        !skipSourceValidation));
    for (const auto& package : lock.packages)
    {
        String packageRoot;
        Manifest manifest;
        SLANG_RETURN_ON_FAIL(_readMaterializedManifest(
            projectRoot,
            getWorkspaceDepsDirectory(rootManifest),
            package,
            localPackages,
            packageRoot,
            manifest,
            outError));
        SLANG_RETURN_ON_FAIL(_validatePackageTree(
            packageRoot,
            manifest,
            modules,
            sourceFiles,
            outError,
            false,
            !skipSourceValidation));
    }
    if (outPrimaryModules)
        _collectPrimaryModules(modules, *outPrimaryModules);
    if (outSourceFiles)
        _collectExportedSourceFiles(sourceFiles, *outSourceFiles);
    return SLANG_OK;
}

SlangResult validateBuildableProject(
    const String& projectRoot,
    String& outError,
    List<String>* outWarnings,
    List<PrimaryModule>* outPrimaryModules,
    List<ExportedSourceFile>* outSourceFiles,
    bool skipSourceValidation)
{
    Manifest rootManifest;
    SLANG_RETURN_ON_FAIL(
        readManifest(Path::combine(projectRoot, kManifestFileName), rootManifest, outError));

    List<LocalPackage> localPackages;
    SLANG_RETURN_ON_FAIL(readProjectLocalPackages(projectRoot, localPackages, outError));
    String lockPath = Path::combine(projectRoot, kLockFileName);
    LockFile lock;
    if (File::exists(lockPath))
    {
        SLANG_RETURN_ON_FAIL(readLockFile(lockPath, lock, outError));
    }
    else if (rootManifest.dependencies.getCount() || localPackages.getCount())
    {
        outError = localPackages.getCount()
                       ? "Registered local packages require slang-pkg-lock.json."
                       : "Package dependencies require slang-pkg-lock.json.";
        return SLANG_FAIL;
    }

    return validateBuildableResolvedProject(
        projectRoot,
        rootManifest,
        lock,
        localPackages,
        outError,
        outWarnings,
        outPrimaryModules,
        outSourceFiles,
        skipSourceValidation);
}

} // namespace PackageTool
} // namespace Slang
