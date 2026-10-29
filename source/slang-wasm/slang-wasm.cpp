#include "slang-wasm.h"

#include "core/slang-blob.h"
#include "core/slang-exception.h"
#include "slang/slang-capability.h"
#include "slang/slang-language-server.h"

#include <deque>
#include <slang.h>
#include <stdint.h>
#include <vector>

using namespace slang;

namespace slang
{
namespace wgsl
{

Error g_error;

Error getLastError()
{
    Error currentError = g_error;
    g_error = {};
    return currentError;
}

std::string getVersionString()
{
    return std::string(spGetBuildTagString());
}

emscripten::val getCompileTargets()
{
    struct TargetPair
    {
        const char* name;
        SlangCompileTarget target;
    };
    static const TargetPair targets[] = {
        {"GLSL", SLANG_GLSL},
        {"HLSL", SLANG_HLSL},
        {"WGSL", SLANG_WGSL},
        {"SPIRV", SLANG_SPIRV},
        {"METAL", SLANG_METAL},
        {"CUDA", SLANG_CUDA_SOURCE},
    };

    std::vector<emscripten::val> result;
    for (auto target : targets)
    {
        auto entry = emscripten::val::object();
        entry.set("name", target.name);
        entry.set("value", (int)target.target);
        result.push_back(entry);
    }
    return emscripten::val::array(result);
}

GlobalSession* createGlobalSession()
{
    IGlobalSession* globalSession = nullptr;
    {
        SlangResult result = slang::createGlobalSession(&globalSession);
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return nullptr;
        }
    }

    return new GlobalSession(globalSession);
}

// True if `object` has an own property `name` whose value is neither `undefined` nor `null`.
// - Own property: a descriptor is read like a plain object literal. Properties inherited from a
//   prototype, including getters on a class, are not read.
// - `undefined` and `null` count as leaving the field out, so a caller can build a descriptor
//   with `{ profile: maybeProfile }` and get the C++ default when `maybeProfile` is not set.
static bool hasOwnValue(const emscripten::val& object, const char* name)
{
    return object.hasOwnProperty(name) && !object[name].isUndefined() && !object[name].isNull();
}

static bool isArray(const emscripten::val& value)
{
    return emscripten::val::global("Array").call<bool>("isArray", value);
}

// True for a JavaScript object that is not `null` and not an array.
static bool isObject(const emscripten::val& value)
{
    return !value.isNull() && !value.isUndefined() &&
           value.typeOf().as<std::string>() == "object" && !isArray(value);
}

static void setCreateSessionError(const std::string& message)
{
    g_error.type = std::string("USER");
    g_error.result = SLANG_E_INVALID_ARG;
    g_error.message = "createSession: " + message;
}

// An HLSL target with no profile gets sm_6_6. This is the one departure from the C++ defaults.
// A session created from a number has always had it, and existing callers (slang-playground)
// rely on it; the SessionDesc form applies it too, so the two forms give the same target. An
// explicit `profile: 0` (SLANG_PROFILE_UNKNOWN) counts as no profile.
static void applyLegacyHlslDefaultProfile(IGlobalSession* globalSession, TargetDesc& target)
{
    if (target.format == SLANG_HLSL && target.profile == SLANG_PROFILE_UNKNOWN)
        target.profile = globalSession->findProfile("sm_6_6");
}

static bool isCompileTarget(int format)
{
    return format > SLANG_TARGET_UNKNOWN && format < SLANG_TARGET_COUNT_OF;
}

// Reads `value` as an int if it is a JavaScript number that is an integer in the int32 range.
// Converting anything else (NaN, 1.5, 2**40) to int would be undefined behaviour.
static bool readInt32(const emscripten::val& value, int& out)
{
    if (!value.isNumber() || !emscripten::val::global("Number").call<bool>("isInteger", value))
        return false;
    const double number = value.as<double>();
    if (number < double(INT32_MIN) || number > double(INT32_MAX))
        return false;
    out = int(number);
    return true;
}

namespace
{
// Converts a js::SessionDesc into a slang::SessionDesc. The C++ descriptors point into this
// object, so it must outlive the IGlobalSession::createSession call.
//
// What the reader accepts:
// - The fields it reads are the ones listed for createSession in slang-wasm.h. Any other
//   property is ignored, as JavaScript APIs usually do, so a descriptor that sets a field this
//   version doesn't read yet still works.
// - Every malformed field it reads gives the same result: createSession returns null and
//   getLastError() is a USER error with SLANG_E_INVALID_ARG and a message that names the field
//   by its path (for example `targets[1].format`). That covers a missing required field, a
//   field of the wrong JavaScript type, a number field that is not an int32 integer, an item
//   that is not an object, and a list that is not an array.
// - Some values are also range-checked. The rule: a value is checked when it decides how the
//   rest of the descriptor is read, or when the compiler uses it as a table index without a
//   check of its own. So `format` must be a SlangCompileTarget, an option `name` must be inside
//   the CompilerOptionName range, `value.kind` must be Int or String, and the Int value of a
//   Capability entry must be a capability atom (the compiler indexes its capability table with
//   it). The range check on `name` does not reject the retired and sentinel values inside the
//   range, which the C++ API also accepts.
// - Other enum values (profile, flags, floatingPointMode, lineDirectiveMode,
//   defaultMatrixLayoutMode) and the other option values are passed on without checks, as the
//   C++ API passes them.
//
// Every `read...` function returns false after setting g_error; the caller then returns null
// without touching g_error again.
struct SessionDescReader
{
    IGlobalSession* globalSession;

    // A deque, so the c_str() pointers stay valid while more strings are added.
    std::deque<std::string> strings;
    std::vector<const char*> searchPaths;
    std::vector<PreprocessorMacroDesc> macros;
    std::vector<CompilerOptionEntry> options;
    std::vector<TargetDesc> targets;
    // One list per target. It is sized before it is filled, so the inner vectors don't move.
    std::vector<std::vector<CompilerOptionEntry>> targetOptions;

    // Stores a copy of `string` that lives as long as the reader, and returns its c_str().
    const char* internString(std::string string)
    {
        strings.push_back(std::move(string));
        return strings.back().c_str();
    }

    static std::string fieldPath(const std::string& path, const char* name)
    {
        return path.empty() ? std::string(name) : path + "." + name;
    }

    static std::string itemPath(const std::string& path, const char* name, size_t index)
    {
        return fieldPath(path, name) + "[" + std::to_string(index) + "]";
    }

    bool fail(const std::string& path, const char* problem)
    {
        setCreateSessionError("'" + path + "' " + problem);
        return false;
    }

    // Each of these reads an optional field: if it is left out, `out` is unchanged and the
    // result is true. If it is present but has the wrong type, the result is false.
    bool readInt(const emscripten::val& object, const std::string& path, const char* name, int& out)
    {
        if (!hasOwnValue(object, name))
            return true;
        const emscripten::val value = object[name];
        if (!value.isNumber())
            return fail(fieldPath(path, name), "must be a number");
        if (!readInt32(value, out))
            return fail(fieldPath(path, name), "must be an integer in the int32 range");
        return true;
    }

    bool readBool(
        const emscripten::val& object,
        const std::string& path,
        const char* name,
        bool& out)
    {
        if (!hasOwnValue(object, name))
            return true;
        if (!object[name].isTrue() && !object[name].isFalse())
            return fail(fieldPath(path, name), "must be a boolean");
        out = object[name].isTrue();
        return true;
    }

    bool readString(
        const emscripten::val& object,
        const std::string& path,
        const char* name,
        const char*& out)
    {
        if (!hasOwnValue(object, name))
            return true;
        if (!object[name].isString())
            return fail(fieldPath(path, name), "must be a string");
        out = internString(object[name].as<std::string>());
        return true;
    }

    // Reads an optional array field.
    bool readArray(
        const emscripten::val& object,
        const std::string& path,
        const char* name,
        std::vector<emscripten::val>& out)
    {
        out.clear();
        if (!hasOwnValue(object, name))
            return true;
        if (!isArray(object[name]))
            return fail(fieldPath(path, name), "must be an array");
        out = emscripten::vecFromJSArray<emscripten::val>(object[name]);
        return true;
    }

    // Reads `object.compilerOptionEntries`: [{name, value: {kind, intValue0?, intValue1?,
    // stringValue0?, stringValue1?}}], as slang::CompilerOptionEntry. Values that are left out
    // keep the C++ defaults: 0 for the ints and nullptr for the strings.
    bool readCompilerOptionEntries(
        const emscripten::val& object,
        const std::string& path,
        std::vector<CompilerOptionEntry>& out)
    {
        std::vector<emscripten::val> entries;
        if (!readArray(object, path, "compilerOptionEntries", entries))
            return false;
        for (size_t i = 0; i < entries.size(); i++)
        {
            const emscripten::val& entry = entries[i];
            const std::string entryPath = itemPath(path, "compilerOptionEntries", i);
            const std::string valuePath = fieldPath(entryPath, "value");
            if (!isObject(entry))
                return fail(entryPath, "must be an object");
            if (!hasOwnValue(entry, "name"))
                return fail(fieldPath(entryPath, "name"), "is required");
            if (!hasOwnValue(entry, "value"))
                return fail(valuePath, "is required");
            const emscripten::val value = entry["value"];
            if (!isObject(value))
                return fail(valuePath, "must be an object");
            if (!hasOwnValue(value, "kind"))
                return fail(fieldPath(valuePath, "kind"), "is required");

            int name = 0;
            int kind = 0;
            if (!readInt(entry, entryPath, "name", name) ||
                !readInt(value, valuePath, "kind", kind))
                return false;
            // A name outside the enum can't be an option; reject it instead of storing an entry
            // that nothing reads.
            if (name < 0 || name >= int(CompilerOptionName::CountOf))
                return fail(
                    fieldPath(entryPath, "name"),
                    "is outside the CompilerOptionName range");
            if (kind != int(CompilerOptionValueKind::Int) &&
                kind != int(CompilerOptionValueKind::String))
                return fail(fieldPath(valuePath, "kind"), "must be 0 (Int) or 1 (String)");

            CompilerOptionEntry option = {};
            option.name = CompilerOptionName(name);
            option.value.kind = CompilerOptionValueKind(kind);
            if (!readInt(value, valuePath, "intValue0", option.value.intValue0) ||
                !readInt(value, valuePath, "intValue1", option.value.intValue1) ||
                !readString(value, valuePath, "stringValue0", option.value.stringValue0) ||
                !readString(value, valuePath, "stringValue1", option.value.stringValue1))
                return false;
            // getTargetCaps indexes the capability table with this value, with only a debug
            // assert on its range.
            if (option.name == CompilerOptionName::Capability &&
                option.value.kind == CompilerOptionValueKind::Int &&
                (option.value.intValue0 < 0 ||
                 option.value.intValue0 >= int(Slang::CapabilityName::Count)))
                return fail(fieldPath(valuePath, "intValue0"), "is not a capability atom");
            out.push_back(option);
        }
        return true;
    }

    // Reads `object` into targets[index] and targetOptions[index]. `read` sizes both vectors
    // before it calls this.
    bool readTarget(const emscripten::val& object, size_t index)
    {
        const std::string path = itemPath("", "targets", index);
        if (!isObject(object))
            return fail(path, "must be an object");
        if (!hasOwnValue(object, "format"))
            return fail(fieldPath(path, "format"), "is required");
        TargetDesc& target = targets[index];
        int format = 0;
        int profile = int(target.profile);
        int flags = int(target.flags);
        int floatingPointMode = int(target.floatingPointMode);
        int lineDirectiveMode = int(target.lineDirectiveMode);
        if (!readInt(object, path, "format", format) ||
            !readInt(object, path, "profile", profile) || !readInt(object, path, "flags", flags) ||
            !readInt(object, path, "floatingPointMode", floatingPointMode) ||
            !readInt(object, path, "lineDirectiveMode", lineDirectiveMode) ||
            !readBool(
                object,
                path,
                "forceGLSLScalarBufferLayout",
                target.forceGLSLScalarBufferLayout))
            return false;
        if (!isCompileTarget(format))
            return fail(fieldPath(path, "format"), "is not a SlangCompileTarget");
        target.format = SlangCompileTarget(format);
        target.profile = SlangProfileID(profile);
        target.flags = SlangTargetFlags(flags);
        target.floatingPointMode = SlangFloatingPointMode(floatingPointMode);
        target.lineDirectiveMode = SlangLineDirectiveMode(lineDirectiveMode);
        applyLegacyHlslDefaultProfile(globalSession, target);
        if (!readCompilerOptionEntries(object, path, targetOptions[index]))
            return false;
        target.compilerOptionEntries = targetOptions[index].data();
        target.compilerOptionEntryCount = (uint32_t)targetOptions[index].size();
        return true;
    }

    bool read(const emscripten::val& desc, SessionDesc& sessionDesc)
    {
        std::vector<emscripten::val> items;

        if (!readArray(desc, "", "targets", items))
            return false;
        targets.resize(items.size());
        targetOptions.resize(items.size());
        for (size_t i = 0; i < items.size(); i++)
        {
            if (!readTarget(items[i], i))
                return false;
        }

        if (!readArray(desc, "", "searchPaths", items))
            return false;
        for (size_t i = 0; i < items.size(); i++)
        {
            if (!items[i].isString())
                return fail(itemPath("", "searchPaths", i), "must be a string");
            searchPaths.push_back(internString(items[i].as<std::string>()));
        }

        if (!readArray(desc, "", "preprocessorMacros", items))
            return false;
        for (size_t i = 0; i < items.size(); i++)
        {
            const std::string macroPath = itemPath("", "preprocessorMacros", i);
            if (!isObject(items[i]))
                return fail(macroPath, "must be an object");
            if (!hasOwnValue(items[i], "name"))
                return fail(fieldPath(macroPath, "name"), "is required");
            // A macro with no `value` is defined as empty, as `-D NAME` is.
            PreprocessorMacroDesc macro = {nullptr, ""};
            if (!readString(items[i], macroPath, "name", macro.name) ||
                !readString(items[i], macroPath, "value", macro.value))
                return false;
            macros.push_back(macro);
        }

        if (!readCompilerOptionEntries(desc, "", options))
            return false;

        int matrixLayoutMode = int(sessionDesc.defaultMatrixLayoutMode);
        if (!readInt(desc, "", "defaultMatrixLayoutMode", matrixLayoutMode))
            return false;
        sessionDesc.defaultMatrixLayoutMode = SlangMatrixLayoutMode(matrixLayoutMode);
        sessionDesc.targets = targets.data();
        sessionDesc.targetCount = (SlangInt)targets.size();
        sessionDesc.searchPaths = searchPaths.data();
        sessionDesc.searchPathCount = (SlangInt)searchPaths.size();
        sessionDesc.preprocessorMacros = macros.data();
        sessionDesc.preprocessorMacroCount = (SlangInt)macros.size();
        sessionDesc.compilerOptionEntries = options.data();
        sessionDesc.compilerOptionEntryCount = (uint32_t)options.size();
        return true;
    }
};
} // namespace

Session* GlobalSession::createSession(js::SessionDescOrTarget desc)
{
    SessionDesc sessionDesc = {};
    SessionDescReader reader = {m_interface};

    // A number is a compile target: the session has that one target, with the C++ defaults and
    // applyLegacyHlslDefaultProfile. `numberTarget` is declared here, outside the `if`, because
    // `sessionDesc.targets` points at it until createSession returns.
    TargetDesc numberTarget = {};
    if (desc.isNumber())
    {
        int format = 0;
        if (!readInt32(desc, format) || !isCompileTarget(format))
        {
            setCreateSessionError("the compile target number is not a SlangCompileTarget");
            return nullptr;
        }
        numberTarget.format = SlangCompileTarget(format);
        applyLegacyHlslDefaultProfile(m_interface, numberTarget);
        sessionDesc.targets = &numberTarget;
        sessionDesc.targetCount = 1;
    }
    else if (!isObject(desc))
    {
        setCreateSessionError("expected a compile target number or a SessionDesc object");
        return nullptr;
    }
    else if (!reader.read(desc, sessionDesc))
    {
        return nullptr;
    }

    Slang::ComPtr<ISession> session;
    SlangResult result = m_interface->createSession(sessionDesc, session.writeRef());
    if (result != SLANG_OK)
    {
        g_error.type = std::string("USER");
        g_error.result = result;
        return nullptr;
    }
    return new Session(session);
}

int GlobalSession::findProfile(const std::string& name)
{
    return (int)m_interface->findProfile(name.c_str());
}

std::string GlobalSession::getBuiltinModuleSource(const std::string& moduleName)
{
    Slang::ComPtr<ISlangBlob> blob;
    SlangResult result = Slang::getBuiltinModuleSource(
        Slang::UnownedStringSlice(moduleName.c_str()),
        blob.writeRef());
    // The callee always succeeds: an unrecognized name yields an empty blob, not a failure.
    SLANG_ASSERT(result == SLANG_OK);
    SLANG_UNUSED(result);
    return std::string((const char*)blob->getBufferPointer(), blob->getBufferSize());
}

Session::~Session()
{
    m_componentTypes = {};
    auto refCount = static_cast<Slang::Linkage*>(m_interface.get())->debugGetReferenceCount();
    m_interface = nullptr;
}

emscripten::val Session::loadModuleFromSource(
    const std::string& slangCode,
    const std::string& name,
    const std::string& path)
{
    IModule* module = nullptr;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        Slang::ComPtr<ISlangBlob> slangCodeBlob =
            Slang::RawBlob::create(slangCode.c_str(), slangCode.size());
        module = m_interface->loadModuleFromSource(
            name.c_str(),
            path.c_str(),
            slangCodeBlob,
            diagnosticsBlob.writeRef());
        if (!module)
        {
            g_error.type = std::string("USER");
            g_error.message = std::string(
                (char*)diagnosticsBlob->getBufferPointer(),
                (char*)diagnosticsBlob->getBufferPointer() + diagnosticsBlob->getBufferSize());
            return emscripten::val::null();
        }
    }
    return emscripten::val(Module(module, this));
}

emscripten::val Module::findEntryPointByName(const std::string& name)
{
    Slang::ComPtr<IEntryPoint> entryPoint;
    {
        SlangResult result =
            moduleInterface()->findEntryPointByName(name.c_str(), entryPoint.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return emscripten::val::null();
        }
    }
    m_session->addComponentType(entryPoint.get());
    return emscripten::val(EntryPoint(entryPoint.get(), m_session));
}

emscripten::val Module::findAndCheckEntryPoint(const std::string& name, int stage)
{
    Slang::ComPtr<IEntryPoint> entryPoint;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = moduleInterface()->findAndCheckEntryPoint(
            name.c_str(),
            (SlangStage)stage,
            entryPoint.writeRef(),
            diagnosticsBlob.writeRef());
        if (!SLANG_SUCCEEDED(result))
        {
            g_error.type = std::string("USER");
            g_error.result = result;

            if (diagnosticsBlob->getBufferSize())
            {
                char* diagnostics = (char*)diagnosticsBlob->getBufferPointer();
                g_error.message = std::string(diagnostics);
            }
            return emscripten::val::null();
        }
    }
    m_session->addComponentType(entryPoint.get());
    return emscripten::val(EntryPoint(entryPoint.get(), m_session));
}

int Module::getDefinedEntryPointCount()
{
    return moduleInterface()->getDefinedEntryPointCount();
}

emscripten::val Module::getDefinedEntryPoint(int index)
{
    if (moduleInterface()->getDefinedEntryPointCount() <= index)
        return emscripten::val::null();

    Slang::ComPtr<IEntryPoint> entryPoint;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = moduleInterface()->getDefinedEntryPoint(index, entryPoint.writeRef());
        if (!SLANG_SUCCEEDED(result))
        {
            g_error.type = std::string("USER");
            g_error.result = result;

            if (diagnosticsBlob->getBufferSize())
            {
                char* diagnostics = (char*)diagnosticsBlob->getBufferPointer();
                g_error.message = std::string(diagnostics);
            }
            return emscripten::val::null();
        }
    }
    m_session->addComponentType(entryPoint.get());
    return emscripten::val(EntryPoint(entryPoint.get(), m_session));
}


emscripten::val Session::createCompositeComponentType(emscripten::val components)
{
    if (!components.isArray())
    {
        g_error.type = std::string("Slang WASM Bind");
        g_error.message = std::string("createCompositeComponentType: Components must be an array");
        return emscripten::val::null();
    }
    std::vector<emscripten::val> componentsArray =
        emscripten::vecFromJSArray<emscripten::val>(components);

    Slang::ComPtr<IComponentType> composite;
    {
        std::vector<IComponentType*> nativeComponents;
        for (size_t i = 0U; i < componentsArray.size(); i++)
        {
            auto componentVal = componentsArray[i];
            if (componentVal.instanceof (emscripten::val::module_property("ComponentType")))
            {
                auto componentType = componentVal.as<ComponentType>();
                nativeComponents.push_back(componentType.interface());
            }
        }
        SlangResult result = m_interface->createCompositeComponentType(
            nativeComponents.data(),
            (SlangInt)nativeComponents.size(),
            composite.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return emscripten::val::null();
        }
    }
    addComponentType(composite.get());
    return emscripten::val(ComponentType(composite, this));
}

emscripten::val ComponentType::link()
{
    Slang::ComPtr<IComponentType> linkedProgram;
    {
        Slang::ComPtr<ISlangBlob> diagnosticBlob;
        SlangResult result = interface()->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            g_error.message = std::string(
                (char*)diagnosticBlob->getBufferPointer(),
                (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
            return emscripten::val::null();
        }
    }
    m_session->addComponentType(linkedProgram.get());
    return emscripten::val(ComponentType(linkedProgram, m_session));
}

std::string ComponentType::getEntryPointCode(int entryPointIndex, int targetIndex)
{
    {
        Slang::ComPtr<IBlob> kernelBlob;
        Slang::ComPtr<ISlangBlob> diagnosticBlob;
        SlangResult result = interface()->getEntryPointCode(
            entryPointIndex,
            targetIndex,
            kernelBlob.writeRef(),
            diagnosticBlob.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            g_error.message = std::string(
                (char*)diagnosticBlob->getBufferPointer(),
                (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
            return "";
        }
        std::string wgslCode = std::string(
            (char*)kernelBlob->getBufferPointer(),
            (char*)kernelBlob->getBufferPointer() + kernelBlob->getBufferSize());
        return wgslCode;
    }

    return {};
}

// Since result code is binary, we can't return it as a string, we will need to use emscripten::val
// to wrap it and return it to the javascript side.
emscripten::val ComponentType::getEntryPointCodeBlob(int entryPointIndex, int targetIndex)
{
    Slang::ComPtr<IBlob> kernelBlob;
    Slang::ComPtr<ISlangBlob> diagnosticBlob;
    SlangResult result = interface()->getEntryPointCode(
        entryPointIndex,
        targetIndex,
        kernelBlob.writeRef(),
        diagnosticBlob.writeRef());
    if (result != SLANG_OK)
    {
        g_error.type = std::string("USER");
        g_error.result = result;
        g_error.message = std::string(
            (char*)diagnosticBlob->getBufferPointer(),
            (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
        return {};
    }

    const uint8_t* ptr = (uint8_t*)kernelBlob->getBufferPointer();
    return emscripten::val(emscripten::typed_memory_view(kernelBlob->getBufferSize(), ptr));
}

std::string ComponentType::getTargetCode(int targetIndex)
{
    {
        Slang::ComPtr<IBlob> kernelBlob;
        Slang::ComPtr<ISlangBlob> diagnosticBlob;
        SlangResult result = interface()->getTargetCode(
            targetIndex,
            kernelBlob.writeRef(),
            diagnosticBlob.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            g_error.message = std::string(
                (char*)diagnosticBlob->getBufferPointer(),
                (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
            return "";
        }
        std::string targetCode = std::string(
            (char*)kernelBlob->getBufferPointer(),
            (char*)kernelBlob->getBufferPointer() + kernelBlob->getBufferSize());
        return targetCode;
    }

    return {};
}

// Since result code is binary, we can't return it as a string, we will need to use emscripten::val
// to wrap it and return it to the javascript side.
emscripten::val ComponentType::getTargetCodeBlob(int targetIndex)
{
    Slang::ComPtr<IBlob> kernelBlob;
    Slang::ComPtr<ISlangBlob> diagnosticBlob;
    SlangResult result =
        interface()->getTargetCode(targetIndex, kernelBlob.writeRef(), diagnosticBlob.writeRef());
    if (result != SLANG_OK)
    {
        g_error.type = std::string("USER");
        g_error.result = result;
        g_error.message = std::string(
            (char*)diagnosticBlob->getBufferPointer(),
            (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
        return {};
    }

    const uint8_t* ptr = (uint8_t*)kernelBlob->getBufferPointer();
    return emscripten::val(emscripten::typed_memory_view(kernelBlob->getBufferSize(), ptr));
}

emscripten::val ComponentType::loadStrings()
{
    slang::ProgramLayout* slangReflection = interface()->getLayout();
    if (!slangReflection)
    {
        g_error.type = std::string("USER");
        g_error.message = std::string("Failed to get reflection data");
        return emscripten::val::null();
    }

    SlangUInt hashedStringCount = slangReflection->getHashedStringCount();
    if (hashedStringCount == 0)
    {
        g_error.type = std::string("USER");
        g_error.message = std::string("Warn: No reflection data found");
        return emscripten::val::null();
    }

    size_t stringSize = 0;
    std::vector<emscripten::val> result;
    for (SlangUInt ii = 0; ii < hashedStringCount; ++ii)
    {
        // For each string we can fetch its bytes from the Slang
        // reflection data.
        //
        size_t stringSize = 0;
        char const* stringData = slangReflection->getHashedString(ii, &stringSize);

        // Then we can compute the hash code for that string using
        // another Slang API function.
        //
        // Note: the exact hashing algorithm that Slang uses for
        // string literals is not currently documented, and may
        // change in future releases of the compiler.
        //
        int hash = spComputeStringHash(stringData, stringSize);

        emscripten::val entry = emscripten::val::object();
        entry.set("hash", hash);
        entry.set("string", std::string(stringData));
        result.push_back(entry);
    }
    return emscripten::val::array(result);
}

ProgramLayout* ComponentType::getLayout(uint32_t targetIndex)
{
    return (slang::wgsl::ProgramLayout*)interface()->getLayout(targetIndex);
}

uint32_t ProgramLayout::getParameterCount()
{
    return interface()->getParameterCount();
}

emscripten::val ProgramLayout::toJsonObject()
{
    Slang::ComPtr<ISlangBlob> blob;
    if (SLANG_FAILED(interface()->toJson(blob.writeRef())))
        return {};
    auto jsonString = std::string(
        (char*)blob->getBufferPointer(),
        (char*)blob->getBufferPointer() + blob->getBufferSize());
    emscripten::val parsedObject =
        emscripten::val::global("JSON").call<emscripten::val>("parse", jsonString);
    return parsedObject;
}

VariableLayoutReflection* ProgramLayout::getParameterByIndex(uint32_t index)
{
    return (slang::wgsl::VariableLayoutReflection*)(interface()->getParameterByIndex(index));
}

TypeLayoutReflection* ProgramLayout::getGlobalParamsTypeLayout()
{
    return (slang::wgsl::TypeLayoutReflection*)(interface()->getGlobalParamsTypeLayout());
}

FunctionReflection* ProgramLayout::findFunctionByName(const std::string& name)
{
    return (slang::wgsl::FunctionReflection*)(interface()->findFunctionByName(name.c_str()));
}

TypeReflection* ProgramLayout::findTypeByName(const std::string& name)
{
    return (slang::wgsl::TypeReflection*)(interface()->findTypeByName(name.c_str()));
}

VariableReflection* ProgramLayout::findVarByNameInType(
    slang::wgsl::TypeReflection* type,
    const std::string& name)
{
    if (!type)
        return nullptr;

    return (slang::wgsl::VariableReflection*)interface()->findVarByNameInType(
        type->interface(),
        name.c_str());
}

EntryPointReflection* ProgramLayout::findEntryPointByName(const std::string& name)
{
    return (slang::wgsl::EntryPointReflection*)(interface()->findEntryPointByName(name.c_str()));
}

EntryPointReflection::ThreadGroupSize EntryPointReflection::getComputeThreadGroupSize()
{
    SlangUInt size[3];
    interface()->getComputeThreadGroupSize(3, size);
    return {size[0], size[1], size[2]};
}

BindingType TypeLayoutReflection::getDescriptorSetDescriptorRangeType(
    uint32_t setIndex,
    uint32_t rangeIndex)
{
    return interface()->getDescriptorSetDescriptorRangeType(setIndex, rangeIndex);
}

std::string DeclReflection::getName()
{
    return interface()->getName();
}


slang::DeclReflection::Kind DeclReflection::getKind()
{
    return interface()->getKind();
}

uint32_t DeclReflection::getChildrenCount()
{
    return interface()->getChildrenCount();
};

slang::wgsl::DeclReflection* DeclReflection::getChild(uint32_t index)
{
    return (slang::wgsl::DeclReflection*)interface()->getChild(index);
}

slang::wgsl::TypeReflection* DeclReflection::getType()
{
    return (slang::wgsl::TypeReflection*)interface()->getType();
}
slang::wgsl::VariableReflection* DeclReflection::asVariable()
{
    return (slang::wgsl::VariableReflection*)interface()->asVariable();
}
slang::wgsl::FunctionReflection* DeclReflection::asFunction()
{
    return (slang::wgsl::FunctionReflection*)interface()->asFunction();
}
slang::wgsl::GenericReflection* DeclReflection::asGeneric()
{
    return (slang::wgsl::GenericReflection*)interface()->asGeneric();
}
slang::wgsl::DeclReflection* DeclReflection::getParent()
{
    return (slang::wgsl::DeclReflection*)interface()->getParent();
}


std::string GenericReflection::getName()
{
    return interface()->getName();
}

slang::wgsl::DeclReflection* GenericReflection::asDecl()
{
    return (slang::wgsl::DeclReflection*)interface()->asDecl();
}

uint32_t GenericReflection::getTypeParameterCount()
{
    return interface()->getTypeParameterCount();
}
slang::wgsl::VariableReflection* GenericReflection::getTypeParameter(unsigned index)
{
    return (slang::wgsl::VariableReflection*)interface()->getTypeParameter(index);
}
uint32_t GenericReflection::getValueParameterCount()
{
    return interface()->getValueParameterCount();
}
slang::wgsl::VariableReflection* GenericReflection::getValueParameter(unsigned index)
{
    return (slang::wgsl::VariableReflection*)interface()->getValueParameter(index);
}

slang::wgsl::DeclReflection* GenericReflection::getInnerDecl()
{
    return (slang::wgsl::DeclReflection*)interface()->getInnerDecl();
}

SlangDeclKind GenericReflection::getInnerKind()
{
    return interface()->getInnerKind();
}

slang::wgsl::GenericReflection* GenericReflection::getOuterGenericContainer()
{
    return (slang::wgsl::GenericReflection*)interface()->getOuterGenericContainer();
}

std::string VariableLayoutReflection::getName()
{
    return interface()->getName();
}

slang::wgsl::TypeLayoutReflection* VariableLayoutReflection::getTypeLayout()
{
    return (slang::wgsl::TypeLayoutReflection*)(interface()->getTypeLayout());
}

uint32_t VariableLayoutReflection::getBindingIndex()
{
    return interface()->getBindingIndex();
}

std::string VariableReflection::getName()
{
    return interface()->getName();
}

uint32_t VariableReflection::getUserAttributeCount()
{
    return interface()->getUserAttributeCount();
}

slang::wgsl::UserAttribute* VariableReflection::getUserAttributeByIndex(uint32_t index)
{
    return (slang::wgsl::UserAttribute*)interface()->getUserAttributeByIndex(index);
}

bool VariableReflection::hasDefaultValue()
{
    Slang::ComPtr<ISlangBlob> defaultValueBlob;
    return SLANG_SUCCEEDED(interface()->getDefaultValueBlob(defaultValueBlob.writeRef())) &&
           defaultValueBlob != nullptr;
}

emscripten::val VariableReflection::getDefaultValueBlob()
{
    Slang::ComPtr<ISlangBlob> defaultValueBlob;
    SlangResult result = interface()->getDefaultValueBlob(defaultValueBlob.writeRef());
    if (SLANG_FAILED(result))
    {
        g_error.type = std::string("USER");
        g_error.result = result;
        return emscripten::val::null();
    }

    if (!defaultValueBlob)
        return emscripten::val::null();

    uint8_t* ptr = (uint8_t*)defaultValueBlob->getBufferPointer();
    auto view =
        emscripten::val(emscripten::typed_memory_view(defaultValueBlob->getBufferSize(), ptr));
    auto resultArray =
        emscripten::val::global("Uint8Array").new_(defaultValueBlob->getBufferSize());
    resultArray.call<void>("set", view);
    return resultArray;
}

slang::wgsl::TypeReflection* VariableReflection::getType()
{
    return (slang::wgsl::TypeReflection*)interface()->getType();
}

Modifier* VariableReflection::findModifier(Modifier::ID id)
{
    return interface()->findModifier(id);
}

slang::wgsl::VariableReflection* TypeReflection::getFieldByIndex(uint32_t index)
{
    return (slang::wgsl::VariableReflection*)interface()->getFieldByIndex(index);
}

slang::TypeReflection::ScalarType TypeReflection::getScalarType()
{
    return interface()->getScalarType();
}

slang::TypeReflection::Kind TypeReflection::getKind()
{
    return interface()->getKind();
}


std::string UserAttribute::getName()
{
    return interface()->getName();
}

float UserAttribute::getArgumentValueFloat(uint32_t index)
{
    float value;
    interface()->getArgumentValueFloat(index, &value);
    return value;
}

std::string UserAttribute::getArgumentValueString(uint32_t index)
{
    size_t len = 0;
    const char* out = interface()->getArgumentValueString(index, &len);
    return std::string(out, len);
}


slang::wgsl::TypeReflection* UserAttribute::getArgumentType(uint32_t index)
{
    return (slang::wgsl::TypeReflection*)interface()->getArgumentType(index);
}


uint32_t UserAttribute::getArgumentCount()
{
    return interface()->getArgumentCount();
}

std::string FunctionReflection::getName()
{
    return interface()->getName();
}

uint32_t FunctionReflection::getUserAttributeCount()
{
    return interface()->getUserAttributeCount();
}

slang::wgsl::UserAttribute* FunctionReflection::getUserAttributeByIndex(uint32_t index)
{
    return (slang::wgsl::UserAttribute*)interface()->getUserAttributeByIndex(index);
}

namespace lsp
{
Position translate(Slang::LanguageServerProtocol::Position p)
{
    Position result;
    result.line = p.line;
    result.character = p.character;
    return result;
}
Range translate(Slang::LanguageServerProtocol::Range r)
{
    Range result;
    result.start = translate(r.start);
    result.end = translate(r.end);
    return result;
}
Location translate(Slang::LanguageServerProtocol::Location l)
{
    Location result;
    result.uri = l.uri.getBuffer();
    result.range = translate(l.range);
    return result;
}
Slang::LanguageServerProtocol::Position translate(Position p)
{
    Slang::LanguageServerProtocol::Position result;
    result.line = p.line;
    result.character = p.character;
    return result;
}
Slang::LanguageServerProtocol::Range translate(Range r)
{
    Slang::LanguageServerProtocol::Range result;
    result.start = translate(r.start);
    result.end = translate(r.end);
    return result;
}
Slang::LanguageServerProtocol::Location translate(Location l)
{
    Slang::LanguageServerProtocol::Location result;
    result.uri = l.uri.c_str();
    result.range = translate(l.range);
    return result;
}

LanguageServer::LanguageServer()
{
    Slang::LanguageServerStartupOptions options = {};
    m_core = new Slang::LanguageServerCore(options);
    init();
}

LanguageServer::~LanguageServer()
{
    delete m_core;
}

void LanguageServer::init()
{
    Slang::LanguageServerProtocol::InitializeParams args = {};
    Slang::LanguageServerProtocol::WorkspaceFolder folder = {};
    folder.uri = "file:///";
    folder.name = "/";
    args.workspaceFolders.add(folder);
    m_core->init(args);
}

void LanguageServer::didOpenTextDocument(std::string uri, std::string text)
{
    Slang::LanguageServerProtocol::DidOpenTextDocumentParams args = {};
    args.textDocument.uri = uri.c_str();
    args.textDocument.languageId = "slang";
    args.textDocument.text = text.c_str();
    m_core->didOpenTextDocument(args);
}

void LanguageServer::didCloseTextDocument(std::string uri)
{
    Slang::LanguageServerProtocol::DidCloseTextDocumentParams args = {};
    args.textDocument.uri = uri.c_str();
    m_core->didCloseTextDocument(args);
}

void LanguageServer::didChangeTextDocument(
    std::string uri,
    const std::vector<lsp::TextEdit>& changes)
{
    Slang::LanguageServerProtocol::DidChangeTextDocumentParams args = {};
    args.textDocument.uri = uri.c_str();
    for (auto change : changes)
    {
        Slang::LanguageServerProtocol::TextDocumentContentChangeEvent lspChange;
        lspChange.text = change.text.c_str();
        lspChange.range = translate(change.range);
        args.contentChanges.add(lspChange);
    }
    m_core->didChangeTextDocument(args);
}

std::optional<lsp::Hover> LanguageServer::hover(std::string uri, lsp::Position position)
{
    Slang::LanguageServerProtocol::HoverParams args = {};
    args.textDocument.uri = uri.c_str();
    args.position = translate(position);
    auto coreResult = m_core->hover(args);
    if (coreResult.isNull)
        return std::nullopt;
    lsp::Hover result;
    result.contents.kind = coreResult.result.contents.kind.getBuffer();
    result.contents.value = coreResult.result.contents.value.getBuffer();
    result.range = translate(coreResult.result.range);
    return result;
}

std::optional<std::vector<lsp::Location>> LanguageServer::gotoDefinition(
    std::string uri,
    lsp::Position position)
{
    Slang::LanguageServerProtocol::DefinitionParams args = {};
    args.textDocument.uri = uri.c_str();
    args.position = translate(position);
    auto coreResult = m_core->gotoDefinition(args);
    if (coreResult.isNull)
        return std::nullopt;
    std::vector<lsp::Location> result;
    for (auto location : coreResult.result)
        result.push_back(translate(location));
    return result;
}

std::optional<std::vector<lsp::CompletionItem>> LanguageServer::completion(
    std::string uri,
    lsp::Position position,
    CompletionContext context)
{
    Slang::LanguageServerProtocol::CompletionParams args = {};
    args.textDocument.uri = uri.c_str();
    args.position = translate(position);
    args.context.triggerKind = context.triggerKind;
    args.context.triggerCharacter = context.triggerCharacter.c_str();
    auto coreResult = m_core->completion(args);
    if (coreResult.isNull)
        return std::nullopt;
    std::vector<lsp::CompletionItem> result;
    for (auto item : coreResult.result.items)
    {
        lsp::CompletionItem completionItem;
        completionItem.label = item.label.getBuffer();
        completionItem.kind = item.kind;
        completionItem.detail = item.detail.getBuffer();
        MarkupContent documentation;
        documentation.kind = item.documentation.kind.getBuffer();
        documentation.value = item.documentation.value.getBuffer();
        completionItem.documentation = documentation;
        completionItem.textEdit = std::nullopt;
        completionItem.data = item.data.getBuffer();
        std::vector<std::string> commitCharacters;
        for (auto character : item.commitCharacters)
            commitCharacters.push_back(character.getBuffer());
        completionItem.commitCharacters = commitCharacters;
        result.push_back(completionItem);
    }
    return result;
}

std::optional<lsp::CompletionItem> LanguageServer::completionResolve(lsp::CompletionItem args)
{
    Slang::LanguageServerProtocol::CompletionItem coreArgs = {};
    coreArgs.label = args.label.c_str();
    coreArgs.kind = args.kind;
    coreArgs.detail = args.detail.c_str();
    if (args.documentation.has_value())
    {
        coreArgs.documentation.kind = args.documentation.value().kind.c_str();
        coreArgs.documentation.value = args.documentation.value().value.c_str();
    }
    coreArgs.data = args.data.c_str();
    if (args.commitCharacters.has_value())
    {
        for (auto character : args.commitCharacters.value())
            coreArgs.commitCharacters.add(character.c_str());
    }
    Slang::LanguageServerProtocol::TextEditCompletionItem editItem;
    editItem.label = coreArgs.label;
    editItem.kind = coreArgs.kind;
    editItem.detail = coreArgs.detail;
    editItem.documentation.kind = coreArgs.documentation.kind;
    editItem.documentation.value = coreArgs.documentation.value;
    editItem.data = coreArgs.data;

    for (auto character : coreArgs.commitCharacters)
        editItem.commitCharacters.add(character);

    auto coreResult = m_core->completionResolve(coreArgs, editItem);
    if (coreResult.isNull)
        return std::nullopt;
    lsp::CompletionItem result;
    result.label = coreResult.result.label.getBuffer();
    result.kind = coreResult.result.kind;
    result.detail = coreResult.result.detail.getBuffer();
    MarkupContent documentation;
    documentation.kind = coreResult.result.documentation.kind.getBuffer();
    documentation.value = coreResult.result.documentation.value.getBuffer();
    result.documentation = documentation;
    result.textEdit = std::nullopt;
    result.data = coreResult.result.data.getBuffer();
    std::vector<std::string> commitCharacters;
    for (auto character : coreResult.result.commitCharacters)
        commitCharacters.push_back(character.getBuffer());
    result.commitCharacters = commitCharacters;
    return result;
}

std::optional<std::vector<uint32_t>> LanguageServer::semanticTokens(std::string uri)
{
    Slang::LanguageServerProtocol::SemanticTokensParams args = {};
    args.textDocument.uri = uri.c_str();
    auto coreResult = m_core->semanticTokens(args);
    if (coreResult.isNull)
        return std::nullopt;
    std::vector<uint32_t> result;
    result.reserve((size_t)coreResult.result.data.getCount());
    for (auto token : coreResult.result.data)
        result.push_back(token);
    return result;
}

std::optional<lsp::SignatureHelp> LanguageServer::signatureHelp(
    std::string uri,
    lsp::Position position)
{
    Slang::LanguageServerProtocol::SignatureHelpParams args = {};
    args.textDocument.uri = uri.c_str();
    args.position = translate(position);
    auto coreResult = m_core->signatureHelp(args);
    if (coreResult.isNull)
        return std::nullopt;
    lsp::SignatureHelp result;
    for (auto signature : coreResult.result.signatures)
    {
        lsp::SignatureInformation signatureInfo;
        signatureInfo.label = signature.label.getBuffer();
        signatureInfo.documentation.kind = signature.documentation.kind.getBuffer();
        signatureInfo.documentation.value = signature.documentation.value.getBuffer();
        for (auto parameter : signature.parameters)
        {
            lsp::ParameterInformation parameterInfo;
            parameterInfo.label[0] = parameter.label[0];
            parameterInfo.label[1] = parameter.label[1];
            parameterInfo.documentation.kind = parameter.documentation.kind.getBuffer();
            parameterInfo.documentation.value = parameter.documentation.value.getBuffer();
            signatureInfo.parameters.push_back(parameterInfo);
        }
        result.signatures.push_back(signatureInfo);
    }
    result.activeSignature = coreResult.result.activeSignature;
    result.activeParameter = coreResult.result.activeParameter;
    return result;
}

lsp::DocumentSymbol translate(Slang::LanguageServerProtocol::DocumentSymbol symbol)
{
    lsp::DocumentSymbol result;
    result.name = symbol.name.getBuffer();
    result.detail = symbol.detail.getBuffer();
    result.kind = symbol.kind;
    result.range = translate(symbol.range);
    result.selectionRange = translate(symbol.selectionRange);
    for (auto child : symbol.children)
        result.children.push_back(translate(child));
    return result;
}

std::optional<std::vector<lsp::DocumentSymbol>> LanguageServer::documentSymbol(std::string uri)
{
    Slang::LanguageServerProtocol::DocumentSymbolParams args = {};
    args.textDocument.uri = uri.c_str();
    auto coreResult = m_core->documentSymbol(args);
    if (coreResult.isNull)
        return std::nullopt;
    std::vector<lsp::DocumentSymbol> result;
    for (auto symbol : coreResult.result)
    {
        auto documentSymbol = translate(symbol);
        result.push_back(documentSymbol);
    }
    return result;
}

std::optional<std::vector<lsp::Diagnostics>> LanguageServer::getDiagnostics(std::string uri)
{
    std::vector<lsp::Diagnostics> result;
    auto module = m_core->m_workspace->getCurrentVersion()->getOrLoadModule(
        Slang::URI::fromString(Slang::UnownedStringSlice(uri.c_str())).getPath());
    if (!module)
        return std::nullopt;
    for (auto& docDiag : m_core->m_workspace->getCurrentVersion()->diagnostics)
    {
        for (auto& message : docDiag.second.messages)
        {
            lsp::Diagnostics diag;
            diag.code = Slang::String(message.code).getBuffer();
            diag.range = translate(message.range);
            diag.severity = (int)message.severity;
            diag.message = message.message.getBuffer();
            result.push_back(diag);
        }
    }
    return result;
}


LanguageServer* createLanguageServer()
{
    return new LanguageServer();
}

} // namespace lsp

} // namespace wgsl
} // namespace slang
