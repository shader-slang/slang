#include "hgvm-test-helpers.h"
#include "slang-hgvm/slang-hgvm.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

enum RayValidationId : HGBCId
{
    UIntType = 1,
    FloatType,
    VoidType,
    StringType,
    ExtentType,
    FunctionType,
    PipelineType,
    AnotherPipelineType,
    UIntPointerType,
    PipelinePointerType,
    ExtentPointerType,
    UIntValue,
    FloatValue,
    Name,
    Binary,
    NullPipeline,
    Module,
    EntryPoint,
    SpecializedEntryPoint,
    HitGroup,
    Function,
    Block,
    Pipeline,
    Extent,
    UIntAddress,
    PipelineAddress,
    ExtentAddress,
};

HGBC::Record& appendRayValidationRecord(
    HGBC::Program& program,
    HGBCSectionTag sectionTag,
    HGBCOpcode opcode,
    HGBC::Vector<uint32_t> operands)
{
    if (program.sections.empty() || program.sections.back().tag != sectionTag)
    {
        HGBC::Section section;
        section.tag = sectionTag;
        program.sections.add(section);
    }
    program.sections.back().recordIndices.add(program.records.size());
    HGBC::Record record;
    record.opcode = opcode;
    record.operands = std::move(operands);
    program.records.add(std::move(record));
    return program.records.back();
}

/// Builds valid wire metadata without loading a shader module or requiring a graphics device.
/// The embedded module bytes are intentionally empty: semantic HGBC validation checks entry-point
/// references, while shader reflection checks the actual entry-point stages at execution time.
HGBC::Program makeRayValidationProgram()
{
    HGBC::Program program;
    const auto type = HGBCSectionTag::TYPE;
    appendRayValidationRecord(program, type, HGBCOpcode::UIntType, {UIntType});
    appendRayValidationRecord(program, type, HGBCOpcode::Float32Type, {FloatType});
    appendRayValidationRecord(program, type, HGBCOpcode::VoidType, {VoidType});
    appendRayValidationRecord(program, type, HGBCOpcode::StringType, {StringType});
    appendRayValidationRecord(program, type, HGBCOpcode::VectorType, {ExtentType, 3, UIntType});
    appendRayValidationRecord(program, type, HGBCOpcode::FuncType, {FunctionType, VoidType});
    appendRayValidationRecord(program, type, HGBCOpcode::RaytracingPipelineType, {PipelineType});
    appendRayValidationRecord(
        program,
        type,
        HGBCOpcode::RaytracingPipelineType,
        {AnotherPipelineType});
    appendRayValidationRecord(program, type, HGBCOpcode::PtrType, {UIntPointerType, UIntType});
    appendRayValidationRecord(
        program,
        type,
        HGBCOpcode::PtrType,
        {PipelinePointerType, PipelineType});
    appendRayValidationRecord(program, type, HGBCOpcode::PtrType, {ExtentPointerType, ExtentType});

    const auto literal = HGBCSectionTag::LITR;
    appendRayValidationRecord(program, literal, HGBCOpcode::IntLit, {UIntType, UIntValue, 1});
    appendRayValidationRecord(program, literal, HGBCOpcode::FloatLit, {FloatType, FloatValue, 0});
    appendRayValidationRecord(program, literal, HGBCOpcode::StringLit, {StringType, Name, 1})
        .payload = {'s'};
    appendRayValidationRecord(program, literal, HGBCOpcode::BinaryLit, {Binary, 0});
    appendRayValidationRecord(program, literal, HGBCOpcode::NullLit, {PipelineType, NullPipeline});

    const auto embedded = HGBCSectionTag::EMBD;
    appendRayValidationRecord(program, embedded, HGBCOpcode::SlangModule, {Module, Name, Binary});
    appendRayValidationRecord(
        program,
        embedded,
        HGBCOpcode::SlangEntryPoint,
        {EntryPoint, Module, Name});
    appendRayValidationRecord(
        program,
        embedded,
        HGBCOpcode::SpecializeSlangEntryPoint,
        {SpecializedEntryPoint, EntryPoint});
    appendRayValidationRecord(
        program,
        embedded,
        HGBCOpcode::RayHitGroup,
        {HitGroup,
         uint32_t(HGBCRayGeometryKind::Triangles),
         UIntType,
         UIntType,
         uint32_t(HGBCRayStageRole::ClosestHit),
         EntryPoint,
         uint32_t(HGBCRayStageRole::AnyHit),
         SpecializedEntryPoint});

    const auto function = HGBCSectionTag::FUNC;
    appendRayValidationRecord(program, function, HGBCOpcode::Func, {FunctionType, Function});
    appendRayValidationRecord(program, function, HGBCOpcode::Block, {Block});
    appendRayValidationRecord(program, function, HGBCOpcode::Var, {UIntPointerType, UIntAddress});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::Var,
        {PipelinePointerType, PipelineAddress});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::Var,
        {ExtentPointerType, ExtentAddress});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::MakeRaytracingPipeline,
        {PipelineType, Pipeline, UIntValue});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::MakeVector,
        {ExtentType, Extent, UIntValue, UIntValue, UIntValue});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::SetRayMiss,
        {Pipeline, UIntValue, EntryPoint, UIntType, 4, UIntType, UIntValue});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::SetRayHitGroup,
        {Pipeline,
         UIntValue,
         UIntValue,
         HitGroup,
         uint32_t(HGBCRayStageRole::AnyHit),
         1,
         UIntType,
         UIntValue,
         uint32_t(HGBCRayStageRole::ClosestHit),
         1,
         UIntType,
         UIntValue});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::DispatchRays,
        {SpecializedEntryPoint, Extent, 1, 7, AnotherPipelineType, Pipeline});
    appendRayValidationRecord(program, function, HGBCOpcode::Return, {});
    appendRayValidationRecord(program, function, HGBCOpcode::BlockEnd, {});
    appendRayValidationRecord(program, function, HGBCOpcode::FunctionEnd, {});
    return program;
}

HGBC::Record& findRayValidationRecord(HGBC::Program& program, HGBCOpcode opcode)
{
    for (auto& record : program.records)
        if (record.opcode == opcode)
            return record;
    SLANG_UNEXPECTED("ray validation fixture is missing a record");
}

void checkRayValidation(const HGBC::Program& source, const char* expectedError = nullptr)
{
    HGBC::Vector<uint8_t> bytes;
    SLANG_CHECK_ABORT(HGBC::writeProgram(source, bytes));
    RefPtr<HGVM::Program> program;
    String error;
    SlangResult result =
        HGVM::Program::load(bytes.data(), bytes.size(), "ray-validation", program, error);
    if (expectedError)
    {
        SLANG_CHECK(SLANG_FAILED(result));
        SLANG_CHECK(error.contains(expectedError));
    }
    else
    {
        if (SLANG_FAILED(result))
            fprintf(stderr, "%s\n", error.getBuffer());
        SLANG_CHECK(SLANG_SUCCEEDED(result));
    }
}

enum AccelerationStructureValidationId : HGBCId
{
    ASBottomLevelType = 100,
    ASTopLevelType,
    ASHandleType,
    ASFloat3Type,
    ASTransformType,
    ASVerticesType,
    ASIndicesType,
    ASInputType,
    ASInputsType,
    ASNullTopLevel,
    ASFunction,
    ASBlock,
    ASStructure,
    ASInputs,
    ASHandle,
    ASResource,
    ASFieldNameBegin,
};

/// Uses named ordinary structs for the two build input ABIs. Deliberately reverse the fields so
/// validation cannot accidentally identify vertex buffers, BLAS handles, or transforms by order.
HGBC::Program makeAccelerationStructureValidationProgram(bool bottomLevel)
{
    auto program = makeRayValidationProgram();
    const auto type = HGBCSectionTag::TYPE;
    appendRayValidationRecord(
        program,
        type,
        HGBCOpcode::RaytracingBottomLevelAccelerationStructureType,
        {ASBottomLevelType});
    appendRayValidationRecord(
        program,
        type,
        HGBCOpcode::RaytracingAccelerationStructureType,
        {ASTopLevelType});
    appendRayValidationRecord(
        program,
        type,
        HGBCOpcode::DescriptorHandleType,
        {ASHandleType, ASTopLevelType});
    appendRayValidationRecord(program, type, HGBCOpcode::VectorType, {ASFloat3Type, 3, FloatType});
    appendRayValidationRecord(
        program,
        type,
        HGBCOpcode::MatrixType,
        {ASTransformType, 3, 4, 0, FloatType});
    appendRayValidationRecord(
        program,
        type,
        HGBCOpcode::StructuredBufferType,
        {ASVerticesType, ASFloat3Type});
    appendRayValidationRecord(
        program,
        type,
        HGBCOpcode::StructuredBufferType,
        {ASIndicesType, UIntType});
    appendRayValidationRecord(
        program,
        type,
        HGBCOpcode::RuntimeArrayType,
        {ASInputsType, ASInputType});
    HGBC::Vector<uint32_t> fields = {ASInputType};
    const char* triangleNames[] = {
        "vertices",
        "indices",
        "vertexOffset",
        "vertexCount",
        "indexOffset",
        "indexCount",
        "flags"};
    const char* instanceNames[] = {"blas", "transform", "instanceID", "mask", "flags"};
    const HGBCId triangleTypes[] =
        {ASVerticesType, ASIndicesType, UIntType, UIntType, UIntType, UIntType, UIntType};
    const HGBCId instanceTypes[] =
        {ASBottomLevelType, ASTransformType, UIntType, UIntType, UIntType};
    size_t fieldCount = bottomLevel ? SLANG_COUNT_OF(triangleNames) : SLANG_COUNT_OF(instanceNames);
    for (size_t i = fieldCount; i-- > 0;)
    {
        fields.add(ASFieldNameBegin + HGBCId(i));
        fields.add(bottomLevel ? triangleTypes[i] : instanceTypes[i]);
    }
    appendRayValidationRecord(program, type, HGBCOpcode::StructType, std::move(fields));
    const auto literal = HGBCSectionTag::LITR;
    for (size_t i = 0; i < fieldCount; ++i)
    {
        const char* name = bottomLevel ? triangleNames[i] : instanceNames[i];
        size_t size = strlen(name);
        auto& record = appendRayValidationRecord(
            program,
            literal,
            HGBCOpcode::StringLit,
            {StringType, ASFieldNameBegin + HGBCId(i), uint32_t(size)});
        record.payload.addRange(reinterpret_cast<const uint8_t*>(name), size);
    }
    appendRayValidationRecord(
        program,
        literal,
        HGBCOpcode::NullLit,
        {ASTopLevelType, ASNullTopLevel});

    const auto function = HGBCSectionTag::FUNC;
    appendRayValidationRecord(program, function, HGBCOpcode::Func, {FunctionType, ASFunction});
    appendRayValidationRecord(program, function, HGBCOpcode::Block, {ASBlock});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::AllocAccelerationStructure,
        {bottomLevel ? ASBottomLevelType : ASTopLevelType,
         ASStructure,
         uint32_t(
             bottomLevel ? HGBCAccelerationStructureKind::BottomLevel
                         : HGBCAccelerationStructureKind::TopLevel)});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::AllocateRuntimeArray,
        {ASInputsType, ASInputs, UIntValue});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::BuildAccelerationStructure,
        {uint32_t(
             bottomLevel ? HGBCAccelerationStructureKind::BottomLevel
                         : HGBCAccelerationStructureKind::TopLevel),
         uint32_t(
             bottomLevel ? HGBCAccelerationStructureInputKind::Triangles
                         : HGBCAccelerationStructureInputKind::Instances),
         ASInputType,
         ASStructure,
         ASInputs,
         UIntValue,
         UIntValue});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::ResourceToDescriptorHandle,
        {ASHandleType, ASHandle, ASNullTopLevel});
    appendRayValidationRecord(
        program,
        function,
        HGBCOpcode::DescriptorHandleToResource,
        {ASTopLevelType, ASResource, ASHandle});
    appendRayValidationRecord(program, function, HGBCOpcode::Return, {});
    appendRayValidationRecord(program, function, HGBCOpcode::BlockEnd, {});
    appendRayValidationRecord(program, function, HGBCOpcode::FunctionEnd, {});
    return program;
}

} // namespace

SLANG_UNIT_TEST(hgvmAccelerationStructureValidation)
{
    for (bool bottomLevel : {true, false})
    {
        const auto source = makeAccelerationStructureValidationProgram(bottomLevel);
        checkRayValidation(source);
        struct Corruption
        {
            HGBCOpcode opcode;
            size_t operandIndex;
            uint32_t value;
            const char* error;
        };
        const Corruption corruptions[] = {
            {HGBCOpcode::AllocAccelerationStructure,
             0,
             PipelineType,
             "allocation result does not match"},
            {HGBCOpcode::BuildAccelerationStructure,
             1,
             uint32_t(
                 bottomLevel ? HGBCAccelerationStructureInputKind::Instances
                             : HGBCAccelerationStructureInputKind::Triangles),
             "build input kind does not match"},
            {HGBCOpcode::BuildAccelerationStructure,
             2,
             UIntType,
             "build inputs must be a runtime array"},
            {HGBCOpcode::BuildAccelerationStructure,
             3,
             NullPipeline,
             "build target does not match"},
            {HGBCOpcode::BuildAccelerationStructure,
             4,
             UIntValue,
             "build inputs must be a runtime array"},
            {HGBCOpcode::BuildAccelerationStructure,
             5,
             FloatValue,
             "build flags and mode must be uint32"},
            {HGBCOpcode::BuildAccelerationStructure,
             6,
             UIntAddress,
             "build flags and mode must be uint32"},
            {HGBCOpcode::DescriptorHandleType,
             1,
             ASBottomLevelType,
             "must reference a structured-buffer, texture, or top-level"},
        };
        for (const auto& corruption : corruptions)
        {
            auto program = source;
            findRayValidationRecord(program, corruption.opcode).operands[corruption.operandIndex] =
                corruption.value;
            checkRayValidation(program, corruption.error);
        }

        auto malformedField = source;
        auto& descriptor = findRayValidationRecord(malformedField, HGBCOpcode::StructType);
        // The reversed fixture puts vertices/BLAS last. Preserve the field name and corrupt its
        // type so the test proves validation checks semantic roles rather than only field count.
        descriptor.operands[descriptor.operands.size() - 1] = UIntType;
        checkRayValidation(
            malformedField,
            bottomLevel ? "triangle vertices must be" : "instance blas must be");

        auto malformedSecondaryField = source;
        auto& secondaryDescriptor =
            findRayValidationRecord(malformedSecondaryField, HGBCOpcode::StructType);
        secondaryDescriptor.operands[secondaryDescriptor.operands.size() - 3] = FloatType;
        checkRayValidation(
            malformedSecondaryField,
            bottomLevel ? "triangle indices must be" : "instance transform must be");

        auto malformedFlags = source;
        findRayValidationRecord(malformedFlags, HGBCOpcode::StructType).operands[2] = FloatType;
        checkRayValidation(
            malformedFlags,
            bottomLevel ? "triangle ranges and flags must be"
                        : "instance ID, mask, and flags must be");

        auto missingField = source;
        findRayValidationRecord(missingField, HGBCOpcode::StructType).operands.resize(3);
        checkRayValidation(missingField, "build input type has an invalid descriptor shape");
    }
}

SLANG_UNIT_TEST(hgvmRayInstructionValidation)
{
    const auto source = makeRayValidationProgram();
    checkRayValidation(source);

    struct Corruption
    {
        HGBCOpcode opcode;
        size_t operandIndex;
        uint32_t value;
        const char* error;
    };
    const Corruption corruptions[] = {
        {HGBCOpcode::MakeRaytracingPipeline, 0, UIntType, "must produce a ray-tracing pipeline"},
        {HGBCOpcode::MakeRaytracingPipeline, 2, FloatValue, "ray-type count must be a uint32"},
        {HGBCOpcode::MakeRaytracingPipeline, 2, UIntAddress, "ray-type count must be a uint32"},
        {HGBCOpcode::SetRayMiss, 0, UIntValue, "requires a ray-tracing pipeline"},
        {HGBCOpcode::SetRayMiss, 0, PipelineAddress, "requires a ray-tracing pipeline"},
        {HGBCOpcode::SetRayMiss, 1, FloatValue, "registration slots must be uint32"},
        {HGBCOpcode::SetRayMiss, 1, UIntAddress, "registration slots must be uint32"},
        {HGBCOpcode::SetRayMiss, 2, UIntValue, "miss shader does not reference an entry point"},
        {HGBCOpcode::SetRayMiss, 3, UIntValue, "references a missing type"},
        {HGBCOpcode::SetRayMiss, 6, FloatValue, "uniform argument value does not match"},
        {HGBCOpcode::SetRayMiss, 6, UIntAddress, "uniform argument value does not match"},
        {HGBCOpcode::SetRayHitGroup, 0, UIntValue, "requires a ray-tracing pipeline"},
        {HGBCOpcode::SetRayHitGroup, 0, PipelineAddress, "requires a ray-tracing pipeline"},
        {HGBCOpcode::SetRayHitGroup, 1, FloatValue, "registration slots must be uint32"},
        {HGBCOpcode::SetRayHitGroup, 1, UIntAddress, "registration slots must be uint32"},
        {HGBCOpcode::SetRayHitGroup, 2, FloatValue, "registration slots must be uint32"},
        {HGBCOpcode::SetRayHitGroup, 2, UIntAddress, "registration slots must be uint32"},
        {HGBCOpcode::SetRayHitGroup, 3, EntryPoint, "does not reference a hit-group descriptor"},
        {HGBCOpcode::SetRayHitGroup,
         4,
         uint32_t(HGBCRayStageRole::Intersection),
         "uniform argument refers to an undeclared hit-group stage"},
        {HGBCOpcode::SetRayHitGroup, 7, FloatValue, "uniform argument value does not match"},
        {HGBCOpcode::SetRayHitGroup, 7, UIntAddress, "uniform argument value does not match"},
        {HGBCOpcode::DispatchRays, 0, UIntValue, "ray-generation shader does not reference"},
        {HGBCOpcode::DispatchRays, 1, UIntValue, "dispatch extent must be a uint3"},
        {HGBCOpcode::DispatchRays, 1, ExtentAddress, "dispatch extent must be a uint3"},
        {HGBCOpcode::DispatchRays, 2, 0, "has an invalid recursion depth"},
        {HGBCOpcode::DispatchRays, 5, UIntValue, "uniform argument value does not match"},
        {HGBCOpcode::DispatchRays, 5, PipelineAddress, "uniform argument value does not match"},
    };
    for (const auto& corruption : corruptions)
    {
        auto program = source;
        findRayValidationRecord(program, corruption.opcode).operands[corruption.operandIndex] =
            corruption.value;
        checkRayValidation(program, corruption.error);
    }

    // An explicitly declared address argument still has coherent HGBC metadata. Whether a
    // runtime can preserve its lifetime is a capability check, not an implicit value conversion.
    auto addressArgument = source;
    auto& miss = findRayValidationRecord(addressArgument, HGBCOpcode::SetRayMiss);
    miss.operands[5] = UIntPointerType;
    miss.operands[6] = UIntAddress;
    checkRayValidation(addressArgument);

    // Registration owns each argument once. A repeated reflected index is invalid within one
    // stage, while the valid fixture deliberately uses index one in two different hit stages.
    for (auto opcode :
         {HGBCOpcode::SetRayMiss, HGBCOpcode::SetRayHitGroup, HGBCOpcode::DispatchRays})
    {
        auto program = source;
        auto& record = findRayValidationRecord(program, opcode);
        size_t firstArgument = opcode == HGBCOpcode::DispatchRays ? 3 : 4;
        auto arguments = record.operands;
        record.operands.addRange(
            arguments.data() + firstArgument,
            arguments.size() - firstArgument);
        checkRayValidation(program, "duplicate uniform parameter binding");
    }
}

/// These mutations preserve valid HGBC value/type pairs. Only the specialized shader reflection
/// can detect a missing uniform or a forged native payload/uniform ABI before ray submission.
SLANG_UNIT_TEST(hgvmRayDispatchTypeValidation)
{
    if (!(unitTestContext->enabledApis & RenderApiFlag::Vulkan))
    {
        SLANG_IGNORE_TEST;
    }
    rhi::DeviceDesc deviceDesc{};
    deviceDesc.deviceType = rhi::DeviceType::Vulkan;
    deviceDesc.slang.slangGlobalSession = unitTestContext->slangGlobalSession;
    deviceDesc.debugCallback = unitTestContext->debugCallback;
    ComPtr<rhi::IDevice> device = rhi::getRHI()->createDevice(deviceDesc);
    SLANG_CHECK_ABORT(device);
    if (!device->hasFeature(rhi::Feature::RayTracing) ||
        !device->hasFeature(rhi::Feature::Bindless))
    {
        SLANG_IGNORE_TEST;
    }
    auto source = HGVM::compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
        import slang.pipeline_rt;

        [raypayload] struct Payload
        {
            uint value : read(caller, miss) : write(caller, miss);
        }
        [shader("miss")]
        void miss(inout Payload payload, uniform uint value) { payload.value = value; }
        [shader("raygeneration")]
        void generate(uniform RaytracingPipeline tracker, uniform float value,
            uniform RWStructuredBuffer<uint>.Handle output,
            uniform RWStructuredBuffer<uint, ScalarDataLayout>.Handle scalarOutput)
        {
            output[0] = uint(value) + getBindingOffset(tracker);
            scalarOutput[0] = uint(value);
        }
        [host] void main()
        {
            let tracker = makeRaytracingPipeline(1);
            let output = RWStructuredBuffer<uint>::alloc(1);
            let scalarOutput = RWStructuredBuffer<uint, ScalarDataLayout>::alloc(1);
            __setRayMiss(tracker, 0, miss)(5);
            __dispatchRays(generate, uint3(1, 1, 1), 1)(tracker, 13.0f,
                output.getDescriptorHandle(), scalarOutput.getDescriptorHandle());
        }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_ray_dispatch_type_validation");
    // Both direct handle arguments must keep their exact source specialization, even though
    // their uint buffers have identical element sizes and strides in the host representation.
    HGVM::Desc validDesc{};
    validDesc.device = device;
    HGVM::Interpreter validInterpreter(validDesc);
    String validError;
    SlangResult validResult = validInterpreter.call(source, "main", validError);
    if (SLANG_FAILED(validResult))
        fprintf(stderr, "%s\n", validError.getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(validResult));
    const char* errors[] = {
        "ray payload type does not match shader reflection",
        "ray uniform argument type does not match shader reflection",
        "ray record is missing a required uniform shader argument",
        "ray record is missing a required uniform shader argument",
    };
    for (uint32_t corruption = 0; corruption < SLANG_COUNT_OF(errors); ++corruption)
    {
        auto wire = source->getDecodedProgram();
        auto& miss = findRayValidationRecord(wire, HGBCOpcode::SetRayMiss);
        switch (corruption)
        {
        case 0:
            miss.operands[3] = miss.operands[5];
            break;
        case 1:
            {
                const auto& scalar = findRayValidationRecord(wire, HGBCOpcode::FloatLit);
                miss.operands[5] = scalar.operands[0];
                miss.operands[6] = scalar.operands[1];
            }
            break;
        case 2:
            miss.operands.resize(4);
            break;
        case 3:
            {
                auto& dispatch = findRayValidationRecord(wire, HGBCOpcode::DispatchRays);
                dispatch.operands.resize(dispatch.operands.size() - 3);
            }
            break;
        }
        HGBC::Vector<uint8_t> bytes;
        SLANG_CHECK_ABORT(HGBC::writeProgram(wire, bytes));
        RefPtr<HGVM::Program> program;
        String error;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(HGVM::Program::load(
            bytes.data(),
            bytes.size(),
            "ray-dispatch-type-validation",
            program,
            error)));
        HGVM::Desc desc{};
        desc.device = device;
        HGVM::Interpreter interpreter(desc);
        SlangResult result = interpreter.call(program, "main", error);
        SLANG_CHECK(SLANG_FAILED(result));
        if (!error.contains(errors[corruption]))
            fprintf(stderr, "%s\n", error.getBuffer());
        SLANG_CHECK(error.contains(errors[corruption]));
    }
}

SLANG_UNIT_TEST(hgvmRayHitGroupValidation)
{
    const auto source = makeRayValidationProgram();
    struct Corruption
    {
        size_t operandIndex;
        uint32_t value;
        const char* error;
    };
    const Corruption corruptions[] = {
        {1, 99, "has an invalid enum operand"},
        {2, UIntValue, "references a missing type"},
        {3, UIntValue, "references a missing type"},
        {4, 99, "has an invalid enum operand"},
        {6, uint32_t(HGBCRayStageRole::ClosestHit), "duplicate hit-group stage role"},
        {5, UIntValue, "hit-group stage does not reference an entry point"},
        {1, uint32_t(HGBCRayGeometryKind::Procedural), "requires an intersection stage"},
        {6, uint32_t(HGBCRayStageRole::Intersection), "cannot declare an intersection stage"},
    };
    for (const auto& corruption : corruptions)
    {
        auto program = source;
        findRayValidationRecord(program, HGBCOpcode::RayHitGroup)
            .operands[corruption.operandIndex] = corruption.value;
        checkRayValidation(program, corruption.error);
    }
    auto empty = source;
    findRayValidationRecord(empty, HGBCOpcode::RayHitGroup).operands.resize(4);
    findRayValidationRecord(empty, HGBCOpcode::SetRayHitGroup).operands.resize(4);
    checkRayValidation(empty);
    findRayValidationRecord(empty, HGBCOpcode::RayHitGroup).operands[1] =
        uint32_t(HGBCRayGeometryKind::Procedural);
    checkRayValidation(empty, "requires an intersection stage");

    // Keep the shared contract broader than current runtime capabilities. Triangle groups may
    // contain only any-hit, and procedural groups may contain only intersection or all stages.
    for (bool procedural : {false, true})
    {
        auto program = source;
        auto& group = findRayValidationRecord(program, HGBCOpcode::RayHitGroup);
        group.operands[1] =
            uint32_t(procedural ? HGBCRayGeometryKind::Procedural : HGBCRayGeometryKind::Triangles);
        group.operands.resize(6);
        group.operands[4] =
            uint32_t(procedural ? HGBCRayStageRole::Intersection : HGBCRayStageRole::AnyHit);
        findRayValidationRecord(program, HGBCOpcode::SetRayHitGroup).operands.resize(4);
        checkRayValidation(program);
        if (procedural)
        {
            group.operands.add(uint32_t(HGBCRayStageRole::AnyHit));
            group.operands.add(EntryPoint);
            group.operands.add(uint32_t(HGBCRayStageRole::ClosestHit));
            group.operands.add(SpecializedEntryPoint);
            checkRayValidation(program);
        }
    }
}
