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

static SlangResult _readOptionalBool(
    JSONContainer* container,
    const JSONValue& object,
    const char* key,
    bool defaultValue,
    bool& outValue,
    String& outError)
{
    JSONValue value = _find(container, object, key);
    if (!value.isValid())
    {
        outValue = defaultValue;
        return SLANG_OK;
    }
    if (value.getKind() != JSONValue::Kind::Bool)
    {
        outError = String("Field '") + key + "' must be a boolean.";
        return SLANG_FAIL;
    }
    outValue = container->asBool(value);
    return SLANG_OK;
}

static SlangResult _readOptionalString(
    JSONContainer* container,
    const JSONValue& object,
    const char* key,
    String& outValue,
    String& outError)
{
    JSONValue value = _find(container, object, key);
    if (!value.isValid())
    {
        outValue = String();
        return SLANG_OK;
    }
    if (value.getKind() != JSONValue::Kind::String)
    {
        outError = String("Field '") + key + "' must be a string.";
        return SLANG_FAIL;
    }
    outValue = container->getString(value);
    if (outValue.getLength() == 0)
    {
        outError = String("Field '") + key + "' cannot be empty.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _requireSchemaVersion(
    JSONContainer* container,
    const JSONValue& root,
    const char* fileName,
    String& outError)
{
    JSONValue value = _find(container, root, "schema_version");
    if (!value.isValid())
    {
        outError = String("Field 'schema_version' is required in ") + fileName + ".";
        return SLANG_FAIL;
    }
    if (value.getKind() != JSONValue::Kind::Integer ||
        container->asInteger(value) != kSchemaVersion)
    {
        outError = String("Field 'schema_version' in ") + fileName + " must be the integer 1.";
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

/// Return whether either simplified relative workspace path contains the other.
static bool _workspacePathsOverlap(const String& left, const String& right)
{
    List<UnownedStringSlice> leftComponents;
    List<UnownedStringSlice> rightComponents;
    Path::split(left.getUnownedSlice(), leftComponents);
    Path::split(right.getUnownedSlice(), rightComponents);
    Index commonCount = leftComponents.getCount();
    if (rightComponents.getCount() < commonCount)
        commonCount = rightComponents.getCount();
    for (Index i = 0; i < commonCount; ++i)
    {
        if (leftComponents[i] != rightComponents[i])
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

static bool _isSafeGitRef(const String& ref)
{
    if (ref.getLength() == 0 || ref.getBuffer()[0] == '-')
        return false;
    for (auto c : ref.getUnownedSlice())
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

static bool _isSafeLocalPath(const String& path)
{
    if (path.getLength() == 0 || Path::isAbsolute(path))
        return false;
    for (auto c : path.getUnownedSlice())
    {
        if (c < ' ' || c == '"' || c == '\'')
            return false;
    }
    return true;
}

static SlangResult _readRelativePathArray(
    JSONContainer* container,
    const JSONValue& object,
    const char* key,
    List<String>& outValues,
    String& outError)
{
    JSONValue value = _find(container, object, key);
    if (!value.isValid())
    {
        outError = String("Missing required field '") + key + "'.";
        return SLANG_FAIL;
    }
    if (value.getKind() != JSONValue::Kind::Array)
    {
        outError = String("Field '") + key + "' must be an array.";
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
            outError = String("Entries in '") + key + "' must be non-empty relative paths: " + path;
            return SLANG_FAIL;
        }
        for (const auto& existing : outValues)
        {
            if (existing == path)
            {
                outError = String("Duplicate entry in '") + key + "': " + path;
                return SLANG_FAIL;
            }
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
        for (auto field : container->getObject(pair.value))
        {
            String key = container->getStringFromKey(field.key);
            if (key != "git" && key != "path" && key != "version" && key != "ref" && key != "as")
            {
                outError = String("Unknown field in dependency '") + dependency.name + "': " + key;
                return SLANG_FAIL;
            }
        }
        SLANG_RETURN_ON_FAIL(
            _readOptionalString(container, pair.value, "git", dependency.git, outError));
        SLANG_RETURN_ON_FAIL(
            _readOptionalString(container, pair.value, "path", dependency.path, outError));
        if ((dependency.git.getLength() != 0) == (dependency.path.getLength() != 0))
        {
            outError = String("Dependency must contain exactly one of 'git' or 'path': ") +
                       dependency.name;
            return SLANG_FAIL;
        }
        if (dependency.git.getLength() && !_isSafeGitLocation(dependency.git))
        {
            outError = String("Dependency has an unsafe Git location: ") + dependency.name;
            return SLANG_FAIL;
        }
        if (dependency.path.getLength() && !_isSafeLocalPath(dependency.path))
        {
            outError = String("Dependency path must be relative: ") + dependency.name;
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(
            _readOptionalString(container, pair.value, "version", dependency.version, outError));
        SLANG_RETURN_ON_FAIL(
            _readOptionalString(container, pair.value, "ref", dependency.ref, outError));
        SLANG_RETURN_ON_FAIL(
            _readOptionalString(container, pair.value, "as", dependency.as, outError));
        if (dependency.path.getLength())
        {
            if (dependency.version.getLength() || dependency.ref.getLength() ||
                !dependency.as.getLength())
            {
                outError =
                    String("Path dependency must contain 'path' and 'as' only: ") + dependency.name;
                return SLANG_FAIL;
            }
        }
        else
        {
            if ((dependency.ref.getLength() != 0) != (dependency.as.getLength() != 0))
            {
                outError = String("Git dependency '") + dependency.name +
                           "' must contain 'ref' and 'as' together.";
                return SLANG_FAIL;
            }
            if (dependency.ref.getLength() && !_isSafeGitRef(dependency.ref))
            {
                outError = String("Dependency has an unsafe Git ref: ") + dependency.name;
                return SLANG_FAIL;
            }
            if (!dependency.version.getLength() && !dependency.ref.getLength())
            {
                outError = String("Git dependency '") + dependency.name +
                           "' requires 'version' or 'ref' with 'as'.";
                return SLANG_FAIL;
            }
        }
        SemanticVersion providedVersion;
        if (dependency.as.getLength())
        {
            SLANG_RETURN_ON_FAIL(parseExactVersion(dependency.as, providedVersion, outError));
        }
        if (dependency.version.getLength())
        {
            VersionConstraint constraint;
            SLANG_RETURN_ON_FAIL(parseDependencyConstraint(dependency, constraint, outError));
            if (dependency.as.getLength() && !constraint.matches(providedVersion))
            {
                outError = String("Dependency '") + dependency.name + "' provides version " +
                           dependency.as + " via 'as', which does not satisfy 'version' " +
                           dependency.version + ".";
                return SLANG_FAIL;
            }
        }
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

static SlangResult _readVersionPolicy(
    JSONContainer* container,
    const JSONValue& entry,
    const char* fieldName,
    bool allowPackage,
    String& outVersion,
    String& outReason,
    String& outError)
{
    if (entry.getKind() != JSONValue::Kind::Object)
    {
        outError = String("Every entry in '") + fieldName + "' must be an object.";
        return SLANG_FAIL;
    }
    for (auto pair : container->getObject(entry))
    {
        String key = container->getStringFromKey(pair.key);
        if (key != "version" && key != "reason" && !(allowPackage && key == "package"))
        {
            outError = String("Unknown field in '") + fieldName + "': " + key;
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(_readRequiredString(container, entry, "version", outVersion, outError));
    SLANG_RETURN_ON_FAIL(_readRequiredString(container, entry, "reason", outReason, outError));
    VersionConstraint ignored;
    if (SLANG_FAILED(parseVersionConstraint(outVersion, ignored, outError)))
    {
        outError = String("Invalid version in '") + fieldName + "': " + outError;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _readRetractions(
    JSONContainer* container,
    const JSONValue& root,
    List<Retraction>& outRetractions,
    String& outError)
{
    JSONValue retractions = _find(container, root, "retractions");
    if (!retractions.isValid())
        return SLANG_OK;
    if (retractions.getKind() != JSONValue::Kind::Array)
    {
        outError = "Field 'retractions' must be an array.";
        return SLANG_FAIL;
    }
    for (auto item : container->getArray(retractions))
    {
        Retraction retraction;
        SLANG_RETURN_ON_FAIL(_readVersionPolicy(
            container,
            item,
            "retractions",
            false,
            retraction.version,
            retraction.reason,
            outError));
        for (const auto& existing : outRetractions)
        {
            if (existing.version == retraction.version)
            {
                outError = String("Duplicate retraction version: ") + retraction.version;
                return SLANG_FAIL;
            }
        }
        outRetractions.add(retraction);
    }
    return SLANG_OK;
}

static SlangResult _readExclusions(
    JSONContainer* container,
    const JSONValue& workspace,
    List<Exclusion>& outExclusions,
    String& outError)
{
    JSONValue exclusions = _find(container, workspace, "excludes");
    if (!exclusions.isValid())
        return SLANG_OK;
    if (exclusions.getKind() != JSONValue::Kind::Array)
    {
        outError = "Workspace 'excludes' must be an array.";
        return SLANG_FAIL;
    }
    for (auto item : container->getArray(exclusions))
    {
        Exclusion exclusion;
        SLANG_RETURN_ON_FAIL(_readVersionPolicy(
            container,
            item,
            "excludes",
            true,
            exclusion.version,
            exclusion.reason,
            outError));
        SLANG_RETURN_ON_FAIL(
            _readRequiredString(container, item, "package", exclusion.packageName, outError));
        if (!isValidPackageName(exclusion.packageName))
        {
            outError = String("Invalid excluded package name: ") + exclusion.packageName;
            return SLANG_FAIL;
        }
        for (const auto& existing : outExclusions)
        {
            if (existing.packageName == exclusion.packageName &&
                existing.version == exclusion.version)
            {
                outError = String("Duplicate workspace exclusion: ") + exclusion.packageName + " " +
                           exclusion.version;
                return SLANG_FAIL;
            }
        }
        outExclusions.add(exclusion);
    }
    return SLANG_OK;
}

static SlangResult _readWorkspace(
    JSONContainer* container,
    const JSONValue& root,
    WorkspaceSettings& outWorkspace,
    String& outError)
{
    JSONValue workspace = _find(container, root, "workspace");
    if (!workspace.isValid())
        return SLANG_OK;
    if (workspace.getKind() != JSONValue::Kind::Object)
    {
        outError = "Field 'workspace' must be an object.";
        return SLANG_FAIL;
    }
    for (auto pair : container->getObject(workspace))
    {
        String key = container->getStringFromKey(pair.key);
        if (key != "deps" && key != "build" && key != "excludes" && key != "bundle")
        {
            outError = String("Unknown field in 'workspace': ") + key;
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(
        _readOptionalString(container, workspace, "deps", outWorkspace.depsDirectory, outError));
    SLANG_RETURN_ON_FAIL(
        _readOptionalString(container, workspace, "build", outWorkspace.buildDirectory, outError));
    SLANG_RETURN_ON_FAIL(_readExclusions(container, workspace, outWorkspace.exclusions, outError));
    JSONValue bundle = _find(container, workspace, "bundle");
    if (bundle.isValid())
    {
        if (bundle.getKind() != JSONValue::Kind::Object)
        {
            outError = "Workspace 'bundle' must be an object.";
            return SLANG_FAIL;
        }
        for (auto pair : container->getObject(bundle))
        {
            String key = container->getStringFromKey(pair.key);
            if (key != "modules" && key != "source")
            {
                outError = String("Unknown field in 'workspace.bundle': ") + key;
                return SLANG_FAIL;
            }
        }
        SLANG_RETURN_ON_FAIL(_readOptionalBool(
            container,
            bundle,
            "modules",
            true,
            outWorkspace.bundle.modules,
            outError));
        SLANG_RETURN_ON_FAIL(_readOptionalBool(
            container,
            bundle,
            "source",
            true,
            outWorkspace.bundle.source,
            outError));
    }
    if (outWorkspace.depsDirectory.getLength())
        outWorkspace.depsDirectory = Path::simplify(outWorkspace.depsDirectory);
    if (outWorkspace.buildDirectory.getLength())
        outWorkspace.buildDirectory = Path::simplify(outWorkspace.buildDirectory);
    if ((outWorkspace.depsDirectory.getLength() &&
         (outWorkspace.depsDirectory == "." || !_isSafeRelativePath(outWorkspace.depsDirectory))) ||
        (outWorkspace.buildDirectory.getLength() &&
         (outWorkspace.buildDirectory == "." || !_isSafeRelativePath(outWorkspace.buildDirectory))))
    {
        outError = "Workspace 'deps' and 'build' must be relative paths inside the workspace.";
        return SLANG_FAIL;
    }
    String effectiveDeps =
        outWorkspace.depsDirectory.getLength() ? outWorkspace.depsDirectory : "deps";
    String effectiveBuild =
        outWorkspace.buildDirectory.getLength() ? outWorkspace.buildDirectory : "build";
    if (_workspacePathsOverlap(effectiveDeps, effectiveBuild))
    {
        outError = "Workspace 'deps' and 'build' directories must not overlap.";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _readHost(
    JSONContainer* container,
    const JSONValue& root,
    HostSettings& outHost,
    String& outError)
{
    JSONValue host = _find(container, root, "host");
    if (!host.isValid())
        return SLANG_OK;
    if (host.getKind() != JSONValue::Kind::Object)
    {
        outError = "Field 'host' must be an object.";
        return SLANG_FAIL;
    }
    for (auto pair : container->getObject(host))
    {
        String key = container->getStringFromKey(pair.key);
        if (key != "executables" && key != "default")
        {
            outError = String("Unknown field in 'host': ") + key;
            return SLANG_FAIL;
        }
    }

    JSONValue executables = _find(container, host, "executables");
    if (executables.getKind() != JSONValue::Kind::Array)
    {
        outError = "Host 'executables' must be a non-empty array of names.";
        return SLANG_FAIL;
    }
    outHost.executables.clear();
    for (auto item : container->getArray(executables))
    {
        if (item.getKind() != JSONValue::Kind::String)
        {
            outError = "Every host executable name must be a string.";
            return SLANG_FAIL;
        }
        String name = container->getString(item);
        if (!isValidPackageName(name))
        {
            outError =
                String("Host executable name must be a filename without directory separators: ") +
                name;
            return SLANG_FAIL;
        }
        for (const auto& existing : outHost.executables)
        {
            if (existing == name)
            {
                outError = String("Duplicate host executable name: ") + name;
                return SLANG_FAIL;
            }
        }
        outHost.executables.add(name);
    }
    if (outHost.executables.getCount() == 0)
    {
        outError = "Host 'executables' must contain at least one name.";
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(
        _readOptionalString(container, host, "default", outHost.defaultExecutable, outError));
    if (outHost.defaultExecutable.getLength() == 0)
    {
        if (outHost.executables.getCount() != 1)
        {
            outError = "Host 'default' is required when more than one executable is listed.";
            return SLANG_FAIL;
        }
        outHost.defaultExecutable = outHost.executables[0];
        return SLANG_OK;
    }
    for (const auto& executable : outHost.executables)
    {
        if (executable == outHost.defaultExecutable)
            return SLANG_OK;
    }
    outError =
        String("Host 'default' is not listed in 'executables': ") + outHost.defaultExecutable;
    return SLANG_FAIL;
}

static SlangResult _readManifest(ParsedJSON& json, Manifest& outManifest, String& outError)
{
    outManifest = Manifest();
    if (json.root.getKind() != JSONValue::Kind::Object)
    {
        outError = "Package manifest must be an object.";
        return SLANG_FAIL;
    }
    for (auto pair : json.container->getObject(json.root))
    {
        String key = json.container->getStringFromKey(pair.key);
        if (key != "schema_version" && key != "name" && key != "exports" &&
            key != "license_files" && key != "dependencies" && key != "retractions" &&
            key != "workspace" && key != "host")
        {
            outError = String("Unknown field in slang-package.json: ") + key;
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(
        _requireSchemaVersion(json.container, json.root, "slang-package.json", outError));
    SLANG_RETURN_ON_FAIL(
        _readRequiredString(json.container, json.root, "name", outManifest.name, outError));
    if (!isValidPackageName(outManifest.name))
    {
        outError = String("Invalid package name: ") + outManifest.name;
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(_readRelativePathArray(
        json.container,
        json.root,
        "exports",
        outManifest.exports,
        outError));
    SLANG_RETURN_ON_FAIL(_readRelativePathArray(
        json.container,
        json.root,
        "license_files",
        outManifest.licenseFiles,
        outError));
    SLANG_RETURN_ON_FAIL(
        _readDependencies(json.container, json.root, outManifest.dependencies, outError));
    SLANG_RETURN_ON_FAIL(
        _readRetractions(json.container, json.root, outManifest.retractions, outError));
    SLANG_RETURN_ON_FAIL(
        _readWorkspace(json.container, json.root, outManifest.workspace, outError));
    SLANG_RETURN_ON_FAIL(_readHost(json.container, json.root, outManifest.host, outError));
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
    SLANG_RELEASE_ASSERT(isValidPackageName(dependency.name));
    writer.addUnquotedKey(dependency.name.getUnownedSlice(), SourceLoc());
    writer.startObject(SourceLoc());
    if (dependency.path.getLength())
    {
        _writeKey(writer, "path");
        writer.addStringValue(dependency.path.getUnownedSlice(), SourceLoc());
        _writeKey(writer, "as");
        writer.addStringValue(dependency.as.getUnownedSlice(), SourceLoc());
    }
    else
    {
        _writeKey(writer, "git");
        writer.addStringValue(dependency.git.getUnownedSlice(), SourceLoc());
        if (dependency.version.getLength() != 0)
        {
            _writeKey(writer, "version");
            writer.addStringValue(dependency.version.getUnownedSlice(), SourceLoc());
        }
        if (dependency.ref.getLength() != 0)
        {
            _writeKey(writer, "ref");
            writer.addStringValue(dependency.ref.getUnownedSlice(), SourceLoc());
            _writeKey(writer, "as");
            writer.addStringValue(dependency.as.getUnownedSlice(), SourceLoc());
        }
    }
    writer.endObject(SourceLoc());
}

static void _writeVersionPolicy(
    JSONWriter& writer,
    const String* packageName,
    const String& version,
    const String& reason)
{
    writer.startObject(SourceLoc());
    if (packageName)
    {
        _writeKey(writer, "package");
        writer.addStringValue(packageName->getUnownedSlice(), SourceLoc());
    }
    _writeKey(writer, "version");
    writer.addStringValue(version.getUnownedSlice(), SourceLoc());
    _writeKey(writer, "reason");
    writer.addStringValue(reason.getUnownedSlice(), SourceLoc());
    writer.endObject(SourceLoc());
}

SlangResult writeManifest(const String& path, const Manifest& manifest, String& outError)
{
    JSONWriter writer(JSONWriter::IndentationStyle::Allman);
    writer.startObject(SourceLoc());
    _writeKey(writer, "schema_version");
    writer.addIntegerValue(kSchemaVersion, SourceLoc());
    _writeKey(writer, "name");
    writer.addStringValue(manifest.name.getUnownedSlice(), SourceLoc());
    _writeKey(writer, "exports");
    _writeStringArray(writer, manifest.exports);
    _writeKey(writer, "license_files");
    _writeStringArray(writer, manifest.licenseFiles);
    _writeKey(writer, "dependencies");
    writer.startObject(SourceLoc());
    for (const auto& dependency : manifest.dependencies)
        _writeDependency(writer, dependency);
    writer.endObject(SourceLoc());
    if (manifest.retractions.getCount())
    {
        _writeKey(writer, "retractions");
        writer.startArray(SourceLoc());
        for (const auto& retraction : manifest.retractions)
            _writeVersionPolicy(writer, nullptr, retraction.version, retraction.reason);
        writer.endArray(SourceLoc());
    }
    if (manifest.workspace.depsDirectory.getLength() ||
        manifest.workspace.buildDirectory.getLength() || manifest.workspace.exclusions.getCount())
    {
        _writeKey(writer, "workspace");
        writer.startObject(SourceLoc());
        if (manifest.workspace.depsDirectory.getLength())
        {
            _writeKey(writer, "deps");
            writer.addStringValue(manifest.workspace.depsDirectory.getUnownedSlice(), SourceLoc());
        }
        if (manifest.workspace.buildDirectory.getLength())
        {
            _writeKey(writer, "build");
            writer.addStringValue(manifest.workspace.buildDirectory.getUnownedSlice(), SourceLoc());
        }
        _writeKey(writer, "bundle");
        writer.startObject(SourceLoc());
        _writeKey(writer, "modules");
        writer.addBoolValue(manifest.workspace.bundle.modules, SourceLoc());
        _writeKey(writer, "source");
        writer.addBoolValue(manifest.workspace.bundle.source, SourceLoc());
        writer.endObject(SourceLoc());
        if (manifest.workspace.exclusions.getCount())
        {
            _writeKey(writer, "excludes");
            writer.startArray(SourceLoc());
            for (const auto& exclusion : manifest.workspace.exclusions)
            {
                _writeVersionPolicy(
                    writer,
                    &exclusion.packageName,
                    exclusion.version,
                    exclusion.reason);
            }
            writer.endArray(SourceLoc());
        }
        writer.endObject(SourceLoc());
    }
    if (manifest.host.executables.getCount())
    {
        for (const auto& executable : manifest.host.executables)
            SLANG_RELEASE_ASSERT(isValidPackageName(executable));
        _writeKey(writer, "host");
        writer.startObject(SourceLoc());
        _writeKey(writer, "executables");
        _writeStringArray(writer, manifest.host.executables);
        if (manifest.host.defaultExecutable.getLength())
        {
            SLANG_RELEASE_ASSERT(isValidPackageName(manifest.host.defaultExecutable));
            _writeKey(writer, "default");
            writer.addStringValue(manifest.host.defaultExecutable.getUnownedSlice(), SourceLoc());
        }
        writer.endObject(SourceLoc());
    }
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
    for (auto field : container->getObject(pair.value))
    {
        String key = container->getStringFromKey(field.key);
        if (key != "git" && key != "path" && key != "ref" && key != "version" && key != "commit" &&
            key != "exports" && key != "dependencies")
        {
            outError = String("Unknown field in locked package '") + outPackage.name + "': " + key;
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(
        _readOptionalString(container, pair.value, "git", outPackage.git, outError));
    SLANG_RETURN_ON_FAIL(
        _readOptionalString(container, pair.value, "path", outPackage.path, outError));
    SLANG_RETURN_ON_FAIL(
        _readRequiredString(container, pair.value, "version", outPackage.version, outError));
    SemanticVersion ignoredVersion;
    if (SLANG_FAILED(parseExactVersion(outPackage.version, ignoredVersion, outError)))
    {
        outError = String("Locked package has an invalid effective version: ") + outPackage.name;
        return SLANG_FAIL;
    }
    if (outPackage.path.getLength())
    {
        if (_find(container, pair.value, "ref").isValid() ||
            _find(container, pair.value, "commit").isValid())
        {
            outError = String("Locked local package cannot also contain ref or commit: ") +
                       outPackage.name;
            return SLANG_FAIL;
        }
        if (!_isSafeLocalPath(outPackage.path))
        {
            outError = String("Locked local path must be relative: ") + outPackage.name;
            return SLANG_FAIL;
        }
        if (outPackage.git.getLength() && !_isSafeGitLocation(outPackage.git))
        {
            outError = String("Locked package has an unsafe Git location: ") + outPackage.name;
            return SLANG_FAIL;
        }
    }
    else
    {
        if (!outPackage.git.getLength())
        {
            outError = String("Locked package must contain 'git' or 'path': ") + outPackage.name;
            return SLANG_FAIL;
        }
        if (!_isSafeGitLocation(outPackage.git))
        {
            outError = String("Locked package has an unsafe Git location: ") + outPackage.name;
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(
            _readRequiredString(container, pair.value, "ref", outPackage.ref, outError));
        if (!_isSafeGitRef(outPackage.ref))
        {
            outError = String("Locked package has an unsafe Git ref: ") + outPackage.name;
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
    }
    SLANG_RETURN_ON_FAIL(
        _readRelativePathArray(container, pair.value, "exports", outPackage.exports, outError));
    if (!_find(container, pair.value, "dependencies").isValid())
    {
        outError = String("Locked package is missing dependency requirements: ") + outPackage.name +
                   ". Run 'slang package update'.";
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        _readDependencies(container, pair.value, outPackage.dependencies, outError));

    return SLANG_OK;
}

SlangResult readLockFile(const String& path, LockFile& outLock, String& outError)
{
    ParsedJSON json;
    SLANG_RETURN_ON_FAIL(_parseJSON(path, json, outError));
    outLock = LockFile();
    if (json.root.getKind() != JSONValue::Kind::Object)
    {
        outError = "Lock file must be an object.";
        return SLANG_FAIL;
    }

    for (auto pair : json.container->getObject(json.root))
    {
        String key = json.container->getStringFromKey(pair.key);
        if (key != "schema_version" && key != "packages")
        {
            outError = String("Unknown field in slang-package-lock.json: ") + key;
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(
        _requireSchemaVersion(json.container, json.root, "slang-package-lock.json", outError));

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
    _writeKey(writer, "schema_version");
    writer.addIntegerValue(kSchemaVersion, SourceLoc());
    _writeKey(writer, "packages");
    writer.startObject(SourceLoc());
    for (const auto& package : lock.packages)
    {
        SLANG_RELEASE_ASSERT(isValidPackageName(package.name));
        writer.addUnquotedKey(package.name.getUnownedSlice(), SourceLoc());
        writer.startObject(SourceLoc());
        if (package.git.getLength())
        {
            _writeKey(writer, "git");
            writer.addStringValue(package.git.getUnownedSlice(), SourceLoc());
        }
        if (package.path.getLength())
        {
            _writeKey(writer, "path");
            writer.addStringValue(package.path.getUnownedSlice(), SourceLoc());
        }
        else
        {
            _writeKey(writer, "ref");
            writer.addStringValue(package.ref.getUnownedSlice(), SourceLoc());
            _writeKey(writer, "commit");
            writer.addStringValue(package.commit.getUnownedSlice(), SourceLoc());
        }
        _writeKey(writer, "version");
        writer.addStringValue(package.version.getUnownedSlice(), SourceLoc());
        _writeKey(writer, "exports");
        _writeStringArray(writer, package.exports);
        _writeKey(writer, "dependencies");
        writer.startObject(SourceLoc());
        for (const auto& dependency : package.dependencies)
            _writeDependency(writer, dependency);
        writer.endObject(SourceLoc());
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

SlangResult readLocalPackages(const String& path, List<LocalPackage>& outPackages, String& outError)
{
    ParsedJSON json;
    SLANG_RETURN_ON_FAIL(_parseJSON(path, json, outError));
    if (json.root.getKind() != JSONValue::Kind::Object)
    {
        outError = "Workspace file must be an object.";
        return SLANG_FAIL;
    }
    for (auto pair : json.container->getObject(json.root))
    {
        String key = json.container->getStringFromKey(pair.key);
        if (key != "schema_version" && key != "edits" && key != "overrides")
        {
            outError = String("Unknown field in slang-workspace.json: ") + key;
            return SLANG_FAIL;
        }
    }
    SLANG_RETURN_ON_FAIL(
        _requireSchemaVersion(json.container, json.root, "slang-workspace.json", outError));
    outPackages.clear();
    JSONValue edits = _find(json.container, json.root, "edits");
    if (edits.isValid() && edits.getKind() != JSONValue::Kind::Object)
    {
        outError = "Field 'edits' must be an object.";
        return SLANG_FAIL;
    }
    if (edits.isValid())
    {
        for (auto pair : json.container->getObject(edits))
        {
            LocalPackage package;
            package.name = json.container->getStringFromKey(pair.key);
            if (!isValidPackageName(package.name) ||
                pair.value.getKind() != JSONValue::Kind::Object)
            {
                outError = String("Invalid edited package entry: ") + package.name;
                return SLANG_FAIL;
            }
            for (auto field : json.container->getObject(pair.value))
            {
                outError = String("Unknown field in edited package '") + package.name +
                           "': " + json.container->getStringFromKey(field.key);
                return SLANG_FAIL;
            }
            package.kind = LocalPackageKind::Edit;
            outPackages.add(package);
        }
    }

    JSONValue overrides = _find(json.container, json.root, "overrides");
    if (overrides.isValid() && overrides.getKind() != JSONValue::Kind::Object)
    {
        outError = "Field 'overrides' must be an object.";
        return SLANG_FAIL;
    }
    if (overrides.isValid())
    {
        for (auto pair : json.container->getObject(overrides))
        {
            LocalPackage package;
            package.name = json.container->getStringFromKey(pair.key);
            if (!isValidPackageName(package.name) ||
                pair.value.getKind() != JSONValue::Kind::Object)
            {
                outError = String("Invalid override entry: ") + package.name;
                return SLANG_FAIL;
            }
            for (auto field : json.container->getObject(pair.value))
            {
                String key = json.container->getStringFromKey(field.key);
                if (key != "path" && key != "as")
                {
                    outError = String("Unknown field in override '") + package.name + "'.";
                    return SLANG_FAIL;
                }
            }
            SLANG_RETURN_ON_FAIL(
                _readRequiredString(json.container, pair.value, "path", package.path, outError));
            SLANG_RETURN_ON_FAIL(
                _readOptionalString(json.container, pair.value, "as", package.as, outError));
            if (package.as.getLength())
            {
                SemanticVersion ignoredVersion;
                SLANG_RETURN_ON_FAIL(parseExactVersion(package.as, ignoredVersion, outError));
            }
            if (!_isSafeLocalPath(package.path))
            {
                outError = String("Override path must be relative: ") + package.name;
                return SLANG_FAIL;
            }
            bool duplicate = false;
            for (const auto& existing : outPackages)
                duplicate = duplicate || existing.name == package.name;
            if (duplicate)
            {
                outError = String("Package cannot be both edited and overridden: ") + package.name;
                return SLANG_FAIL;
            }
            outPackages.add(package);
        }
    }
    if (!edits.isValid() && !overrides.isValid())
    {
        outError = "Workspace file must contain 'edits' or 'overrides'.";
        return SLANG_FAIL;
    }
    outPackages.sort([](const LocalPackage& left, const LocalPackage& right)
                     { return left.name < right.name; });
    return SLANG_OK;
}

SlangResult writeLocalPackages(
    const String& path,
    const List<LocalPackage>& packages,
    String& outError)
{
    JSONWriter writer(JSONWriter::IndentationStyle::Allman);
    writer.startObject(SourceLoc());
    _writeKey(writer, "schema_version");
    writer.addIntegerValue(kSchemaVersion, SourceLoc());
    _writeKey(writer, "edits");
    writer.startObject(SourceLoc());
    for (const auto& package : packages)
    {
        if (!isEditedLocalPackage(package))
            continue;
        SLANG_RELEASE_ASSERT(isValidPackageName(package.name));
        writer.addUnquotedKey(package.name.getUnownedSlice(), SourceLoc());
        writer.startObject(SourceLoc());
        writer.endObject(SourceLoc());
    }
    writer.endObject(SourceLoc());
    _writeKey(writer, "overrides");
    writer.startObject(SourceLoc());
    for (const auto& package : packages)
    {
        if (isEditedLocalPackage(package))
            continue;
        SLANG_RELEASE_ASSERT(isValidPackageName(package.name));
        writer.addUnquotedKey(package.name.getUnownedSlice(), SourceLoc());
        writer.startObject(SourceLoc());
        _writeKey(writer, "path");
        writer.addStringValue(package.path.getUnownedSlice(), SourceLoc());
        if (package.as.getLength())
        {
            _writeKey(writer, "as");
            writer.addStringValue(package.as.getUnownedSlice(), SourceLoc());
        }
        writer.endObject(SourceLoc());
    }
    writer.endObject(SourceLoc());
    writer.endObject(SourceLoc());
    writer.getBuilder() << "\n";
    if (SLANG_FAILED(File::writeAllText(path, writer.getBuilder())))
    {
        outError = String("Cannot write local package registry: ") + path;
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
