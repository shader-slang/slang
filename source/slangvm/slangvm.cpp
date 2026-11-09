#include "core/slang-math.h"
#include "slang-hgvm/slang-hgvm-compiler.h"
#include "slang-hgvm/slang-hgvm.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <stdexcept>

namespace nb = nanobind;
using namespace Slang;
using namespace Slang::HGVM;

namespace
{
void check(SlangResult result, const String& error)
{
    if (SLANG_FAILED(result))
        throw std::runtime_error(error.getLength() ? error.getBuffer() : "HGVM operation failed");
}

void checkInvocation(SlangResult result, const String& error)
{
    if (result == SLANG_E_INVALID_ARG)
        throw nb::type_error(error.getBuffer());
    check(result, error);
}

struct Module;

/// Maps Python's backend setting to the existing RHI device factory.
rhi::DeviceType getDeviceType(const std::string& backend)
{
    static const struct
    {
        const char* name;
        rhi::DeviceType type;
    } backends[] = {
        {"default", rhi::DeviceType::Default},
        {"vulkan", rhi::DeviceType::Vulkan},
        {"d3d12", rhi::DeviceType::D3D12},
        {"d3d11", rhi::DeviceType::D3D11},
        {"metal", rhi::DeviceType::Metal},
        {"cuda", rhi::DeviceType::CUDA},
        {"webgpu", rhi::DeviceType::WGPU},
        {"cpu", rhi::DeviceType::CPU},
    };
    for (const auto& entry : backends)
        if (backend == entry.name)
            return entry.type;
    throw nb::value_error(
        "Unknown slangvm backend; expected none, default, vulkan, d3d12, d3d11, metal, cuda, "
        "webgpu, or cpu");
}

/// Owns the same compiler and interpreter used by the native command-line host.
struct VM
{
    ComPtr<slang::IGlobalSession> global;
    RefPtr<HGBCCompiler> compiler;
    RefPtr<Interpreter> interpreter;
    std::vector<std::string> searchPaths;
    std::vector<std::weak_ptr<Module>> modules;

    VM(std::vector<std::string> paths,
       const std::string& workingDirectory,
       const std::string& backend)
        : searchPaths(std::move(paths))
    {
        check(
            slang_createGlobalSession(SLANG_API_VERSION, global.writeRef()),
            "Cannot create Slang session");
        compiler = new HGBCCompiler(global);
        ComPtr<rhi::IDevice> device;
        if (backend != "none")
        {
            rhi::DeviceDesc deviceDesc = {};
            deviceDesc.deviceType = getDeviceType(backend);
            deviceDesc.slang.slangGlobalSession = global;
            // Device shaders use the same experimental language features as their host program.
            slang::CompilerOptionEntry experimental = {};
            experimental.name = slang::CompilerOptionName::ExperimentalFeature;
            experimental.value.kind = slang::CompilerOptionValueKind::Int;
            experimental.value.intValue0 = 1;
            deviceDesc.slang.compilerOptionEntries = &experimental;
            deviceDesc.slang.compilerOptionEntryCount = 1;
            check(
                rhi::getRHI()->createDevice(deviceDesc, device.writeRef()),
                String("Cannot create the '") + backend.c_str() +
                    "' RHI device for slangvm. Check that the backend is built and a compatible "
                    "device and driver are available.");
        }
        Desc desc;
        // Interpreter retains the device and drains its work before releasing it on close.
        desc.device = device;
        desc.workingDirectory = workingDirectory.c_str();
        interpreter = new Interpreter(desc);
    }

    void requireOpen() const
    {
        if (!interpreter || interpreter->isClosed())
            throw std::runtime_error("The slangvm VM is closed");
    }

    void close();
};

struct Module
{
    std::shared_ptr<VM> vm;
    RefPtr<Program> program;
    std::vector<std::string> functionNames;
};

void VM::close()
{
    if (!interpreter)
        return;
    String error;
    SlangResult result = interpreter->close(error);
    if (interpreter->isClosed())
    {
        for (auto& weakModule : modules)
            if (auto module = weakModule.lock())
                module->program = nullptr;
        modules.clear();
        interpreter = nullptr;
        compiler = nullptr;
        global = nullptr;
    }
    check(result, error);
}

std::shared_ptr<Module> load(const std::shared_ptr<VM>& vm, const std::string& path)
{
    vm->requireOpen();
    List<const char*> paths;
    for (const auto& searchPath : vm->searchPaths)
        paths.add(searchPath.c_str());
    ComPtr<slang::IBlob> code;
    String error;
    check(
        vm->compiler->compile(
            path.c_str(),
            nullptr,
            paths.getBuffer(),
            paths.getCount(),
            SLANG_DEBUG_INFO_LEVEL_NONE,
            SLANG_DIAGNOSTIC_COLOR_NEVER,
            code,
            error),
        error);
    auto module = std::make_shared<Module>();
    module->vm = vm;
    check(
        Program::load(
            code->getBufferPointer(),
            code->getBufferSize(),
            path,
            module->program,
            error),
        error);
    uint64_t programId = 0;
    check(vm->interpreter->prepareProgram(module->program, programId, error), error);
    for (const auto& entry : module->program->getIndex().exports)
        module->functionNames.push_back(entry.first);
    vm->modules.push_back(module);
    return module;
}

/// Stores scalar bytes without aliasing C++ scalar objects through incompatible pointer types.
struct ScalarStorage
{
    uint8_t bytes[sizeof(double)] = {};

    template<typename T>
    void set(T value)
    {
        static_assert(sizeof(T) <= sizeof(bytes));
        std::memcpy(bytes, &value, sizeof(value));
    }

    template<typename T>
    T get() const
    {
        T value;
        std::memcpy(&value, bytes, sizeof(value));
        return value;
    }
};

void overflow()
{
    PyErr_SetString(PyExc_OverflowError, "Value is outside the declared Slang scalar range");
    throw nb::python_error();
}

ScalarStorage encode(nb::handle value, ScalarType type)
{
    ScalarStorage result;
    if (type == ScalarType::Bool)
    {
        if (!PyBool_Check(value.ptr()))
            throw nb::type_error("A Slang bool parameter requires a Python bool");
        result.set<uint8_t>(value.ptr() == Py_True);
        return result;
    }
    if (PyBool_Check(value.ptr()))
        throw nb::type_error("A numeric Slang parameter does not accept Python bool");
    if (type == ScalarType::Int32 || type == ScalarType::UInt32)
    {
        if (!PyLong_Check(value.ptr()))
            throw nb::type_error("A Slang integer parameter requires a Python int");
        long long integer = PyLong_AsLongLong(value.ptr());
        if (PyErr_Occurred())
            throw nb::python_error();
        if (type == ScalarType::Int32)
        {
            if (integer < INT32_MIN || integer > INT32_MAX)
                overflow();
            result.set<int32_t>(int32_t(integer));
        }
        else
        {
            if (integer < 0 || uint64_t(integer) > UINT32_MAX)
                overflow();
            result.set<uint32_t>(uint32_t(integer));
        }
        return result;
    }
    if (!PyFloat_Check(value.ptr()) && !PyLong_Check(value.ptr()))
        throw nb::type_error("A Slang floating-point parameter requires a Python float or int");
    double number = PyFloat_AsDouble(value.ptr());
    if (PyErr_Occurred())
        throw nb::python_error();
    switch (type)
    {
    case ScalarType::Float64:
        result.set(number);
        break;
    case ScalarType::Float32:
    case ScalarType::Float16:
        {
            if (std::isfinite(number) && std::abs(number) > std::numeric_limits<float>::max())
                overflow();
            float narrowed = float(number);
            if (type == ScalarType::Float32)
                result.set(narrowed);
            else
            {
                uint16_t bits = FloatToHalf(narrowed);
                if (std::isfinite(number) && !std::isfinite(HalfToFloat(bits)))
                    overflow();
                result.set(bits);
            }
            break;
        }
    default:
        throw nb::type_error("Unsupported Slang scalar type");
    }
    return result;
}

nb::object decode(const ScalarStorage& value, ScalarType type)
{
    switch (type)
    {
    case ScalarType::Bool:
        return nb::bool_(value.get<uint8_t>() != 0);
    case ScalarType::Int32:
        return nb::int_(value.get<int32_t>());
    case ScalarType::UInt32:
        return nb::int_(value.get<uint32_t>());
    case ScalarType::Float16:
        return nb::float_(HalfToFloat(value.get<uint16_t>()));
    case ScalarType::Float32:
        return nb::float_(value.get<float>());
    case ScalarType::Float64:
        return nb::float_(value.get<double>());
    default:
        throw nb::type_error("Unsupported Slang scalar result");
    }
}

struct PythonFunction
{
    std::shared_ptr<VM> vm;
    RefPtr<BoundFunction> bound;
    std::string name;

    nb::object call(nb::args args)
    {
        vm->requireOpen();
        auto parameters = bound->getParameterTypes();
        if (args.size() != size_t(parameters.getCount()))
            throw nb::type_error("Argument count does not match the Slang function signature");
        List<Argument> arguments;
        for (Index i = 0; i < parameters.getCount(); ++i)
        {
            const auto& type = parameters[i];
            if (type.kind == HostValueType::Kind::Scalar)
            {
                auto storage = encode(args[size_t(i)], type.scalarType);
                arguments.add(Argument::fromScalar(type.scalarType, storage.bytes, type.byteSize));
            }
            else
            {
                if (!nb::isinstance<Value>(args[size_t(i)]))
                    throw nb::type_error("A non-scalar Slang parameter requires a slangvm.Value");
                arguments.add(Argument::fromValue(nb::cast<const Value&>(args[size_t(i)])));
            }
        }
        Value result;
        String error;
        // Keep the GIL during the synchronous API. All Python access and the VM invocation
        // are serialized, and no Python callbacks or background execution are installed.
        checkInvocation(
            vm->interpreter->invoke(bound, arguments.getArrayView(), result, error),
            error);
        if (result.isVoid())
            return nb::none();
        const auto& type = result.getType();
        if (type.kind != HostValueType::Kind::Scalar)
            return nb::cast(std::move(result));
        ScalarStorage storage;
        checkInvocation(
            result.getScalarData(type.scalarType, storage.bytes, type.byteSize, error),
            error);
        return decode(storage, type.scalarType);
    }
};

std::shared_ptr<PythonFunction> findFunction(
    const std::shared_ptr<Module>& module,
    const std::string& name)
{
    module->vm->requireOpen();
    auto source = module->program->findFunctionExport(name);
    if (!source)
        throw nb::attribute_error(("No Slang function named " + name).c_str());
    auto function = std::make_shared<PythonFunction>();
    function->vm = module->vm;
    function->name = name;
    String error;
    checkInvocation(
        module->vm->interpreter->bindFunction(module->program, source->id, function->bound, error),
        error);
    return function;
}
} // namespace

NB_MODULE(_slangvm, m)
{
    nb::class_<Value>(m, "Value")
        .def_prop_ro(
            "type_name",
            [](const Value& value) { return std::string(value.getType().name.getBuffer()); })
        .def(
            "__repr__",
            [](const Value& value)
            {
                return std::string("slangvm.Value(") + value.getType().name.getBuffer() +
                       (value.isValid() ? ")" : ", closed)");
            });
    nb::class_<VM>(m, "VM")
        .def(
            nb::init<std::vector<std::string>, const std::string&, const std::string&>(),
            nb::arg("search_paths"),
            nb::arg("working_directory"),
            nb::arg("backend") = "none")
        .def("load", &load)
        .def("close", &VM::close)
        .def_prop_ro(
            "is_closed",
            [](const VM& vm) { return !vm.interpreter || vm.interpreter->isClosed(); });
    nb::class_<Module>(m, "Module")
        .def("function", &findFunction)
        .def_prop_ro("functions", [](const Module& module) { return module.functionNames; });
    nb::class_<PythonFunction>(m, "Function")
        .def("__call__", &PythonFunction::call)
        .def_prop_ro("name", [](const PythonFunction& function) { return function.name; });
}
