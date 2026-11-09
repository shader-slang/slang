#include "core/slang-math.h"
#include "slang-hgvm/slang-hgvm-compiler.h"
#include "slang-hgvm/slang-hgvm.h"
#include "slangvm-browser-call.h"
#include "slangvm-js-file-system.h"
#include "slangvm-browser-host.h"

#include <cmath>
#include <emscripten/bind.h>
#include <limits>

namespace Slang::HGVM
{
namespace
{
using emscripten::val;

val failure(SlangResult code, const String& error)
{
    val result = val::object();
    result.set(
        "error",
        std::string(error.getLength() ? error.getBuffer() : "HGVM operation failed"));
    result.set("code", code);
    return result;
}

/// Keeps reentry checks tied to native execution, rather than to a Promise's later JS reaction.
struct ActiveCall
{
    explicit ActiveCall(bool& active)
        : active(active)
    {
        active = true;
    }
    ~ActiveCall() { active = false; }
    bool& active;
};

class BrowserOutput : public IHGVMOutputSink
{
public:
    explicit BrowserOutput(val host)
        : m_host(std::move(host))
    {
    }

    void writeLine(std::string_view text) override
    {
        val arguments = val::array();
        arguments.call<void>("push", std::string(text));
        auto result = callBrowserHost(m_host, "print", arguments);
        getBrowserError(result, error);
    }

    String error;

private:
    val m_host;
};

SlangResult encodeScalar(const val& value, ScalarType type, Argument& outArgument, String& error)
{
    std::string jsType = value.typeOf().as<std::string>();
    if (type == ScalarType::Bool)
    {
        if (jsType != "boolean")
        {
            error = "A Slang bool parameter requires a JavaScript boolean";
            return SLANG_E_INVALID_ARG;
        }
        outArgument = Argument::scalar(value.as<bool>());
        return SLANG_OK;
    }
    if (jsType != "number")
    {
        error = "A Slang numeric parameter requires a JavaScript number";
        return SLANG_E_INVALID_ARG;
    }
    double number = value.as<double>();
    switch (type)
    {
    case ScalarType::Int32:
    case ScalarType::UInt32:
        {
            double minimum = type == ScalarType::Int32 ? double(INT32_MIN) : 0.0;
            double maximum = type == ScalarType::Int32 ? double(INT32_MAX) : double(UINT32_MAX);
            if (!std::isfinite(number) || std::trunc(number) != number || number < minimum ||
                number > maximum)
            {
                error = "Value is not an integer in the declared Slang scalar range";
                return SLANG_E_INVALID_ARG;
            }
            outArgument = type == ScalarType::Int32 ? Argument::scalar(int32_t(number))
                                                    : Argument::scalar(uint32_t(number));
            return SLANG_OK;
        }
    case ScalarType::Float64:
        outArgument = Argument::scalar(number);
        return SLANG_OK;
    case ScalarType::Float32:
    case ScalarType::Float16:
        {
            if (std::isfinite(number) && std::abs(number) > std::numeric_limits<float>::max())
                break;
            float narrowed = float(number);
            if (type == ScalarType::Float32)
            {
                outArgument = Argument::scalar(narrowed);
                return SLANG_OK;
            }
            uint16_t bits = FloatToHalf(narrowed);
            if (std::isfinite(number) && !std::isfinite(HalfToFloat(bits)))
                break;
            outArgument = Argument::fromScalar(type, &bits, sizeof(bits));
            return SLANG_OK;
        }
    default:
        error = "This Slang scalar type is not supported by the JavaScript binding";
        return SLANG_E_NOT_IMPLEMENTED;
    }
    error = "Value is outside the declared Slang scalar range";
    return SLANG_E_INVALID_ARG;
}

template<typename T>
val decodeScalar(const Value& result, String& error)
{
    T scalar = {};
    SlangResult status =
        result.getScalarData(result.getType().scalarType, &scalar, sizeof(T), error);
    return SLANG_SUCCEEDED(status) ? val(scalar) : failure(status, error);
}
} // namespace

/// Owns the same compiler, staged programs, and interpreter used by the Python and C++ hosts.
/// IDs index bridge-owned handles; native addresses and aggregate bytes never escape to JavaScript.
class WasmVM
{
public:
    WasmVM(val settings, val host, val fileSystem)
        : m_settings(std::move(settings)),
          m_host(std::move(host)),
          m_fileSystem(new JavaScriptFileSystem(std::move(fileSystem)))
    {
    }

    bool isBusy() const { return m_busy; }

    val initialize()
    {
        if (m_busy || m_interpreter || m_closed)
            return failure(SLANG_FAIL, "The slangvm VM is already initialized, busy, or closed");
        ActiveCall active(m_busy);
        SlangResult status = slang_createGlobalSession(SLANG_API_VERSION, m_global.writeRef());
        if (SLANG_FAILED(status))
            return failure(status, "Cannot create the Slang global session");
        m_compiler = new HGBCCompiler(m_global, m_fileSystem);

        val backend = m_settings["backend"];
        if (!backend.isUndefined() && backend.as<std::string>() == "webgpu")
        {
            rhi::DeviceDesc desc;
            desc.deviceType = rhi::DeviceType::WGPU;
            desc.slang.slangGlobalSession = m_global;
            slang::CompilerOptionEntry experimental = {};
            experimental.name = slang::CompilerOptionName::ExperimentalFeature;
            experimental.value.kind = slang::CompilerOptionValueKind::Int;
            experimental.value.intValue0 = 1;
            desc.slang.compilerOptionEntries = &experimental;
            desc.slang.compilerOptionEntryCount = 1;
            status = rhi::getRHI()->createDevice(desc, m_device.writeRef());
            if (SLANG_FAILED(status))
                return failure(status, "Cannot create the browser WebGPU device");
        }

        m_hostServices = new BrowserHostServices(m_device, m_host);
        m_output = new BrowserOutput(m_host);
        Desc desc;
        desc.device = m_device;
        desc.fileSystem = m_fileSystem;
        desc.workingDirectory = ".";
        desc.hostServices = m_hostServices;
        desc.outputSink = m_output;
        m_interpreter = new Interpreter(desc);
        return val::object();
    }

    val load(const std::string& path)
    {
        String error;
        if (!requireIdle(error))
            return failure(SLANG_FAIL, error);
        ActiveCall active(m_busy);
        List<String> searchPathStorage;
        List<const char*> searchPaths;
        val paths = m_settings["searchPaths"];
        if (!paths.isUndefined())
        {
            for (uint32_t i = 0; i < paths["length"].as<uint32_t>(); ++i)
                searchPathStorage.add(String(paths[i].as<std::string>().c_str()));
            for (const auto& searchPath : searchPathStorage)
                searchPaths.add(searchPath.getBuffer());
        }

        ComPtr<slang::IBlob> code;
        m_fileSystem->clearLastError();
        SlangResult status = m_compiler->compile(
            path.c_str(),
            nullptr,
            searchPaths.getBuffer(),
            searchPaths.getCount(),
            SLANG_DEBUG_INFO_LEVEL_NONE,
            SLANG_DIAGNOSTIC_COLOR_NEVER,
            code,
            error);
        if (SLANG_FAILED(status))
        {
            if (m_fileSystem->getLastError().getLength())
                error = error + "\n" + m_fileSystem->getLastError();
            return failure(status, error);
        }
        if (browserClosed())
            return failure(SLANG_E_ABORT, "The slangvm VM is closed");

        RefPtr<Program> program;
        status =
            Program::load(code->getBufferPointer(), code->getBufferSize(), path, program, error);
        if (SLANG_FAILED(status))
            return failure(status, error);
        uint64_t stagedProgram = 0;
        status = m_interpreter->prepareProgram(program, stagedProgram, error);
        if (SLANG_FAILED(status))
            return failure(status, error);
        m_programs.add(program);

        val exports = val::array();
        for (const auto& entry : program->getIndex().exports)
        {
            auto function = program->findFunctionExport(entry.first);
            if (!function)
                continue;
            NativeFunction native;
            native.program = program;
            native.functionId = function->id;
            m_functions.add(std::move(native));
            val exported = val::object();
            exported.set("name", entry.first);
            exported.set("functionId", uint32_t(m_functions.getCount()));
            exports.call<void>("push", exported);
        }
        val result = val::object();
        result.set("moduleId", uint32_t(m_programs.getCount()));
        result.set("functions", exports);
        return result;
    }

    val invoke(uint32_t functionId, val arguments)
    {
        String error;
        if (!requireIdle(error))
            return failure(SLANG_FAIL, error);
        ActiveCall active(m_busy);
        if (!functionId || functionId > uint32_t(m_functions.getCount()))
            return failure(SLANG_E_INVALID_ARG, "Invalid Slang function handle");
        auto& function = m_functions[functionId - 1];
        if (!function.bound)
        {
            SlangResult status = m_interpreter->bindFunction(
                function.program,
                function.functionId,
                function.bound,
                error);
            if (SLANG_FAILED(status))
                return failure(status, error);
        }
        RefPtr<BoundFunction> bound = function.bound;
        auto parameters = bound->getParameterTypes();
        if (arguments["length"].as<uint32_t>() != uint32_t(parameters.getCount()))
            return failure(
                SLANG_E_INVALID_ARG,
                "Argument count does not match the Slang signature");

        List<Argument> nativeArguments;
        for (Index i = 0; i < parameters.getCount(); ++i)
        {
            const auto& type = parameters[i];
            val argument = arguments[uint32_t(i)];
            Argument native;
            if (type.kind == HostValueType::Kind::Scalar)
            {
                SlangResult status = encodeScalar(argument, type.scalarType, native, error);
                if (SLANG_FAILED(status))
                    return failure(status, error);
            }
            else
            {
                if (argument.isNull() || argument.typeOf().as<std::string>() != "object")
                    return failure(SLANG_E_INVALID_ARG, "A Slang object requires a slangvm handle");
                if (argument["kind"].isUndefined() || argument["id"].isUndefined())
                    return failure(SLANG_E_INVALID_ARG, "Invalid slangvm handle");
                auto kind = argument["kind"].as<std::string>();
                uint32_t id = argument["id"].as<uint32_t>();
                if (kind == "value")
                {
                    auto value = m_values.tryGetValue(id);
                    if (!value)
                        return failure(SLANG_E_INVALID_ARG, "The Slang value has been disposed");
                    native = Argument::fromValue(*value);
                }
                else if (type.kind == HostValueType::Kind::HostObject && kind == "window")
                {
                    auto window = m_windows.tryGetValue(id);
                    if (!window)
                        return failure(SLANG_E_INVALID_ARG, "The Slang window has been disposed");
                    native = Argument::fromHostObject(*window);
                }
                else
                {
                    return failure(SLANG_E_INVALID_ARG, "Handle kind does not match the parameter");
                }
            }
            nativeArguments.add(std::move(native));
        }

        m_output->error = "";
        Value value;
        SlangResult status = (*bound)(nativeArguments.getArrayView(), value, error);
        if (SLANG_FAILED(status))
            return failure(status, error);
        if (browserClosed())
            return failure(SLANG_E_ABORT, "The slangvm VM is closed");
        if (m_output->error.getLength())
            return failure(SLANG_FAIL, m_output->error);

        val result = val::object();
        if (value.isVoid())
        {
            result.set("kind", "void");
            return result;
        }
        const auto& type = value.getType();
        if (type.kind != HostValueType::Kind::Scalar)
        {
            if (!m_nextValue)
                return failure(SLANG_FAIL, "The VM value handle space is exhausted");
            uint32_t id = m_nextValue++;
            m_values.add(id, value);
            result.set("kind", "value");
            result.set("id", id);
            result.set("typeName", std::string(type.name.getBuffer()));
            return result;
        }

        val scalar = val::undefined();
        switch (type.scalarType)
        {
        case ScalarType::Bool:
            {
                uint8_t bits = 0;
                status = value.getScalarData(type.scalarType, &bits, sizeof(bits), error);
                scalar = val(bits != 0);
                break;
            }
        case ScalarType::Int32:
            scalar = decodeScalar<int32_t>(value, error);
            break;
        case ScalarType::UInt32:
            scalar = decodeScalar<uint32_t>(value, error);
            break;
        case ScalarType::Float16:
            {
                uint16_t bits = 0;
                status = value.getScalarData(type.scalarType, &bits, sizeof(bits), error);
                scalar = val(HalfToFloat(bits));
                break;
            }
        case ScalarType::Float32:
            scalar = decodeScalar<float>(value, error);
            break;
        case ScalarType::Float64:
            scalar = decodeScalar<double>(value, error);
            break;
        default:
            return failure(SLANG_E_NOT_IMPLEMENTED, "Unsupported Slang scalar result type");
        }
        if (SLANG_FAILED(status) || getBrowserError(scalar, error))
            return failure(SLANG_FAIL, error);
        result.set("kind", "scalar");
        result.set("value", scalar);
        return result;
    }

    val createWindow(uint32_t canvasId)
    {
        String error;
        if (!requireIdle(error))
            return failure(SLANG_FAIL, error);
        ActiveCall active(m_busy);
        if (!m_device)
            return failure(SLANG_E_NOT_AVAILABLE, "Creating a Window requires backend: 'webgpu'");
        RefPtr<HGVMHostObject> window;
        SlangResult status = m_hostServices->adoptWindow(canvasId, window, error);
        if (SLANG_FAILED(status))
            return failure(status, error);
        if (!m_nextWindow)
            return failure(SLANG_FAIL, "The VM window handle space is exhausted");
        uint32_t id = m_nextWindow++;
        m_windows.add(id, window);
        val result = val::object();
        result.set("windowId", id);
        return result;
    }

    void releaseValue(uint32_t id) { m_values.remove(id); }

    void releaseWindow(uint32_t id)
    {
        if (auto window = m_windows.tryGetValue(id))
        {
            String error;
            m_hostServices->closeWindow(*window, error);
            m_windows.remove(id);
        }
    }

    val close()
    {
        if (m_busy)
            return failure(SLANG_FAIL, "Cannot release a VM while a native call is active");
        if (m_closed)
            return val::object();
        ActiveCall active(m_busy);
        String error;
        SlangResult status = m_interpreter ? m_interpreter->close(error) : SLANG_OK;
        if (m_interpreter && !m_interpreter->isClosed())
            return failure(status, error);
        m_closed = true;
        m_values.clear();
        m_windows.clear();
        m_functions.clear();
        m_programs.clear();
        m_interpreter = nullptr;
        m_compiler = nullptr;
        m_hostServices = nullptr;
        m_fileSystem = nullptr;
        m_output = nullptr;
        m_device = nullptr;
        m_global = nullptr;
        return SLANG_SUCCEEDED(status) ? val::object() : failure(status, error);
    }

private:
    struct NativeFunction
    {
        RefPtr<Program> program;
        HGBCId functionId = 0;
        RefPtr<BoundFunction> bound;
    };

    bool browserClosed() const { return m_host.call<bool>("isClosed"); }

    bool requireIdle(String& error) const
    {
        if (m_closed || !m_interpreter || browserClosed())
        {
            error = "The slangvm VM is closed or uninitialized";
            return false;
        }
        if (m_busy)
        {
            error = "The slangvm VM is busy; await its active call before invoking another";
            return false;
        }
        return true;
    }

    val m_settings;
    val m_host;
    bool m_busy = false;
    bool m_closed = false;
    uint32_t m_nextValue = 1;
    uint32_t m_nextWindow = 1;
    ComPtr<slang::IGlobalSession> m_global;
    ComPtr<JavaScriptFileSystem> m_fileSystem;
    ComPtr<rhi::IDevice> m_device;
    RefPtr<HGBCCompiler> m_compiler;
    RefPtr<BrowserHostServices> m_hostServices;
    RefPtr<BrowserOutput> m_output;
    RefPtr<Interpreter> m_interpreter;
    List<RefPtr<Program>> m_programs;
    List<NativeFunction> m_functions;
    Dictionary<uint32_t, Value> m_values;
    Dictionary<uint32_t, RefPtr<HGVMHostObject>> m_windows;
};
} // namespace Slang::HGVM

EMSCRIPTEN_BINDINGS(slangvm)
{
    using namespace emscripten;
    using Slang::HGVM::WasmVM;
    class_<WasmVM>("WasmVM")
        .constructor<val, val, val>()
        .function("initialize", &WasmVM::initialize, async())
        .function("load", &WasmVM::load, async())
        .function("invoke", &WasmVM::invoke, async())
        .function("createWindow", &WasmVM::createWindow, async())
        .function("close", &WasmVM::close, async())
        .function("isBusy", &WasmVM::isBusy)
        .function("releaseValue", &WasmVM::releaseValue)
        .function("releaseWindow", &WasmVM::releaseWindow);
}
