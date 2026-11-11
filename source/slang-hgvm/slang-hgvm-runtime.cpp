// slang-hgvm-runtime.cpp
#include "slang-hgbc-validation.h"
#include "slang-hgvm-interpreter.h"
#include "slang-hgvm-types.h"
#include "slang-hgvm.h"

namespace Slang::HGVM
{
namespace
{

SlangResult fail(String& outError, const char* message)
{
    outError = message;
    return SLANG_FAIL;
}

SlangResult fail(String& outError, const std::string& message)
{
    outError = message.c_str();
    return SLANG_FAIL;
}

InterpreterCore::Desc makeInterpreterCoreDesc(const Desc& desc)
{
    InterpreterCore::Desc result;
    result.device = desc.device;
    result.fileSystem = desc.fileSystem;
    result.workingDirectory = desc.workingDirectory;
    result.outputSink = desc.outputSink;
    result.hostServices = desc.hostServices;
    result.gpuExecution = desc.gpuExecution;
    result.bufferPoolByteLimit = desc.bufferPoolByteLimit;
    result.enableEventTracking = desc.enableEventTracking;
    result.eventSink = desc.eventSink;
    result.stepControl = desc.stepControl;
    result.debugHandler = desc.debugHandler;
    return result;
}

} // namespace

/// Returns the width-independent physical layout associated with a staged scalar code.
///
/// For example, `Float16` receives a two-byte size and alignment here, which both aggregate layout
/// and the evaluator's `Float16Value` representation use.
ScalarLayout getScalarLayout(ScalarType type)
{
    switch (type)
    {
    case ScalarType::Bool:
        return {1, 1, 1};
    case ScalarType::Int4:
    case ScalarType::UInt4:
    case ScalarType::Float4E2M1:
        return {4, 1, 1};
    case ScalarType::Int8:
    case ScalarType::UInt8:
    case ScalarType::Float8E4M3:
    case ScalarType::Float8E5M2:
        return {8, 1, 1};
    case ScalarType::Int16:
    case ScalarType::UInt16:
    case ScalarType::Float16:
    case ScalarType::BFloat16:
        return {16, 2, 2};
    case ScalarType::Int32:
    case ScalarType::UInt32:
    case ScalarType::Float32:
        return {32, 4, 4};
    case ScalarType::Int64:
    case ScalarType::UInt64:
    case ScalarType::Float64:
        return {64, 8, 8};
    }
    SLANG_UNREACHABLE("unknown HGVM scalar type");
}

SlangResult IHGVMHostServices::createWindow(
    std::string_view title,
    uint32_t width,
    uint32_t height,
    RefPtr<HGVMHostObject>& outWindow,
    String& outError)
{
    SLANG_UNUSED(title);
    SLANG_UNUSED(width);
    SLANG_UNUSED(height);
    outWindow = nullptr;
    return fail(outError, "HGVM window creation is unavailable in this host");
}

SlangResult IHGVMHostServices::processWindowEvents(
    HGVMHostObject* window,
    bool& outOpen,
    String& outError)
{
    SLANG_UNUSED(window);
    outOpen = false;
    return fail(outError, "HGVM window event processing is unavailable in this host");
}

SlangResult IHGVMHostServices::pollInputEvent(
    HGVMHostObject* window,
    HGVMInputEvent& outEvent,
    String& outError)
{
    SLANG_UNUSED(window);
    outEvent = {};
    return fail(outError, "HGVM window input polling is unavailable in this host");
}

SlangResult IHGVMHostServices::closeWindow(HGVMHostObject* window, String& outError)
{
    SLANG_UNUSED(window);
    return fail(outError, "HGVM window closing is unavailable in this host");
}

SlangResult IHGVMHostServices::setWindowTitle(
    HGVMHostObject* window,
    std::string_view title,
    String& outError)
{
    SLANG_UNUSED(window);
    SLANG_UNUSED(title);
    return fail(outError, "HGVM window title updates are unavailable in this host");
}

SlangResult IHGVMHostServices::createSurface(
    HGVMHostObject* window,
    rhi::Format requestedFormat,
    uint32_t imageCount,
    RefPtr<HGVMHostObject>& outSurface,
    String& outError)
{
    SLANG_UNUSED(window);
    SLANG_UNUSED(requestedFormat);
    SLANG_UNUSED(imageCount);
    outSurface = nullptr;
    return fail(outError, "HGVM surface creation is unavailable in this host");
}

SlangResult IHGVMHostServices::resizeSurface(
    HGVMHostObject* surface,
    uint32_t width,
    uint32_t height,
    String& outError)
{
    SLANG_UNUSED(surface);
    SLANG_UNUSED(width);
    SLANG_UNUSED(height);
    return fail(outError, "HGVM surface resizing is unavailable in this host");
}

SlangResult IHGVMHostServices::getSurfaceDimensions(
    HGVMHostObject* surface,
    uint32_t& outWidth,
    uint32_t& outHeight,
    String& outError)
{
    SLANG_UNUSED(surface);
    outWidth = outHeight = 0;
    return fail(outError, "HGVM surface dimensions are unavailable in this host");
}

SlangResult IHGVMHostServices::waitForSurface(HGVMHostObject* surface, String& outError)
{
    SLANG_UNUSED(surface);
    return fail(outError, "HGVM surface waiting is unavailable in this host");
}

SlangResult IHGVMHostServices::acquireSurfaceTexture(
    HGVMHostObject* surface,
    ComPtr<rhi::ITexture>& outTexture,
    String& outError)
{
    SLANG_UNUSED(surface);
    outTexture = nullptr;
    return fail(outError, "HGVM surface acquisition is unavailable in this host");
}

SlangResult IHGVMHostServices::presentSurface(HGVMHostObject* surface, String& outError)
{
    SLANG_UNUSED(surface);
    return fail(outError, "HGVM surface presentation is unavailable in this host");
}

SlangResult IHGVMHostServices::blitTexture2D(
    rhi::ITexture* destination,
    rhi::ITexture* source,
    String& outError)
{
    SLANG_UNUSED(destination);
    SLANG_UNUSED(source);
    return fail(outError, "HGVM texture blitting is unavailable in this host");
}

SlangResult Program::load(
    const void* data,
    size_t size,
    std::string name,
    RefPtr<Program>& outProgram,
    String& outError)
{
    outProgram = nullptr;
    if (!data || !size)
        return fail(outError, "HGBC program data is empty");

    RefPtr<Program> program = new Program();
    program->m_name = std::move(name);
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    program->m_data.assign(bytes, bytes + size);

    std::string parseError;
    if (!HGBC::readProgram(
            program->m_data.data(),
            program->m_data.size(),
            program->m_decodedProgram,
            &parseError))
    {
        return fail(outError, "Failed to parse HGBC program: " + parseError);
    }
    SLANG_RETURN_ON_FAIL(validateProgram(*program, outError));
    outProgram = std::move(program);
    return SLANG_OK;
}

const Function* Program::findFunction(HGBCId id) const
{
    auto it = getIndex().functionIndices.find(id);
    return it == getIndex().functionIndices.end() ? nullptr
                                                  : &m_decodedProgram.functions[it->second];
}

const Function* Program::findFunctionExport(std::string_view name) const
{
    auto it = getIndex().exports.find(std::string(name));
    return it == getIndex().exports.end() ? nullptr : findFunction(it->second);
}

bool Program::tryGetScalarType(HGBCId typeId, ScalarType& outType) const
{
    const HGBC::BasicIndex& index = getIndex();
    if (index.uintTypeIds.contains(typeId))
    {
        outType = ScalarType::UInt32;
        return true;
    }
    if (index.intTypeIds.contains(typeId))
    {
        outType = ScalarType::Int32;
        return true;
    }
    if (index.float16TypeIds.contains(typeId))
    {
        outType = ScalarType::Float16;
        return true;
    }
    if (index.floatTypeIds.contains(typeId))
    {
        outType = index.float64TypeIds.contains(typeId) ? ScalarType::Float64 : ScalarType::Float32;
        return true;
    }
    if (index.boolTypeIds.contains(typeId))
    {
        outType = ScalarType::Bool;
        return true;
    }
    return false;
}

SlangResult Program::getHostValueSize(HGBCId typeId, size_t& outSize, String& outError) const
{
    ScalarType scalar;
    if (tryGetScalarType(typeId, scalar))
    {
        outSize = getScalarLayout(scalar).byteSize;
        return SLANG_OK;
    }
    auto it = getIndex().resultRecords.find(typeId);
    if (it != getIndex().resultRecords.end() &&
        m_decodedProgram.records[it->second].opcode == HGBCOpcode::VoidType)
    {
        outSize = 0;
        return SLANG_OK;
    }
    return fail(
        outError,
        "HGVM host invocation currently supports only scalar values and void results");
}

SlangResult Program::getFunctionSignature(
    HGBCId functionId,
    FunctionSignature& outSignature,
    String& outError) const
{
    outSignature = {};
    const auto function = findFunction(functionId);
    if (!function)
        return fail(outError, "HGVM function ID does not belong to this program");
    const auto& type = m_decodedProgram.records[getIndex().resultRecords.at(function->typeId)];
    SLANG_ASSERT(type.opcode == HGBCOpcode::FuncType && type.operands.size() >= 2);
    outSignature.resultType = type.operands[1];
    for (size_t i = 2; i < type.operands.size(); ++i)
        outSignature.parameterTypes.add(type.operands[i]);
    return SLANG_OK;
}

const HGBC::TypeSizeDescriptor* Program::findTypeSize(HGBCId typeId) const
{
    auto it = m_decodedProgram.typeSizeIndices.find(typeId);
    return it == m_decodedProgram.typeSizeIndices.end() ? nullptr
                                                        : &m_decodedProgram.typeSizes[it->second];
}

const HGBC::StructTypeDescriptor* Program::findStructType(HGBCId typeId) const
{
    auto it = m_decodedProgram.structTypeIndices.find(typeId);
    return it == m_decodedProgram.structTypeIndices.end()
               ? nullptr
               : &m_decodedProgram.structTypes[it->second];
}

const HGBC::ClassTypeDescriptor* Program::findClassType(HGBCId typeId) const
{
    auto it = m_decodedProgram.classTypeIndices.find(typeId);
    return it == m_decodedProgram.classTypeIndices.end() ? nullptr
                                                         : &m_decodedProgram.classTypes[it->second];
}

const HGBC::TaggedUnionTypeDescriptor* Program::findTaggedUnionType(HGBCId typeId) const
{
    return HGBC::findTaggedUnionType(m_decodedProgram, typeId);
}

const std::string* Program::findString(HGBCId id) const
{
    auto it = getIndex().strings.find(id);
    return it == getIndex().strings.end() ? nullptr : &it->second;
}

std::optional<std::string> Program::findStringDecoration(HGBCId targetId, HGBCDecorationKind kind)
    const
{
    auto targetIt = m_decodedProgram.decorationRecordIndices.find(targetId);
    if (targetIt == m_decodedProgram.decorationRecordIndices.end())
        return {};
    auto kindIt = targetIt->second.find(uint32_t(kind));
    if (kindIt == targetIt->second.end() || kindIt->second.empty())
        return {};
    const HGBC::Record& record = m_decodedProgram.records[kindIt->second.front()];
    if (record.operands.size() < 4)
        return {};
    const std::string* value = findString(record.operands[3]);
    return value ? std::optional<std::string>(*value) : std::optional<std::string>();
}

std::optional<std::string> Program::getSlangTypeName(HGBCId typeId) const
{
    auto recordIt = getIndex().resultRecords.find(typeId);
    if (recordIt == getIndex().resultRecords.end())
        return std::nullopt;
    const HGBC::Record& record = getDecodedProgram().records[recordIt->second];
    switch (record.opcode)
    {
    case HGBCOpcode::UIntType:
        return "uint";
    case HGBCOpcode::IntType:
        return "int";
    case HGBCOpcode::Float16Type:
        return "half";
    case HGBCOpcode::Float32Type:
        return "float";
    case HGBCOpcode::Float64Type:
        return "double";
    case HGBCOpcode::BoolType:
        return "bool";
    case HGBCOpcode::VoidType:
        return "void";
    case HGBCOpcode::StringType:
        return "String";
    case HGBCOpcode::TimerType:
        return "Timer";
    case HGBCOpcode::ByteStreamType:
        return "ByteStream";
    case HGBCOpcode::WindowType:
        return "Window";
    case HGBCOpcode::SurfaceType:
        return "Surface";
    case HGBCOpcode::RaytracingPipelineType:
        return "RaytracingPipeline";
    case HGBCOpcode::RaytracingAccelerationStructureType:
        return "RaytracingAccelerationStructure";
    case HGBCOpcode::RaytracingBottomLevelAccelerationStructureType:
        return "RaytracingBottomLevelAccelerationStructure";
    case HGBCOpcode::DescriptorHandleType:
        {
            if (auto name = findStringDecoration(typeId, HGBCDecorationKind::SlangTypeName))
                return name;
            auto resourceName = getSlangTypeName(record.operands[1]);
            if (!resourceName)
                return std::nullopt;
            return "DescriptorHandle<" + *resourceName + ">";
        }
    case HGBCOpcode::StructuredBufferType:
    case HGBCOpcode::RWStructuredBufferType:
    case HGBCOpcode::Texture2DType:
    case HGBCOpcode::RenderTarget2DType:
        {
            auto elementName = getSlangTypeName(record.operands[1]);
            if (!elementName)
                return std::nullopt;
            const char* name =
                record.opcode == HGBCOpcode::StructuredBufferType     ? "StructuredBuffer"
                : record.opcode == HGBCOpcode::RWStructuredBufferType ? "RWStructuredBuffer"
                : record.opcode == HGBCOpcode::Texture2DType          ? "Texture2D"
                                                                      : "RenderTarget2D";
            return std::string(name) + "<" + *elementName + ">";
        }
    case HGBCOpcode::VectorType:
        {
            if (record.operands.size() != 3)
                return std::nullopt;
            auto elementName = getSlangTypeName(record.operands[2]);
            if (!elementName)
                return std::nullopt;
            return "vector<" + *elementName + "," + std::to_string(record.operands[1]) + ">";
        }
    case HGBCOpcode::MatrixType:
        {
            if (record.operands.size() != 5)
                return std::nullopt;
            auto elementName = getSlangTypeName(record.operands[4]);
            if (!elementName)
                return std::nullopt;
            return "matrix<" + *elementName + "," + std::to_string(record.operands[1]) + "," +
                   std::to_string(record.operands[2]) + ">";
        }
    case HGBCOpcode::ArrayType:
        {
            if (record.operands.size() != 3)
                return std::nullopt;
            auto elementName = getSlangTypeName(record.operands[2]);
            if (!elementName)
                return std::nullopt;
            return "Array<" + *elementName + "," + std::to_string(record.operands[1]) + ">";
        }
    case HGBCOpcode::RuntimeArrayType:
        {
            if (record.operands.size() != 2)
                return std::nullopt;
            auto elementName = getSlangTypeName(record.operands[1]);
            if (!elementName)
                return std::nullopt;
            return "RuntimeArray<" + *elementName + ">";
        }
    case HGBCOpcode::StructType:
    case HGBCOpcode::ClassType:
        return findStringDecoration(typeId, HGBCDecorationKind::SlangTypeName);
    default:
        return std::nullopt;
    }
}

struct Interpreter::Impl
{
    explicit Impl(const Desc& desc)
        : interpreterCore(makeInterpreterCoreDesc(desc))
    {
    }

    InterpreterCore interpreterCore;
};

Interpreter::Interpreter(const Desc& desc)
    : m_impl(new Impl(desc))
{
}

Interpreter::~Interpreter() = default;

SlangResult Interpreter::bindFunction(
    Program* program,
    HGBCId functionId,
    RefPtr<BoundFunction>& outFunction,
    String& outError)
{
    return m_impl->interpreterCore.bindFunction(program, functionId, outFunction, outError);
}

SlangResult Interpreter::invoke(
    const BoundFunction* function,
    ConstArrayView<Argument> arguments,
    Value& outResult,
    String& outError)
{
    return m_impl->interpreterCore.invoke(function, arguments, outResult, outError);
}

SlangResult Interpreter::close(String& outError)
{
    return m_impl->interpreterCore.close(outError);
}

bool Interpreter::isClosed() const
{
    return m_impl->interpreterCore.isClosed();
}

SlangResult Interpreter::prepareProgram(Program* program, uint64_t& id, String& error)
{
    return m_impl->interpreterCore.prepareProgram(program, id, error);
}

DebugMap* Interpreter::getDebugMap(uint64_t id) const
{
    return m_impl->interpreterCore.getDebugMap(id);
}

void Interpreter::setBreakpoint(DebugInstructionAddress address, bool enabled)
{
    m_impl->interpreterCore.setBreakpoint(address, enabled);
}

const StagedRecord::Header* Interpreter::getRecord(DebugInstructionAddress address) const
{
    return m_impl->interpreterCore.getRecord(address);
}

BufferPoolStats Interpreter::getBufferPoolStats() const
{
    return m_impl->interpreterCore.getBufferPoolStats();
}

bool Interpreter::readDebugMemory(
    DebugCallId call,
    Address address,
    uint32_t offset,
    void* data,
    uint32_t size)
{
    return m_impl->interpreterCore.readDebugMemory(call, address, offset, data, size);
}

bool Interpreter::getDebugObject(
    DebugCallId call,
    Address object,
    HGBCId& type,
    Address& data,
    uint32_t& count)
{
    return m_impl->interpreterCore.getDebugObject(call, object, type, data, count);
}

bool Interpreter::readDebugString(DebugCallId call, Address address, uint32_t offset, String& text)
{
    return m_impl->interpreterCore.readDebugString(call, address, offset, text);
}

bool Interpreter::getDebugBufferInfo(DebugCallId call, Address object, DebugBufferInfo& info)
{
    return m_impl->interpreterCore.getDebugBufferInfo(call, object, info);
}

SlangResult Interpreter::readDebugBufferElements(
    DebugCallId call,
    Address object,
    uint32_t start,
    uint32_t count,
    List<uint8_t>& bytes,
    String& error)
{
    return m_impl->interpreterCore
        .readDebugBufferElements(call, object, start, count, bytes, error);
}

bool Interpreter::getDebugTextureInfo(DebugCallId call, Address object, TextureInfo& info)
{
    return m_impl->interpreterCore.getDebugTextureInfo(call, object, info);
}

SlangResult Interpreter::exportDebugTextureMip(
    DebugCallId call,
    Address object,
    uint32_t mip,
    SequentialByteWriter& destination,
    String& error)
{
    return m_impl->interpreterCore.exportDebugTextureMip(call, object, mip, destination, error);
}

bool Interpreter::getDebugObjectProperties(
    DebugCallId call,
    Address object,
    HeapObjectKind expectedKind,
    List<DebugObjectProperty>& properties)
{
    return m_impl->interpreterCore.getDebugObjectProperties(call, object, expectedKind, properties);
}

SlangResult Interpreter::trimBufferPool(String& outError)
{
    return m_impl->interpreterCore.trimBufferPool(outError);
}

SlangResult Interpreter::call(Program* program, std::string_view name, String& outError)
{
    if (!program)
        return fail(outError, "HGVM call requires a program");
    const Function* function = program->findFunctionExport(name);
    if (!function)
    {
        return fail(
            outError,
            "HGBC program has no exported function named \"" + std::string(name) + "\"");
    }
    RefPtr<BoundFunction> bound;
    SLANG_RETURN_ON_FAIL(bindFunction(program, function->id, bound, outError));
    Value ignoredResult;
    return invoke(bound, {}, ignoredResult, outError);
}

SlangResult Interpreter::invoke(
    Program* program,
    HGBCId functionId,
    ConstArrayView<ValueView> arguments,
    MutableValueView result,
    String& outError)
{
    outError = "";
    if (!program)
        return fail(outError, "HGVM invocation requires a program");
    FunctionSignature signature;
    SLANG_RETURN_ON_FAIL(program->getFunctionSignature(functionId, signature, outError));
    if (arguments.getCount() != signature.parameterTypes.getCount())
        return fail(
            outError,
            "HGVM invocation argument count does not match the function signature");
    List<Argument> ownedArguments;
    for (Index i = 0; i < arguments.getCount(); ++i)
    {
        const auto& argument = arguments[i];
        size_t size = 0;
        SLANG_RETURN_ON_FAIL(
            program->getHostValueSize(signature.parameterTypes[i], size, outError));
        if (!size || argument.typeId != signature.parameterTypes[i] || argument.size != size ||
            !argument.data)
            return fail(
                outError,
                "HGVM invocation argument type or storage does not match the signature");
        ScalarType scalar;
        SLANG_RELEASE_ASSERT(program->tryGetScalarType(argument.typeId, scalar));
        if (scalar == ScalarType::Bool && *static_cast<const uint8_t*>(argument.data) > 1)
            return fail(outError, "HGVM boolean arguments must contain zero or one");
        ownedArguments.add(Argument::fromScalar(scalar, argument.data, size));
    }
    size_t resultSize = 0;
    SLANG_RETURN_ON_FAIL(program->getHostValueSize(signature.resultType, resultSize, outError));
    if (result.typeId != signature.resultType || result.size != resultSize ||
        (resultSize && !result.data))
        return fail(
            outError,
            "HGVM invocation result type or storage does not match the signature");
    RefPtr<BoundFunction> bound;
    SLANG_RETURN_ON_FAIL(bindFunction(program, functionId, bound, outError));
    Value ownedResult;
    SLANG_RETURN_ON_FAIL(invoke(bound, ownedArguments.getArrayView(), ownedResult, outError));
    if (!resultSize)
        return SLANG_OK;
    ScalarType scalar;
    SLANG_RELEASE_ASSERT(program->tryGetScalarType(signature.resultType, scalar));
    return ownedResult.getScalarData(scalar, result.data, result.size, outError);
}

void Interpreter::releaseProgram(Program* program)
{
    if (program)
        m_impl->interpreterCore.releaseProgram(program);
}

} // namespace Slang::HGVM
