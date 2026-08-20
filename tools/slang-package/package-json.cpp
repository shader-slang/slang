// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-json.h"

#include "compiler-core/slang-json-lexer.h"
#include "compiler-core/slang-json-parser.h"
#include "compiler-core/slang-json-value.h"
#include "compiler-core/slang-source-loc.h"
#include "core/slang-io.h"

namespace Slang
{
namespace PackageTool
{

struct ParsedJSON
{
    SourceManager sourceManager;
    RefPtr<JSONContainer> container;
    JSONValue root;

    ParsedJSON()
    {
        sourceManager.initialize(nullptr, nullptr);
        container = new JSONContainer(&sourceManager);
    }
};

static SlangResult _parseJSONText(
    const String& sourceName,
    const String& text,
    ParsedJSON& out,
    String& outError)
{
    DiagnosticSink sink(&out.sourceManager, nullptr);
    SourceFile* sourceFile =
        out.sourceManager.createSourceFileWithString(PathInfo::makePath(sourceName), text);
    SourceView* sourceView = out.sourceManager.createSourceView(sourceFile, nullptr, SourceLoc());
    JSONLexer lexer;
    lexer.init(sourceView, &sink);
    JSONBuilder builder(out.container);
    JSONParser parser;
    if (SLANG_FAILED(parser.parse(&lexer, sourceView, &builder, &sink)))
    {
        ComPtr<ISlangBlob> diagnostics;
        sink.getBlobIfNeeded(diagnostics.writeRef());
        outError = String("Invalid JSON in file: ") + sourceName;
        if (diagnostics && diagnostics->getBufferSize() != 0)
        {
            const char* begin = (const char*)diagnostics->getBufferPointer();
            outError = outError + "\n" + String(begin, begin + diagnostics->getBufferSize());
        }
        return SLANG_FAIL;
    }
    out.root = builder.getRootValue();
    if (out.root.getKind() != JSONValue::Kind::Object)
    {
        outError = String("The JSON root must be an object: ") + sourceName;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _parseJSON(const String& path, ParsedJSON& out, String& outError)
{
    String text;
    if (SLANG_FAILED(File::readAllText(path, text)))
    {
        outError = String("Cannot read JSON file: ") + path;
        return SLANG_FAIL;
    }
    return _parseJSONText(path, text, out, outError);
}

static JSONValue _find(JSONContainer* container, const JSONValue& object, const char* key)
{
    JSONKey jsonKey = container->findKey(UnownedStringSlice(key));
    return jsonKey ? container->findObjectValue(object, jsonKey) : JSONValue::makeInvalid();
}

static SlangResult _readRequiredString(
    JSONContainer* container,
    const JSONValue& object,
    const char* key,
    String& outValue,
    String& outError)
{
    JSONValue value = _find(container, object, key);
    if (value.getKind() != JSONValue::Kind::String)
    {
        outError = String("Required field '") + key + "' must be a string.";
        return SLANG_FAIL;
    }
    outValue = container->getString(value);
    if (outValue.getLength() == 0)
    {
        outError = String("Required field '") + key + "' cannot be empty.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

bool isValidPackageName(const String& name)
{
    if (name.getLength() == 0)
        return false;
    for (auto c : name.getUnownedSlice())
    {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
              c == '-' || c == '_' || c == '.'))
            return false;
    }
    return name != "." && name != "..";
}

static bool _isSafeRelativePath(const String& path)
{
    if (path.getLength() == 0 || Path::isAbsolute(path))
        return false;
    for (auto c : path.getUnownedSlice())
    {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
              c == '-' || c == '_' || c == '.' || c == '/' || c == '\\'))
            return false;
    }
    List<UnownedStringSlice> components;
    Path::split(path.getUnownedSlice(), components);
    for (auto component : components)
    {
        if (component == "..")
            return false;
    }
    return true;
}

static bool _isSafeGitLocation(const String& location)
{
    if (location.getLength() == 0 || location.getBuffer()[0] == '-' ||
        location.getUnownedSlice().indexOf(UnownedStringSlice("::")) != -1)
        return false;
    for (auto c : location.getUnownedSlice())
    {
        if (c <= ' ' || c == '"' || c == '\'')
            return false;
    }
    return true;
}

static bool _isCommitHash(const String& commit)
{
    if (commit.getLength() != 40 && commit.getLength() != 64)
        return false;
    for (auto c : commit.getUnownedSlice())
    {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            return false;
    }
    return true;
}

static SlangResult _readStringArray(
    JSONContainer* container,
    const JSONValue& object,
    const char* key,
    List<String>& outValues,
    String& outError)
{
    JSONValue value = _find(container, object, key);
    if (value.getKind() != JSONValue::Kind::Array)
    {
        outError = String("Required field '") + key + "' must be an array.";
        return SLANG_FAIL;
    }
    outValues.clear();
    for (auto item : container->getArray(value))
    {
        if (item.getKind() != JSONValue::Kind::String)
        {
            outError = String("Every entry in '") + key + "' must be a string.";
            return SLANG_FAIL;
        }
        String path = container->getString(item);
        if (!_isSafeRelativePath(path))
        {
            outError = String("Export paths must be non-empty relative paths: ") + path;
            return SLANG_FAIL;
        }
        outValues.add(path);
    }
    return SLANG_OK;
}

static SlangResult _readDependencies(
    JSONContainer* container,
    const JSONValue& root,
    List<Dependency>& outDependencies,
    String& outError)
{
    JSONValue dependencies = _find(container, root, "dependencies");
    if (!dependencies.isValid())
    {
        outDependencies.clear();
        return SLANG_OK;
    }
    if (dependencies.getKind() != JSONValue::Kind::Object)
    {
        outError = "Field 'dependencies' must be an object.";
        return SLANG_FAIL;
    }

    outDependencies.clear();
    for (auto pair : container->getObject(dependencies))
    {
        Dependency dependency;
        dependency.name = container->getStringFromKey(pair.key);
        if (!isValidPackageName(dependency.name) || pair.value.getKind() != JSONValue::Kind::Object)
        {
            outError = String("Invalid dependency entry: ") + dependency.name;
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(
            _readRequiredString(container, pair.value, "git", dependency.git, outError));
        if (!_isSafeGitLocation(dependency.git))
        {
            outError = String("Dependency has an unsafe Git location: ") + dependency.name;
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(
            _readRequiredString(container, pair.value, "tag", dependency.tag, outError));
        VersionConstraint ignored;
        SLANG_RETURN_ON_FAIL(parseVersionConstraint(dependency.tag, ignored, outError));
        for (const auto& existing : outDependencies)
        {
            if (existing.name == dependency.name)
            {
                outError = String("Duplicate dependency entry: ") + dependency.name;
                return SLANG_FAIL;
            }
        }
        outDependencies.add(dependency);
    }
    return SLANG_OK;
}

static SlangResult _readManifest(ParsedJSON& json, Manifest& outManifest, String& outError)
{
    outManifest = Manifest();
    SLANG_RETURN_ON_FAIL(
        _readRequiredString(json.container, json.root, "name", outManifest.name, outError));
    if (!isValidPackageName(outManifest.name))
    {
        outError = String("Invalid package name: ") + outManifest.name;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        _readRequiredString(json.container, json.root, "version", outManifest.version, outError));
    SemanticVersion manifestVersion;
    if (SLANG_FAILED(
            SemanticVersion::parse(outManifest.version.getUnownedSlice(), manifestVersion)))
    {
        outError = String("Invalid package version: ") + outManifest.version;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        _readStringArray(json.container, json.root, "exports", outManifest.exports, outError));
    SLANG_RETURN_ON_FAIL(
        _readDependencies(json.container, json.root, outManifest.dependencies, outError));
    return SLANG_OK;
}

SlangResult readManifest(const String& path, Manifest& outManifest, String& outError)
{
    ParsedJSON json;
    SLANG_RETURN_ON_FAIL(_parseJSON(path, json, outError));
    return _readManifest(json, outManifest, outError);
}

SlangResult readManifestText(
    const String& sourceName,
    const String& text,
    Manifest& outManifest,
    String& outError)
{
    ParsedJSON json;
    SLANG_RETURN_ON_FAIL(_parseJSONText(sourceName, text, json, outError));
    return _readManifest(json, outManifest, outError);
}

static void _writeStringArray(JSONWriter& writer, const List<String>& values)
{
    writer.startArray(SourceLoc());
    for (const auto& value : values)
        writer.addStringValue(value.getUnownedSlice(), SourceLoc());
    writer.endArray(SourceLoc());
}

static void _writeKey(JSONWriter& writer, const char* key)
{
    writer.addUnquotedKey(UnownedStringSlice(key), SourceLoc());
}

static void _writeDependency(JSONWriter& writer, const Dependency& dependency)
{
    writer.addUnquotedKey(dependency.name.getUnownedSlice(), SourceLoc());
    writer.startObject(SourceLoc());
    _writeKey(writer, "git");
    writer.addStringValue(dependency.git.getUnownedSlice(), SourceLoc());
    _writeKey(writer, "tag");
    writer.addStringValue(dependency.tag.getUnownedSlice(), SourceLoc());
    writer.endObject(SourceLoc());
}

SlangResult writeManifest(const String& path, const Manifest& manifest, String& outError)
{
    JSONWriter writer(JSONWriter::IndentationStyle::Allman);
    writer.startObject(SourceLoc());
    _writeKey(writer, "name");
    writer.addStringValue(manifest.name.getUnownedSlice(), SourceLoc());
    _writeKey(writer, "version");
    writer.addStringValue(manifest.version.getUnownedSlice(), SourceLoc());
    _writeKey(writer, "exports");
    _writeStringArray(writer, manifest.exports);
    _writeKey(writer, "dependencies");
    writer.startObject(SourceLoc());
    for (const auto& dependency : manifest.dependencies)
        _writeDependency(writer, dependency);
    writer.endObject(SourceLoc());
    writer.endObject(SourceLoc());
    writer.getBuilder() << "\n";
    if (SLANG_FAILED(File::writeAllText(path, writer.getBuilder())))
    {
        outError = String("Cannot write manifest: ") + path;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _readLockedPackage(
    JSONContainer* container,
    const JSONKeyValue& pair,
    LockedPackage& outPackage,
    String& outError)
{
    outPackage = LockedPackage();
    outPackage.name = container->getStringFromKey(pair.key);
    if (!isValidPackageName(outPackage.name) || pair.value.getKind() != JSONValue::Kind::Object)
    {
        outError = String("Invalid locked package entry: ") + outPackage.name;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        _readRequiredString(container, pair.value, "git", outPackage.git, outError));
    if (!_isSafeGitLocation(outPackage.git))
    {
        outError = String("Locked package has an unsafe Git location: ") + outPackage.name;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        _readRequiredString(container, pair.value, "tag", outPackage.tag, outError));
    SemanticVersion ignored;
    if (SLANG_FAILED(parseReleaseTag(outPackage.tag, ignored)))
    {
        outError = String("Locked tag is not a release tag: ") + outPackage.tag;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        _readRequiredString(container, pair.value, "commit", outPackage.commit, outError));
    if (!_isCommitHash(outPackage.commit))
    {
        outError =
            String("Locked commit must be an exact hexadecimal object ID: ") + outPackage.name;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        _readStringArray(container, pair.value, "exports", outPackage.exports, outError));

    return SLANG_OK;
}

SlangResult readLockFile(const String& path, LockFile& outLock, String& outError)
{
    ParsedJSON json;
    SLANG_RETURN_ON_FAIL(_parseJSON(path, json, outError));
    outLock = LockFile();

    JSONValue lockVersion = _find(json.container, json.root, "lock_version");
    if (lockVersion.getKind() != JSONValue::Kind::Integer ||
        json.container->asInteger(lockVersion) != 1)
    {
        outError = "Field 'lock_version' must be the integer 1.";
        return SLANG_FAIL;
    }
    JSONValue packages = _find(json.container, json.root, "packages");
    if (packages.getKind() != JSONValue::Kind::Object)
    {
        outError = "Field 'packages' must be an object.";
        return SLANG_FAIL;
    }
    for (auto pair : json.container->getObject(packages))
    {
        LockedPackage package;
        SLANG_RETURN_ON_FAIL(_readLockedPackage(json.container, pair, package, outError));
        for (const auto& existing : outLock.packages)
        {
            if (existing.name == package.name)
            {
                outError = String("Duplicate locked package entry: ") + package.name;
                return SLANG_FAIL;
            }
        }
        outLock.packages.add(package);
    }
    return SLANG_OK;
}

SlangResult writeLockFile(const String& path, const LockFile& lock, String& outError)
{
    JSONWriter writer(JSONWriter::IndentationStyle::Allman);
    writer.startObject(SourceLoc());
    _writeKey(writer, "lock_version");
    writer.addIntegerValue(lock.lockVersion, SourceLoc());
    _writeKey(writer, "packages");
    writer.startObject(SourceLoc());
    for (const auto& package : lock.packages)
    {
        writer.addUnquotedKey(package.name.getUnownedSlice(), SourceLoc());
        writer.startObject(SourceLoc());
        _writeKey(writer, "git");
        writer.addStringValue(package.git.getUnownedSlice(), SourceLoc());
        _writeKey(writer, "tag");
        writer.addStringValue(package.tag.getUnownedSlice(), SourceLoc());
        _writeKey(writer, "commit");
        writer.addStringValue(package.commit.getUnownedSlice(), SourceLoc());
        _writeKey(writer, "exports");
        _writeStringArray(writer, package.exports);
        writer.endObject(SourceLoc());
    }
    writer.endObject(SourceLoc());
    writer.endObject(SourceLoc());
    writer.getBuilder() << "\n";
    if (SLANG_FAILED(File::writeAllText(path, writer.getBuilder())))
    {
        outError = String("Cannot write lock file: ") + path;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
