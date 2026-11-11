// slang-hgbc-validation.cpp
#include "slang-hgbc-validation.h"

#include "slang-hgvm.h"

#include <functional>
#include <limits>

namespace Slang::HGVM
{
namespace
{

class ProgramValidator
{
public:
    explicit ProgramValidator(const Program& program)
        : m_program(program), m_decodedProgram(program.getDecodedProgram())
    {
    }

    SlangResult validate(String& outError)
    {
        for (const Record& record : m_decodedProgram.records)
        {
            // Debug records must not make otherwise valid execution fail. An attached debugger
            // builds and validates its richer metadata index independently.
            if (hasHGBCOpcodeFlag(record.opcode, HGBCOpcodeFlags::Debug))
                continue;
            if (!validateRecord(record))
                return getFailure(outError);
        }
        if (!validateRuntimeAggregateLayouts())
            return getFailure(outError);

        for (const Record& record : m_decodedProgram.records)
        {
            switch (record.opcode)
            {
            case HGBCOpcode::TaggedUnionType:
                if (!validateTaggedUnionType(record))
                    return getFailure(outError);
                break;
            case HGBCOpcode::SlangModule:
                if (!validateSlangModule(record))
                    return getFailure(outError);
                break;
            case HGBCOpcode::SlangEntryPoint:
                if (!validateSlangEntryPoint(record))
                    return getFailure(outError);
                break;
            case HGBCOpcode::SpecializeSlangEntryPoint:
                if (!validateSpecializedEntryPoint(record))
                    return getFailure(outError);
                break;
            case HGBCOpcode::MakeDispatcher:
                if (!validateComputeDispatcher(record))
                    return getFailure(outError);
                break;
            case HGBCOpcode::TypeLayout:
                if (!validateTypeLayout(record))
                    return getFailure(outError);
                break;
            case HGBCOpcode::LayoutConverter:
                if (!validateLayoutConverter(record))
                    return getFailure(outError);
                break;
            case HGBCOpcode::GraphicsPipelineState:
            case HGBCOpcode::GraphicsColorTargetState:
            case HGBCOpcode::GraphicsAttachments:
            case HGBCOpcode::MakeGraphicsPipeline:
                if (!validateGraphicsPipeline(record))
                    return getFailure(outError);
                break;
            case HGBCOpcode::MakeRayTracer:
                if (!validateRayTracer(record))
                    return getFailure(outError);
                break;
            case HGBCOpcode::RayHitGroup:
                if (!validateRayHitGroup(record))
                    return getFailure(outError);
                break;
            default:
                break;
            }
        }

        for (const Function& function : m_decodedProgram.functions)
        {
            const Record& functionRecord = m_decodedProgram.records[function.recordIndex];
            if (!validateFunction(function, functionRecord))
                return getFailure(outError);
            for (const Block& block : function.blocks)
            {
                for (size_t recordIndex : block.recordIndices)
                {
                    const Record& record = m_decodedProgram.records[recordIndex];
                    if (!validateInstruction(function, record))
                        return getFailure(outError);
                }
            }
        }

        outError = "";
        return SLANG_OK;
    }

private:
    const Program& m_program;
    const HGBC::Program& m_decodedProgram;
    std::string m_error;

    SlangResult getFailure(String& outError)
    {
        outError = m_error.c_str();
        return SLANG_FAIL;
    }

    bool fail(const Record& record, const char* detail)
    {
        return fail(record, std::string(detail));
    }

    bool fail(const Record& record, std::string detail)
    {
        m_error = getHGBCOpcodeName(record.opcode);
        m_error += " at byte offset ";
        m_error += std::to_string(record.debugInfo.offset);
        m_error += " ";
        m_error += detail;
        return false;
    }

    const Record* findResultRecord(HGBCId id) const
    {
        auto recordIt = m_program.getIndex().resultRecords.find(id);
        return recordIt == m_program.getIndex().resultRecords.end()
                   ? nullptr
                   : &m_decodedProgram.records[recordIt->second];
    }

    const Record* findTypeRecord(HGBCId id) const
    {
        const Record* record = findResultRecord(id);
        return record && record->debugInfo.section == HGBCSectionTag::TYPE ? record : nullptr;
    }

    bool isTypeOpcode(HGBCId typeId, HGBCOpcode opcode) const
    {
        const Record* record = findTypeRecord(typeId);
        return record && record->opcode == opcode;
    }

    const HGBC::StructTypeDescriptor* findAggregateType(HGBCId typeId) const
    {
        if (const HGBC::StructTypeDescriptor* type = m_program.findStructType(typeId))
            return type;
        return m_program.findClassType(typeId);
    }

    bool isBufferType(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        return record && (record->opcode == HGBCOpcode::StructuredBufferType ||
                          record->opcode == HGBCOpcode::RWStructuredBufferType);
    }

    bool isTextureType(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        return record && (record->opcode == HGBCOpcode::Texture2DType ||
                          record->opcode == HGBCOpcode::RenderTarget2DType);
    }

    /// Descriptor handles retain typed graph resources until their native descriptor is bound.
    /// Bottom-level acceleration structures are build inputs, not traceable shader resources.
    bool isDescriptorResourceType(HGBCId typeId) const
    {
        return isBufferType(typeId) || isTypeOpcode(typeId, HGBCOpcode::Texture2DType) ||
               isTypeOpcode(typeId, HGBCOpcode::RaytracingAccelerationStructureType);
    }

    bool isNullableManagedType(HGBCId typeId) const
    {
        const Record* type = findTypeRecord(typeId);
        if (!type)
            return false;

        switch (type->opcode)
        {
        case HGBCOpcode::StructuredBufferType:
        case HGBCOpcode::RWStructuredBufferType:
        case HGBCOpcode::Texture2DType:
        case HGBCOpcode::RenderTarget2DType:
        case HGBCOpcode::RaytracingAccelerationStructureType:
        case HGBCOpcode::RaytracingBottomLevelAccelerationStructureType:
        case HGBCOpcode::RaytracingPipelineType:
        case HGBCOpcode::WindowType:
        case HGBCOpcode::SurfaceType:
        case HGBCOpcode::TimerType:
        case HGBCOpcode::ByteStreamType:
        case HGBCOpcode::UploadBufferStorageType:
        case HGBCOpcode::ClassType:
        case HGBCOpcode::RuntimeArrayType:
        case HGBCOpcode::DeferType:
        case HGBCOpcode::DescriptorHandleType:
        case HGBCOpcode::BoxType:
        case HGBCOpcode::TaggedUnionType:
            return true;
        default:
            return false;
        }
    }

    /// Checks that a copied payload owns everything needed to survive its source frame.
    /// A struct containing a frame pointer cannot be boxed, even when another field is managed.
    /// Managed handles are leaves: their own allocation contract owns any referenced storage.
    /// Recursive-by-value aggregates have already been rejected before this predicate is used.
    bool isOwnedBoxPayloadType(HGBCId typeId) const
    {
        if (isNullableManagedType(typeId))
            return true;
        const auto& index = m_program.getIndex();
        if (index.uintTypeIds.contains(typeId) || index.intTypeIds.contains(typeId) ||
            index.floatTypeIds.contains(typeId) || index.boolTypeIds.contains(typeId) ||
            index.stringTypeIds.contains(typeId))
        {
            return true;
        }
        const Record* type = findTypeRecord(typeId);
        if (!type)
            return false;
        switch (type->opcode)
        {
        case HGBCOpcode::VectorType:
        case HGBCOpcode::ArrayType:
            return isOwnedBoxPayloadType(type->operands[2]);
        case HGBCOpcode::MatrixType:
            return isOwnedBoxPayloadType(type->operands[4]);
        case HGBCOpcode::StructType:
            for (const auto& field : m_program.findStructType(typeId)->fields)
                if (!isOwnedBoxPayloadType(field.typeId))
                    return false;
            return true;
        case HGBCOpcode::GraphicsDrawType:
        case HGBCOpcode::GraphicsDrawParametersType:
            return true;
        default:
            return false;
        }
    }

    /// Requires every inhabited union case to have an owning payload representation.
    bool validateTaggedUnionType(const Record& record)
    {
        const auto* type = m_program.findTaggedUnionType(record.operands[0]);
        SLANG_RELEASE_ASSERT(type);
        for (const auto& caseDescriptor : type->cases)
        {
            if (!isTypeOpcode(caseDescriptor.payloadTypeId, HGBCOpcode::VoidType) &&
                !isOwnedBoxPayloadType(caseDescriptor.payloadTypeId))
            {
                return fail(record, "case payload must be an owning value type or void");
            }
        }
        return true;
    }

    const Record* findTypeLayoutRecord(HGBCId id) const
    {
        const Record* record = findResultRecord(id);
        return record && record->opcode == HGBCOpcode::TypeLayout ? record : nullptr;
    }

    const Record* findLayoutConverterRecord(HGBCId id) const
    {
        const Record* record = findResultRecord(id);
        return record && record->opcode == HGBCOpcode::LayoutConverter ? record : nullptr;
    }

    bool validateTypeLayout(const Record& record)
    {
        SLANG_RELEASE_ASSERT(record.operands.size() == 6 || record.operands.size() == 7);
        HGBCLayoutDomain domain = HGBCLayoutDomain(record.operands[2]);
        HGBCLayoutUsage usage = HGBCLayoutUsage(record.operands[3]);
        HGBCId contextId = record.operands[4];
        uint32_t contextIndex = record.operands[5];
        uint32_t entryPointIndex = record.operands.size() == 7 ? record.operands[6] : 0;
        if (domain != HGBCLayoutDomain::ShaderObject && entryPointIndex)
            return fail(record, "non-shader layouts cannot select an entry point");
        switch (domain)
        {
        case HGBCLayoutDomain::Host:
            if (usage != HGBCLayoutUsage::Value || contextId || contextIndex)
                return fail(record, "host layouts must be context-free value layouts");
            return true;
        case HGBCLayoutDomain::DeviceMemory:
            if ((usage != HGBCLayoutUsage::Value &&
                 usage != HGBCLayoutUsage::StructuredBufferElement) ||
                contextId || contextIndex)
            {
                return fail(record, "device-memory layouts have invalid usage or context");
            }
            return true;
        case HGBCLayoutDomain::ShaderObject:
            {
                const Record* context = findResultRecord(contextId);
                if (usage != HGBCLayoutUsage::ShaderParameter || !context ||
                    (context->opcode != HGBCOpcode::MakeDispatcher &&
                     context->opcode != HGBCOpcode::MakeGraphicsPipeline))
                {
                    return fail(record, "shader layouts must reference a dispatcher parameter");
                }
                if (entryPointIndex >
                    (context->opcode == HGBCOpcode::MakeGraphicsPipeline ? 1u : 0u))
                    return fail(record, "shader layout selects an invalid entry point");
                return true;
            }
        case HGBCLayoutDomain::Serialized:
            if (usage != HGBCLayoutUsage::Value || contextId || contextIndex)
                return fail(record, "serialized layouts must be context-free value layouts");
            return true;
        }
        SLANG_UNREACHABLE("invalid HGBC layout domain");
    }

    bool validateLayoutConverter(const Record& record)
    {
        SLANG_RELEASE_ASSERT(record.operands.size() == 3);
        const Record* source = findTypeLayoutRecord(record.operands[1]);
        const Record* destination = findTypeLayoutRecord(record.operands[2]);
        if (!source || !destination)
            return fail(record, "must reference source and destination type layouts");
        if (!doesValueTypeMatch(source->operands[1], destination->operands[1]))
            return fail(record, "source and destination layouts describe different types");
        return true;
    }

    bool isFloat2Type(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        return record && record->opcode == HGBCOpcode::VectorType && record->operands.size() == 3 &&
               record->operands[1] == 2 &&
               isTypeOpcode(record->operands[2], HGBCOpcode::Float32Type);
    }

    bool isFloat4Type(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        return record && record->opcode == HGBCOpcode::VectorType && record->operands.size() == 3 &&
               record->operands[1] == 4 &&
               isTypeOpcode(record->operands[2], HGBCOpcode::Float32Type);
    }

    bool isFloat3Type(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        return record && record->opcode == HGBCOpcode::VectorType && record->operands.size() == 3 &&
               record->operands[1] == 3 &&
               isTypeOpcode(record->operands[2], HGBCOpcode::Float32Type);
    }

    bool isFloat4TextureType(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        return record &&
               ((record->opcode == HGBCOpcode::Texture2DType && record->operands.size() == 4) ||
                (record->opcode == HGBCOpcode::RenderTarget2DType &&
                 record->operands.size() == 3)) &&
               isFloat4Type(record->operands[1]);
    }

    /// Returns the declared storage format of a validated HGBC texture type.
    ///
    /// Consider `RenderTarget2D<float4, RGBA8UnormSrgb>`: this returns the format operand directly;
    /// callers never infer it from `float4`, which would incorrectly mean RGBA32Float.
    HGBCResourceFormat getTextureFormat(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        SLANG_RELEASE_ASSERT(record);
        if (record->opcode == HGBCOpcode::Texture2DType)
        {
            SLANG_RELEASE_ASSERT(record->operands.size() == 4);
            return HGBCResourceFormat(record->operands[3]);
        }
        SLANG_RELEASE_ASSERT(
            record->opcode == HGBCOpcode::RenderTarget2DType && record->operands.size() == 3);
        return HGBCResourceFormat(record->operands[2]);
    }

    HGBCTextureAccess getTextureAccess(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        SLANG_RELEASE_ASSERT(
            record && record->opcode == HGBCOpcode::Texture2DType && record->operands.size() == 4);
        return HGBCTextureAccess(record->operands[2]);
    }

    /// Reports whether one format is valid for a platform presentation surface.
    static bool isSurfaceFormat(HGBCResourceFormat format)
    {
        switch (format)
        {
        case HGBCResourceFormat::RGBA8Unorm:
        case HGBCResourceFormat::RGBA8UnormSrgb:
        case HGBCResourceFormat::RGBA16Float:
        case HGBCResourceFormat::RGB10A2Unorm:
            return true;
        default:
            return false;
        }
    }

    bool isUInt3Type(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        return record && record->opcode == HGBCOpcode::VectorType && record->operands.size() == 3 &&
               record->operands[1] == 3 &&
               m_program.getIndex().uintTypeIds.contains(record->operands[2]);
    }

    bool isUInt2Type(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        return record && record->opcode == HGBCOpcode::VectorType && record->operands.size() == 3 &&
               record->operands[1] == 2 &&
               m_program.getIndex().uintTypeIds.contains(record->operands[2]);
    }

    bool isPrintableScalarType(HGBCId typeId) const
    {
        const HGBC::BasicIndex& index = m_program.getIndex();
        return index.uintTypeIds.contains(typeId) || index.intTypeIds.contains(typeId) ||
               index.floatTypeIds.contains(typeId) || index.boolTypeIds.contains(typeId) ||
               index.stringTypeIds.contains(typeId);
    }

    HGBCId getBufferElementTypeId(HGBCId typeId) const
    {
        const Record* record = findTypeRecord(typeId);
        if (!record ||
            (record->opcode != HGBCOpcode::StructuredBufferType &&
             record->opcode != HGBCOpcode::RWStructuredBufferType) ||
            record->operands.size() != 2)
        {
            return 0;
        }
        return record->operands[1];
    }

    HGBCId getValueTypeId(HGBCId valueId)
    {
        const Record* record = findResultRecord(valueId);
        if (!record)
            return 0;

        int typeOperandIndex = getHGBCTypeOperandIndex(record->opcode);
        if (typeOperandIndex >= 0 && size_t(typeOperandIndex) < record->operands.size())
            return record->operands[size_t(typeOperandIndex)];

        return 0;
    }

    // Returns the value type named by an address-wrapper type, or zero for an ordinary type.
    //
    // Consider both `%p : Ptr<uint> = OpVar` and `%x : uint = OpAdd`: the first type record has
    // the `AddressType` property and names `uint`, while the second is an unwrapped value. This
    // keeps address classification independent of the operation that happened to produce `%p`.
    HGBCId getAddressValueTypeId(HGBCId typeId) const
    {
        const Record* typeRecord = findTypeRecord(typeId);
        if (!typeRecord || !hasHGBCOpcodeFlag(typeRecord->opcode, HGBCOpcodeFlags::AddressType) ||
            typeRecord->operands.size() != 2)
        {
            return 0;
        }
        return typeRecord->operands[1];
    }

    HGBCId getAddressedTypeId(HGBCId valueId)
    {
        return getAddressValueTypeId(getValueTypeId(valueId));
    }

    HGBCId getEffectiveValueTypeId(HGBCId valueId)
    {
        HGBCId addressedTypeId = getAddressedTypeId(valueId);
        return addressedTypeId ? addressedTypeId : getValueTypeId(valueId);
    }

    // Returns whether raw external bytes can safely replace a value without forging managed
    // references or requiring a copy constructor. Consider `struct P { float3 p; uint id; }`:
    // every leaf is numeric, so the complete staged representation is byte-serializable. A struct
    // containing String, ByteStream, or a resource is rejected at this HGBC boundary.
    bool isByteSerializableType(HGBCId typeId) const
    {
        const HGBC::BasicIndex& index = m_program.getIndex();
        if (index.uintTypeIds.contains(typeId) || index.intTypeIds.contains(typeId) ||
            index.floatTypeIds.contains(typeId) || index.boolTypeIds.contains(typeId))
        {
            return true;
        }
        const Record* type = findTypeRecord(typeId);
        if (!type)
            return false;
        if (type->opcode == HGBCOpcode::VectorType)
            return type->operands.size() == 3 && isByteSerializableType(type->operands[2]);
        if (type->opcode == HGBCOpcode::MatrixType)
            return type->operands.size() == 5 && isByteSerializableType(type->operands[4]);
        if (type->opcode == HGBCOpcode::ArrayType)
            return type->operands.size() == 3 && isByteSerializableType(type->operands[2]);
        if (type->opcode == HGBCOpcode::StructType)
        {
            const HGBC::StructTypeDescriptor* descriptor = m_program.findStructType(typeId);
            if (!descriptor)
                return false;
            for (const HGBC::StructFieldDescriptor& field : descriptor->fields)
            {
                if (!isByteSerializableType(field.typeId))
                    return false;
            }
            return true;
        }
        return false;
    }

    bool validateByteStreamTransfer(const Record& record)
    {
        if (!m_program.getIndex().uintTypeIds.contains(record.operands[0]) ||
            !isTypeOpcode(
                getEffectiveValueTypeId(record.operands[3]),
                HGBCOpcode::ByteStreamType) ||
            !m_program.getIndex().uintTypeIds.contains(
                getEffectiveValueTypeId(record.operands[5])) ||
            !m_program.getIndex().uintTypeIds.contains(getEffectiveValueTypeId(record.operands[6])))
        {
            return fail(record, "requires a uint result, byte stream, offset, and byte count");
        }

        HGBCByteStreamEndpointKind endpointKind = HGBCByteStreamEndpointKind(record.operands[2]);
        if (endpointKind == HGBCByteStreamEndpointKind::HostAddress)
        {
            HGBCId valueTypeId = getAddressedTypeId(record.operands[4]);
            if (!valueTypeId || !isByteSerializableType(valueTypeId))
                return fail(record, "host endpoint must address a byte-serializable value");
        }
        else if (endpointKind == HGBCByteStreamEndpointKind::RuntimeArray)
        {
            HGBCId endpointTypeId = getEffectiveValueTypeId(record.operands[4]);
            const Record* endpointType = findTypeRecord(endpointTypeId);
            if (!endpointType || endpointType->opcode != HGBCOpcode::RuntimeArrayType ||
                endpointType->operands.size() != 2 ||
                !isByteSerializableType(endpointType->operands[1]))
            {
                return fail(
                    record,
                    "runtime-array endpoint must contain byte-serializable elements");
            }
        }
        else if (endpointKind == HGBCByteStreamEndpointKind::DeviceBuffer)
        {
            if (!isBufferType(getEffectiveValueTypeId(record.operands[4])))
                return fail(record, "device-buffer endpoint must reference a structured buffer");
        }
        else if (endpointKind == HGBCByteStreamEndpointKind::DeviceTexture2D)
        {
            if (record.opcode != HGBCOpcode::ByteStreamRead ||
                !isTextureType(getEffectiveValueTypeId(record.operands[4])))
            {
                return fail(record, "device-texture endpoint must be a readable 2D texture");
            }
        }
        else
            return fail(record, "byte-stream endpoint kind is invalid");
        return true;
    }

    bool validateImageOperation(const Record& record)
    {
        if (!m_program.getIndex().uintTypeIds.contains(record.operands[0]))
            return fail(record, "must return an encoded uint byte count");

        bool isDecode = record.opcode == HGBCOpcode::ImageDecode;
        HGBCEncodedImageEndpointKind encodedKind =
            HGBCEncodedImageEndpointKind(record.operands[isDecode ? 3 : 4]);
        HGBCPixelImageEndpointKind pixelKind =
            HGBCPixelImageEndpointKind(record.operands[isDecode ? 4 : 3]);
        HGBCId encodedValue = record.operands[isDecode ? 5 : 6];
        HGBCId pixelValue = record.operands[isDecode ? 6 : 5];

        if (encodedKind == HGBCEncodedImageEndpointKind::ByteStream)
        {
            if (!isTypeOpcode(getEffectiveValueTypeId(encodedValue), HGBCOpcode::ByteStreamType))
            {
                return fail(record, "encoded endpoint must be a byte stream");
            }
        }
        else
        {
            return fail(record, "encoded image endpoint kind is not specified yet");
        }

        if (pixelKind == HGBCPixelImageEndpointKind::DeviceTexture2D)
        {
            if (!isFloat4TextureType(getEffectiveValueTypeId(pixelValue)))
                return fail(record, "pixel endpoint must be an RWTexture2D<float4>");
        }
        else
        {
            return fail(record, "pixel image endpoint kind is not specified yet");
        }
        return true;
    }

    bool areScalarTypesCompatible(HGBCId expectedTypeId, HGBCId actualTypeId) const
    {
        const HGBC::BasicIndex& index = m_program.getIndex();
        // Signed and unsigned 32-bit integers share the same runtime bit payload. The declared
        // operation type determines how those bits are interpreted, as it does for
        // ScalarTypeCast.
        bool expectedIsInteger =
            index.uintTypeIds.contains(expectedTypeId) || index.intTypeIds.contains(expectedTypeId);
        bool actualIsInteger =
            index.uintTypeIds.contains(actualTypeId) || index.intTypeIds.contains(actualTypeId);
        const Record* expectedType = findTypeRecord(expectedTypeId);
        const Record* actualType = findTypeRecord(actualTypeId);
        bool matchingFloatTypes =
            expectedType && actualType && index.floatTypeIds.contains(expectedTypeId) &&
            index.floatTypeIds.contains(actualTypeId) && expectedType->opcode == actualType->opcode;
        return (expectedIsInteger && actualIsInteger) || matchingFloatTypes ||
               (index.boolTypeIds.contains(expectedTypeId) &&
                index.boolTypeIds.contains(actualTypeId)) ||
               (index.stringTypeIds.contains(expectedTypeId) &&
                index.stringTypeIds.contains(actualTypeId));
    }

    // Describes the logical shape and leaf type of scalar, vector, and matrix values.
    //
    // Consider `float3x2`: it has kind `Matrix`, three rows, two columns, and `%float` as its
    // scalar type. Arithmetic and comparison validation can then reason about the shape once
    // without maintaining separate scalar, vector, and matrix branches.
    struct NumericType
    {
        enum class Kind : uint8_t
        {
            Scalar,
            Vector,
            Matrix,
        };

        Kind kind = Kind::Scalar;
        HGBCId scalarTypeId = 0;
        uint32_t rowCount = 1;
        uint32_t columnCount = 1;
    };

    // Finds the scalar leaf and logical dimensions of one basic numeric or Boolean type.
    //
    // For example, `bool4` returns four columns with the Boolean scalar ID. Structs and vectors
    // whose element type is another vector return false because they are not HGBC numeric values.
    bool tryGetNumericType(HGBCId typeId, NumericType& outType) const
    {
        const HGBC::BasicIndex& index = m_program.getIndex();
        if (index.uintTypeIds.contains(typeId) || index.intTypeIds.contains(typeId) ||
            index.floatTypeIds.contains(typeId) || index.boolTypeIds.contains(typeId))
        {
            outType = {};
            outType.scalarTypeId = typeId;
            return true;
        }

        const Record* type = findTypeRecord(typeId);
        if (!type)
            return false;
        if (type->opcode == HGBCOpcode::VectorType && type->operands.size() == 3)
        {
            NumericType elementType;
            if (!tryGetNumericType(type->operands[2], elementType) ||
                elementType.kind != NumericType::Kind::Scalar)
            {
                return false;
            }
            outType.kind = NumericType::Kind::Vector;
            outType.scalarTypeId = elementType.scalarTypeId;
            outType.columnCount = type->operands[1];
            return true;
        }
        if (type->opcode == HGBCOpcode::MatrixType && type->operands.size() == 5)
        {
            NumericType elementType;
            if (!tryGetNumericType(type->operands[4], elementType) ||
                elementType.kind != NumericType::Kind::Scalar)
            {
                return false;
            }
            outType.kind = NumericType::Kind::Matrix;
            outType.scalarTypeId = elementType.scalarTypeId;
            outType.rowCount = type->operands[1];
            outType.columnCount = type->operands[2];
            return true;
        }
        return false;
    }

    /// Reports whether a physical texture format can expose the requested logical shader value.
    ///
    /// This is checked for explicitly formatted ordinary textures and presentation render targets,
    /// and again at allocation after the physical storage format is known.
    bool isTextureFormatCompatible(HGBCId elementTypeId, HGBCResourceFormat format) const
    {
        NumericType elementType;
        if (!tryGetNumericType(elementTypeId, elementType) ||
            elementType.kind == NumericType::Kind::Matrix)
        {
            return false;
        }

        const HGBC::BasicIndex& index = m_program.getIndex();
        uint32_t laneCount =
            elementType.kind == NumericType::Kind::Scalar ? 1 : elementType.columnCount;
        switch (format)
        {
        case HGBCResourceFormat::R32Uint:
            return laneCount == 1 && index.uintTypeIds.contains(elementType.scalarTypeId);
        case HGBCResourceFormat::R32Sint:
            return laneCount == 1 && index.intTypeIds.contains(elementType.scalarTypeId);
        case HGBCResourceFormat::D32Float:
        case HGBCResourceFormat::D16Unorm:
        case HGBCResourceFormat::D32FloatS8Uint:
        case HGBCResourceFormat::R32Float:
            return laneCount == 1 &&
                   isTypeOpcode(elementType.scalarTypeId, HGBCOpcode::Float32Type);
        case HGBCResourceFormat::RG32Uint:
            return laneCount == 2 && index.uintTypeIds.contains(elementType.scalarTypeId);
        case HGBCResourceFormat::RG32Sint:
            return laneCount == 2 && index.intTypeIds.contains(elementType.scalarTypeId);
        case HGBCResourceFormat::RG32Float:
            return laneCount == 2 &&
                   isTypeOpcode(elementType.scalarTypeId, HGBCOpcode::Float32Type);
        case HGBCResourceFormat::RGBA32Uint:
            return (laneCount == 3 || laneCount == 4) &&
                   index.uintTypeIds.contains(elementType.scalarTypeId);
        case HGBCResourceFormat::RGBA32Sint:
            return (laneCount == 3 || laneCount == 4) &&
                   index.intTypeIds.contains(elementType.scalarTypeId);
        case HGBCResourceFormat::RGBA32Float:
            return (laneCount == 3 || laneCount == 4) &&
                   isTypeOpcode(elementType.scalarTypeId, HGBCOpcode::Float32Type);
        case HGBCResourceFormat::RGBA8Unorm:
        case HGBCResourceFormat::RGBA8UnormSrgb:
        case HGBCResourceFormat::RGBA16Float:
        case HGBCResourceFormat::RGB10A2Unorm:
            return laneCount == 4 &&
                   isTypeOpcode(elementType.scalarTypeId, HGBCOpcode::Float32Type);
        case HGBCResourceFormat::Undefined:
            return false;
        }
        return false;
    }

    // Reports whether two numeric values have identical scalar/vector/matrix dimensions.
    //
    // This deliberately ignores the leaf scalar ID so a `float3` comparison can be checked
    // against its `bool3` result while still requiring both values to contain three lanes.
    static bool haveSameNumericShape(const NumericType& left, const NumericType& right)
    {
        return left.kind == right.kind && left.rowCount == right.rowCount &&
               left.columnCount == right.columnCount;
    }

    bool isIntegerScalarType(HGBCId typeId) const
    {
        const HGBC::BasicIndex& index = m_program.getIndex();
        return index.uintTypeIds.contains(typeId) || index.intTypeIds.contains(typeId);
    }

    bool isFloatScalarType(HGBCId typeId) const
    {
        return m_program.getIndex().floatTypeIds.contains(typeId);
    }

    bool isBoolScalarType(HGBCId typeId) const
    {
        return m_program.getIndex().boolTypeIds.contains(typeId);
    }

    // Reports whether a type can be one endpoint of OpScalarTypeCast.
    //
    // For example, bool and float are accepted, while float3 is rejected because this operation
    // represents one scalar conversion rather than an implicit aggregate traversal.
    bool isBasicScalarType(HGBCId typeId) const
    {
        return isIntegerScalarType(typeId) || isFloatScalarType(typeId) || isBoolScalarType(typeId);
    }

    // Validates the complete resource descriptor carried by one allocation instruction.
    //
    // Consider a `RWTexture2D<float4>` allocation: the record itself says `Texture`, `Texture2D`,
    // `RGBA32Float`, and its usage flags. This function checks those declarations against each
    // other and the result type once, so neither interpreter needs to inspect the result type to
    // decide which resource or RHI descriptor to create.
    bool validateAllocateResource(const Record& record)
    {
        SLANG_RELEASE_ASSERT(record.operands.size() >= 15);
        HGBCResourceKind kind = HGBCResourceKind(record.operands[2]);
        HGBCResourceDimension dimension = HGBCResourceDimension(record.operands[3]);
        HGBCResourceFormat format = HGBCResourceFormat(record.operands[4]);
        HGBCResourceMemoryType memoryType = HGBCResourceMemoryType(record.operands[5]);
        HGBCResourceState initialState = HGBCResourceState(record.operands[6]);
        HGBCId elementTypeId = record.operands[7];

        if (!findTypeRecord(elementTypeId))
            return fail(record, "element type is missing");
        for (size_t extentIndex = 8; extentIndex <= 10; ++extentIndex)
        {
            if (!m_program.getIndex().uintTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[extentIndex])))
            {
                return fail(record, "resource extents must be uint values");
            }
        }
        if (!record.operands[11] ||
            (kind == HGBCResourceKind::Buffer && record.operands[11] != 1) ||
            record.operands[12] != 1 || record.operands[13] != 0)
        {
            return fail(
                record,
                "requires a positive mip count, one sample, and sample quality zero; buffers use "
                "one mip");
        }
        const Record* layout = findTypeLayoutRecord(record.operands[14]);
        if (!layout || !doesValueTypeMatch(layout->operands[1], elementTypeId) ||
            HGBCLayoutDomain(layout->operands[2]) != HGBCLayoutDomain::DeviceMemory)
        {
            return fail(record, "resource layout does not describe its device element type");
        }
        HGBCLayoutUsage expectedLayoutUsage = kind == HGBCResourceKind::Buffer
                                                  ? HGBCLayoutUsage::StructuredBufferElement
                                                  : HGBCLayoutUsage::Value;
        if (HGBCLayoutUsage(layout->operands[3]) != expectedLayoutUsage)
            return fail(record, "resource layout has an incompatible usage");
        if (record.operands.size() == 15)
            return fail(record, "resource allocation must declare at least one usage");

        bool usageSeen[9] = {};
        bool hasShaderResource = false;
        bool hasUnorderedAccess = false;
        bool hasCopySource = false;
        bool hasCopyDestination = false;
        bool hasRenderTarget = false;
        bool hasDepthStencil = false;
        for (size_t operandIndex = 15; operandIndex < record.operands.size(); ++operandIndex)
        {
            HGBCResourceUsage usage = HGBCResourceUsage(record.operands[operandIndex]);
            uint32_t usageIndex = uint32_t(usage);
            SLANG_RELEASE_ASSERT(usageIndex < SLANG_COUNT_OF(usageSeen));
            if (usageSeen[usageIndex])
                return fail(record, "resource allocation contains a duplicate usage");
            usageSeen[usageIndex] = true;
            hasShaderResource |= usage == HGBCResourceUsage::ShaderResource;
            hasUnorderedAccess |= usage == HGBCResourceUsage::UnorderedAccess;
            hasCopySource |= usage == HGBCResourceUsage::CopySource;
            hasCopyDestination |= usage == HGBCResourceUsage::CopyDestination;
            hasRenderTarget |= usage == HGBCResourceUsage::RenderTarget;
            hasDepthStencil |= usage == HGBCResourceUsage::DepthStencil;

            bool bufferOnly = usage == HGBCResourceUsage::VertexBuffer ||
                              usage == HGBCResourceUsage::IndexBuffer ||
                              usage == HGBCResourceUsage::AccelerationStructureBuildInput;
            bool textureOnly = usage == HGBCResourceUsage::RenderTarget ||
                               usage == HGBCResourceUsage::DepthStencil;
            if ((kind == HGBCResourceKind::Buffer && textureOnly) ||
                (kind == HGBCResourceKind::Texture && bufferOnly))
            {
                return fail(record, "resource kind and usage are incompatible");
            }
        }

        bool initialStateIsUsed =
            initialState == HGBCResourceState::Undefined ||
            (initialState == HGBCResourceState::ShaderResource && hasShaderResource) ||
            (initialState == HGBCResourceState::UnorderedAccess && hasUnorderedAccess) ||
            (initialState == HGBCResourceState::CopySource && hasCopySource) ||
            (initialState == HGBCResourceState::CopyDestination && hasCopyDestination) ||
            (initialState == HGBCResourceState::RenderTarget && hasRenderTarget) ||
            (initialState == HGBCResourceState::DepthWrite && hasDepthStencil);
        if (!initialStateIsUsed)
            return fail(record, "initial resource state is not included in the usage set");

        if (kind == HGBCResourceKind::Buffer)
        {
            if (dimension != HGBCResourceDimension::Buffer ||
                format != HGBCResourceFormat::Undefined)
            {
                return fail(record, "buffers require Buffer dimension and Undefined format");
            }
            if (memoryType == HGBCResourceMemoryType::DeviceLocal)
            {
                HGBCId resultElementTypeId = getBufferElementTypeId(record.operands[0]);
                if (!resultElementTypeId || !doesValueTypeMatch(resultElementTypeId, elementTypeId))
                {
                    return fail(record, "buffer result and explicit element types do not match");
                }
                return true;
            }
            if (memoryType == HGBCResourceMemoryType::Upload)
            {
                if (!isTypeOpcode(record.operands[0], HGBCOpcode::UploadBufferStorageType) ||
                    record.operands.size() != 16 || !hasCopySource)
                {
                    return fail(record, "upload buffers must be CopySource upload storage");
                }
                return true;
            }
            return fail(record, "read-back buffer allocation has no HGBC value type yet");
        }

        if (dimension != HGBCResourceDimension::Texture2D ||
            memoryType != HGBCResourceMemoryType::DeviceLocal ||
            format == HGBCResourceFormat::Undefined)
        {
            return fail(record, "textures currently require device-local Texture2D storage");
        }

        const Record* textureType = findTypeRecord(record.operands[0]);
        if (!textureType || textureType->opcode != HGBCOpcode::Texture2DType ||
            textureType->operands.size() != 4 ||
            !doesValueTypeMatch(textureType->operands[1], elementTypeId) ||
            !isTextureFormatCompatible(elementTypeId, format))
        {
            return fail(record, "texture result type does not match its element type and format");
        }
        HGBCResourceFormat declaredFormat = getTextureFormat(record.operands[0]);
        if (declaredFormat != HGBCResourceFormat::Undefined && declaredFormat != format)
            return fail(record, "texture allocation does not match its declared format");
        HGBCTextureAccess access = getTextureAccess(record.operands[0]);
        if (isHGBCDepthFormat(format))
        {
            if (!hasDepthStencil || hasUnorderedAccess || hasRenderTarget ||
                access != HGBCTextureAccess::ReadOnly || declaredFormat != format)
                return fail(
                    record,
                    "depth storage requires a matching read-only depth texture type and "
                    "DepthStencil usage");
        }
        else if (hasDepthStencil)
            return fail(record, "DepthStencil usage requires a depth/stencil format");
        if ((access == HGBCTextureAccess::ReadOnly && !hasShaderResource) ||
            (access != HGBCTextureAccess::ReadOnly && !hasUnorderedAccess))
            return fail(record, "texture allocation usage does not match its declared access");
        return true;
    }

    // Consider `PerspectiveCamera(makePerspectiveProjection(...), position)`. Generic lowering can
    // emit the projection result and constructor parameter as separate float4x4 records, with one
    // row-major and the other column-major. `OpMakeMatrix` normalizes both to the same row/column
    // value representation before the call. Compare that value-bearing shape, rather than storage
    // layout metadata, while keeping nominal structs and opaque type categories distinct.
    bool doesValueTypeMatch(HGBCId expectedTypeId, HGBCId actualTypeId) const
    {
        if (expectedTypeId == actualTypeId)
            return true;

        const Record* expectedType = findTypeRecord(expectedTypeId);
        const Record* actualType = findTypeRecord(actualTypeId);
        if (!expectedType || !actualType)
            return false;

        if (areScalarTypesCompatible(expectedTypeId, actualTypeId))
            return true;

        if (hasHGBCOpcodeFlag(expectedType->opcode, HGBCOpcodeFlags::AddressType))
        {
            return actualType->opcode == expectedType->opcode &&
                   expectedType->operands.size() == 2 && actualType->operands.size() == 2 &&
                   doesValueTypeMatch(expectedType->operands[1], actualType->operands[1]);
        }

        switch (expectedType->opcode)
        {
        case HGBCOpcode::VoidType:
            return actualType->opcode == HGBCOpcode::VoidType;
        case HGBCOpcode::StructuredBufferType:
        case HGBCOpcode::RWStructuredBufferType:
            return actualType->opcode == expectedType->opcode &&
                   (expectedType->operands[1] == actualType->operands[1] ||
                    areScalarTypesCompatible(expectedType->operands[1], actualType->operands[1]));
        case HGBCOpcode::Texture2DType:
            return actualType->opcode == HGBCOpcode::Texture2DType &&
                   expectedType->operands.size() == 4 && actualType->operands.size() == 4 &&
                   expectedType->operands[2] == actualType->operands[2] &&
                   expectedType->operands[3] == actualType->operands[3] &&
                   (expectedType->operands[1] == actualType->operands[1] ||
                    areScalarTypesCompatible(expectedType->operands[1], actualType->operands[1]));
        case HGBCOpcode::RenderTarget2DType:
            return actualType->opcode == HGBCOpcode::RenderTarget2DType &&
                   expectedType->operands.size() == 3 && actualType->operands.size() == 3 &&
                   expectedType->operands[2] == actualType->operands[2] &&
                   (expectedType->operands[1] == actualType->operands[1] ||
                    areScalarTypesCompatible(expectedType->operands[1], actualType->operands[1]));
        case HGBCOpcode::VectorType:
            return actualType->opcode == HGBCOpcode::VectorType &&
                   expectedType->operands.size() == 3 && actualType->operands.size() == 3 &&
                   expectedType->operands[1] == actualType->operands[1] &&
                   areScalarTypesCompatible(expectedType->operands[2], actualType->operands[2]);
        case HGBCOpcode::MatrixType:
            return actualType->opcode == HGBCOpcode::MatrixType &&
                   expectedType->operands.size() == 5 && actualType->operands.size() == 5 &&
                   expectedType->operands[1] == actualType->operands[1] &&
                   expectedType->operands[2] == actualType->operands[2] &&
                   areScalarTypesCompatible(expectedType->operands[4], actualType->operands[4]);
        case HGBCOpcode::ArrayType:
            return actualType->opcode == HGBCOpcode::ArrayType &&
                   expectedType->operands.size() == 3 && actualType->operands.size() == 3 &&
                   expectedType->operands[1] == actualType->operands[1] &&
                   doesValueTypeMatch(expectedType->operands[2], actualType->operands[2]);
        case HGBCOpcode::RuntimeArrayType:
            return actualType->opcode == HGBCOpcode::RuntimeArrayType &&
                   expectedType->operands.size() == 2 && actualType->operands.size() == 2 &&
                   doesValueTypeMatch(expectedType->operands[1], actualType->operands[1]);
        case HGBCOpcode::DescriptorHandleType:
            return actualType->opcode == HGBCOpcode::DescriptorHandleType &&
                   expectedType->operands.size() == 2 && actualType->operands.size() == 2 &&
                   doesValueTypeMatch(expectedType->operands[1], actualType->operands[1]);
        case HGBCOpcode::DeferType:
            return actualType->opcode == HGBCOpcode::DeferType &&
                   expectedType->operands[1] == actualType->operands[1];
        case HGBCOpcode::TimerType:
        case HGBCOpcode::ByteStreamType:
        case HGBCOpcode::UploadBufferStorageType:
        case HGBCOpcode::WindowType:
        case HGBCOpcode::RaytracingAccelerationStructureType:
        case HGBCOpcode::RaytracingBottomLevelAccelerationStructureType:
        case HGBCOpcode::RaytracingPipelineType:
        case HGBCOpcode::BoxType:
            return actualType->opcode == expectedType->opcode;
        case HGBCOpcode::SurfaceType:
            return actualType->opcode == HGBCOpcode::SurfaceType &&
                   expectedType->operands.size() == 2 && actualType->operands.size() == 2 &&
                   expectedType->operands[1] == actualType->operands[1];
        default:
            return false;
        }
    }

    bool isValueCompatible(HGBCId expectedTypeId, HGBCId valueId)
    {
        const Record* expectedType = findTypeRecord(expectedTypeId);
        if (!expectedType)
            return false;

        HGBCId expectedAddressedTypeId = getAddressValueTypeId(expectedTypeId);
        if (expectedAddressedTypeId)
        {
            return getAddressedTypeId(valueId) == expectedAddressedTypeId;
        }
        // Value operands copy their declared bytes; only an address-typed destination or
        // parameter accepts an address token. Consider OpStore Ptr<Box> %destination %source:
        // %source must be a Box, not another Ptr<Box>. The latter requires an explicit OpLoad.
        // Treating it as a Box would copy a borrowed address into an owning handle slot.
        return doesValueTypeMatch(expectedTypeId, getValueTypeId(valueId));
    }

    bool validateRecord(const Record& record)
    {
        for (size_t operandIndex = 0; operandIndex < record.operands.size(); ++operandIndex)
        {
            const HGBCOperandInfo* operandInfo = getHGBCOperandInfo(record.opcode, operandIndex);
            SLANG_RELEASE_ASSERT(operandInfo);

            HGBCId operand = record.operands[operandIndex];
            if (operandInfo->isRef && !findResultRecord(operand))
                return fail(record, "references a missing value");
            if (operandInfo->kind == HGBCOperandKind::TypeId && !findTypeRecord(operand))
                return fail(record, "references a missing type");
            if (operandInfo->kind == HGBCOperandKind::Bool && operand > 1)
                return fail(record, "has an invalid boolean operand");
            if (operandInfo->kind == HGBCOperandKind::Enum &&
                !getHGBCEnumValueName(operandInfo->enumKind, operand))
            {
                return fail(record, "has an invalid enum operand");
            }
        }
        const HGBC::BasicIndex& index = m_program.getIndex();
        if (record.opcode == HGBCOpcode::Decorate &&
            record.operands[1] == uint32_t(HGBCDecorationKind::TypeMangledName))
        {
            if (!findTypeRecord(record.operands[2]))
                return fail(record, "type identity must decorate a type");
            auto name = index.strings.find(record.operands[3]);
            if (name == index.strings.end() || name->second.empty())
                return fail(record, "type identity must reference a nonempty string");
            auto& decorations = m_decodedProgram.decorationRecordIndices.at(record.operands[2]);
            auto& identities = decorations.at(uint32_t(HGBCDecorationKind::TypeMangledName));
            if (identities.getCount() != 1)
                return fail(record, "type identity must be defined exactly once");
        }
        if (record.opcode == HGBCOpcode::IntLit &&
            !index.uintTypeIds.contains(record.operands[0]) &&
            !index.intTypeIds.contains(record.operands[0]))
        {
            return fail(record, "must have an integer type");
        }
        if (record.opcode == HGBCOpcode::FloatLit &&
            !index.floatTypeIds.contains(record.operands[0]))
        {
            return fail(record, "must have a floating-point type");
        }
        if (record.opcode == HGBCOpcode::FloatLit &&
            record.operands.size() != (index.float64TypeIds.contains(record.operands[0]) ? 4 : 3))
        {
            return fail(record, "literal payload must match its floating-point width");
        }
        if (record.opcode == HGBCOpcode::FloatLit &&
            index.float16TypeIds.contains(record.operands[0]) && (record.operands[2] >> 16) != 0)
        {
            return fail(record, "binary16 literal has nonzero upper payload bits");
        }
        if (record.opcode == HGBCOpcode::BoolLit && !index.boolTypeIds.contains(record.operands[0]))
        {
            return fail(record, "must have a bool type");
        }
        if (record.opcode == HGBCOpcode::StringLit &&
            !index.stringTypeIds.contains(record.operands[0]))
        {
            return fail(record, "must have a string type");
        }
        if (record.opcode == HGBCOpcode::NullLit && !isNullableManagedType(record.operands[0]))
        {
            return fail(record, "must have a nullable managed value type");
        }
        if (record.opcode == HGBCOpcode::DescriptorHandleType &&
            !isDescriptorResourceType(record.operands[1]))
        {
            return fail(
                record,
                "must reference a structured-buffer, texture, or top-level acceleration-structure "
                "type");
        }
        if (record.opcode == HGBCOpcode::VectorType)
        {
            NumericType type;
            if (record.operands[1] < 1 || record.operands[1] > 4 ||
                !tryGetNumericType(record.operands[0], type))
            {
                return fail(record, "must describe one to four basic scalar lanes");
            }
        }
        if (record.opcode == HGBCOpcode::MatrixType)
        {
            NumericType type;
            if (record.operands[1] < 1 || record.operands[1] > 4 || record.operands[2] < 1 ||
                record.operands[2] > 4 || !tryGetNumericType(record.operands[0], type))
            {
                return fail(record, "must describe a one-to-four by one-to-four scalar matrix");
            }
        }
        if (record.opcode == HGBCOpcode::Texture2DType ||
            record.opcode == HGBCOpcode::RenderTarget2DType)
        {
            NumericType elementType;
            if (!tryGetNumericType(record.operands[1], elementType) ||
                elementType.kind == NumericType::Kind::Matrix)
            {
                return fail(record, "must have a scalar or vector numeric shader element type");
            }

            HGBCResourceFormat format = getTextureFormat(record.operands[0]);
            if (format != HGBCResourceFormat::Undefined &&
                !isTextureFormatCompatible(record.operands[1], format))
            {
                return fail(record, "storage format does not match its shader element type");
            }
            if (isHGBCDepthFormat(format) &&
                (record.opcode != HGBCOpcode::Texture2DType ||
                 getTextureAccess(record.operands[0]) != HGBCTextureAccess::ReadOnly))
            {
                return fail(
                    record,
                    "depth/stencil storage requires a shader read-only texture type");
            }
        }
        if (record.opcode == HGBCOpcode::SurfaceType &&
            !isSurfaceFormat(HGBCResourceFormat(record.operands[1])))
        {
            return fail(record, "must use a supported presentation format");
        }
        return true;
    }

    // Ensures runtime aggregate layouts can be built without discovering another program error.
    bool validateRuntimeAggregateLayouts()
    {
        enum class State : uint8_t
        {
            Unvisited,
            Visiting,
            Complete,
        };
        std::unordered_map<HGBCId, State> states;
        std::unordered_map<HGBCId, size_t> slotCounts;
        std::function<bool(HGBCId, size_t&)> validate = [&](HGBCId typeId,
                                                            size_t& outSlotCount) -> bool
        {
            const Record* record = findTypeRecord(typeId);
            bool isStruct = record && record->opcode == HGBCOpcode::StructType;
            bool isArray = record && record->opcode == HGBCOpcode::ArrayType;
            if (!isStruct && !isArray)
            {
                outSlotCount = 1;
                return true;
            }

            State& state = states[typeId];
            if (state == State::Visiting)
                return fail(*record, "participates in a recursive-by-value aggregate layout");
            if (state == State::Complete)
            {
                outSlotCount = slotCounts[typeId];
                return true;
            }
            state = State::Visiting;

            size_t slotCount = 0;
            if (isArray)
            {
                if (record->operands.size() != 3)
                    return fail(*record, "has malformed fixed-size array type operands");
                size_t elementSlotCount = 0;
                if (!validate(record->operands[2], elementSlotCount))
                    return false;
                if (record->operands[1] &&
                    elementSlotCount > std::numeric_limits<size_t>::max() / record->operands[1])
                {
                    return fail(*record, "has a runtime layout that is too large");
                }
                slotCount = elementSlotCount * record->operands[1];
            }
            else
            {
                const HGBC::StructTypeDescriptor* descriptor = m_program.findStructType(typeId);
                SLANG_RELEASE_ASSERT(descriptor);
                for (const HGBC::StructFieldDescriptor& field : descriptor->fields)
                {
                    size_t fieldSlotCount = 0;
                    if (!validate(field.typeId, fieldSlotCount))
                        return false;
                    if (fieldSlotCount > std::numeric_limits<size_t>::max() - slotCount)
                        return fail(*record, "has a runtime layout that is too large");
                    slotCount += fieldSlotCount;
                }
            }

            state = State::Complete;
            slotCounts[typeId] = slotCount;
            outSlotCount = slotCount;
            return true;
        };

        for (const HGBC::StructTypeDescriptor& descriptor :
             m_program.getDecodedProgram().structTypes)
        {
            size_t ignored = 0;
            if (!validate(descriptor.typeId, ignored))
                return false;
        }
        for (const Record& record : m_decodedProgram.records)
        {
            if (record.opcode != HGBCOpcode::ArrayType)
                continue;
            size_t ignored = 0;
            if (!validate(record.operands[0], ignored))
                return false;
        }
        return true;
    }

    bool isEntryPoint(HGBCId id) const
    {
        const Record* record = findResultRecord(id);
        return record && (record->opcode == HGBCOpcode::SlangEntryPoint ||
                          record->opcode == HGBCOpcode::SpecializeSlangEntryPoint);
    }

    bool validateSlangModule(const Record& record)
    {
        if (!m_program.findString(record.operands[1]))
            return fail(record, "does not reference a module name");
        const Record* binary = findResultRecord(record.operands[2]);
        if (!binary || binary->opcode != HGBCOpcode::BinaryLit)
            return fail(record, "does not reference a binary literal");
        for (size_t i = 3; i < record.operands.size(); ++i)
        {
            const Record* dependency = findResultRecord(record.operands[i]);
            if (!dependency || dependency->opcode != HGBCOpcode::SlangModule)
                return fail(record, "dependency does not reference a module");
        }
        return true;
    }

    bool validateSlangEntryPoint(const Record& record)
    {
        const Record* module = findResultRecord(record.operands[1]);
        if (!module || module->opcode != HGBCOpcode::SlangModule)
            return fail(record, "does not reference a module");
        if (!m_program.findString(record.operands[2]))
            return fail(record, "does not reference an entry-point name");
        return true;
    }

    bool validateSpecializedEntryPoint(const Record& record)
    {
        if (!isEntryPoint(record.operands[1]))
            return fail(record, "does not reference a base entry point");
        for (size_t i = 2; i < record.operands.size(); i += 2)
        {
            if (!m_program.findString(record.operands[i + 1]))
                return fail(record, "specialization argument does not reference a string");
        }
        return true;
    }

    bool validateComputeDispatcher(const Record& record)
    {
        SLANG_RELEASE_ASSERT(record.operands.size() >= 4);
        if (record.operands[2] != uint32_t(HGBCDispatchStage::Compute))
            return fail(record, "does not use the compute dispatch stage");

        const Record* type = findTypeRecord(record.operands[0]);
        if (!type || type->opcode != HGBCOpcode::FuncType || type->operands.size() < 3 ||
            !isUInt3Type(type->operands[2]))
        {
            return fail(record, "must accept a uint3 thread-group count");
        }
        if (!isEntryPoint(record.operands[3]))
            return fail(record, "does not reference an entry point");
        const size_t argumentCount = type->operands.size() - 3;
        if (record.operands.size() != argumentCount + 4)
            return fail(record, "layout-converter count does not match its function type");
        for (size_t argumentIndex = 0; argumentIndex < argumentCount; ++argumentIndex)
        {
            const Record* converter = findLayoutConverterRecord(record.operands[argumentIndex + 4]);
            const Record* source =
                converter ? findTypeLayoutRecord(converter->operands[1]) : nullptr;
            const Record* destination =
                converter ? findTypeLayoutRecord(converter->operands[2]) : nullptr;
            if (!source || !destination ||
                !doesValueTypeMatch(source->operands[1], type->operands[argumentIndex + 3]) ||
                HGBCLayoutDomain(source->operands[2]) != HGBCLayoutDomain::Host ||
                HGBCLayoutDomain(destination->operands[2]) != HGBCLayoutDomain::ShaderObject ||
                destination->operands[4] != record.operands[1] ||
                (destination->operands.size() == 7 && destination->operands[6] != 0))
            {
                return fail(record, "argument converter does not match its shader parameter");
            }
        }
        return true;
    }

    /// Validates the typed clear payload independently of the resource's raster role.
    /// For example, clearing only stencil takes a uint even though the depth texture exposes
    /// float shader values. A combined clear carries a float followed by one uint operand.
    bool validateTextureClear(const Record& record)
    {
        using O = HGBCClearTextureOperands;
        const auto& v = record.operands;
        if (!isTextureType(v[O::textureType]) ||
            !isValueCompatible(v[O::textureType], v[O::texture]))
            return fail(record, "declared texture type does not match the clear target");
        for (uint32_t i = O::mip; i <= O::layerCount; ++i)
            if (!isTypeOpcode(getEffectiveValueTypeId(v[i]), HGBCOpcode::UIntType))
                return fail(record, "clear subresource bounds must be uint values");
        auto format = getTextureFormat(v[O::textureType]);
        auto aspect = HGBCTextureAspect(v[O::aspect]);
        auto valueType = getEffectiveValueTypeId(v[O::clearValue]);
        if (isHGBCDepthFormat(format))
        {
            bool stencil = hasHGBCStencilAspect(format);
            if (aspect == HGBCTextureAspect::StencilOnly)
            {
                if (!stencil || v.size() != 8 || !isTypeOpcode(valueType, HGBCOpcode::UIntType))
                    return fail(
                        record,
                        "stencil-only clear requires a stencil format and uint value");
            }
            else
            {
                bool both = stencil && aspect == HGBCTextureAspect::All;
                if (!isTypeOpcode(valueType, HGBCOpcode::Float32Type) ||
                    v.size() != (both ? 9 : 8) ||
                    (both && !isTypeOpcode(getEffectiveValueTypeId(v[8]), HGBCOpcode::UIntType)))
                    return fail(
                        record,
                        "depth clear requires float depth and, when selected, uint stencil");
            }
            return true;
        }
        NumericType value;
        const Record* textureType = findTypeRecord(v[O::textureType]);
        if (aspect != HGBCTextureAspect::All || v.size() != 8 ||
            !doesValueTypeMatch(textureType->operands[1], valueType) ||
            !tryGetNumericType(valueType, value) || value.kind == NumericType::Kind::Matrix ||
            (value.kind == NumericType::Kind::Vector &&
             (value.columnCount < 1 || value.columnCount > 4)) ||
            (!isTypeOpcode(value.scalarTypeId, HGBCOpcode::Float32Type) &&
             !isTypeOpcode(value.scalarTypeId, HGBCOpcode::UIntType) &&
             !isTypeOpcode(value.scalarTypeId, HGBCOpcode::IntType)))
            return fail(
                record,
                "color clear requires All aspects and matching 32-bit scalar/vector values");
        return true;
    }

    bool validateGraphicsPipeline(const Record& record)
    {
        if (record.opcode == HGBCOpcode::GraphicsPipelineState)
        {
            using O = HGBCGraphicsPipelineStateOperands;
            auto& v = record.operands;
            if (v[O::fillMode] > 1 || v[O::cullMode] > 2 || v[O::frontFace] > 1 ||
                v[O::frontStencilFailOp] > 7 || v[O::frontStencilDepthFailOp] > 7 ||
                v[O::frontStencilPassOp] > 7 || v[O::backStencilFailOp] > 7 ||
                v[O::backStencilDepthFailOp] > 7 || v[O::backStencilPassOp] > 7)
                return fail(record, "contains an invalid rasterizer or stencil setting");
            return true;
        }
        if (record.opcode == HGBCOpcode::GraphicsColorTargetState)
        {
            using O = HGBCGraphicsColorTargetStateOperands;
            auto& v = record.operands;
            const Record* texture = findTypeRecord(v[O::textureType]);
            if (!texture || texture->opcode != HGBCOpcode::Texture2DType ||
                texture->operands[2] == uint32_t(HGBCTextureAccess::ReadOnly) ||
                texture->operands[3] == uint32_t(HGBCResourceFormat::Undefined) ||
                isHGBCDepthFormat(HGBCResourceFormat(texture->operands[3])))
                return fail(record, "requires a writable texture with a known color format");
            if (v[O::logicOp] || v[O::writeMask] > 15 || v[O::colorSrcFactor] > 16 ||
                v[O::colorDstFactor] > 16 || v[O::alphaSrcFactor] > 16 ||
                v[O::alphaDstFactor] > 16 || v[O::colorOp] > 4 || v[O::alphaOp] > 4)
                return fail(record, "contains an invalid blend or write-mask setting");
            return true;
        }
        if (record.opcode == HGBCOpcode::GraphicsAttachments)
        {
            auto depthFormat = HGBCResourceFormat(record.operands[1]);
            if (depthFormat != HGBCResourceFormat::Undefined && !isHGBCDepthFormat(depthFormat))
                return fail(record, "graphics depth attachment must use a depth/stencil format");
            for (size_t i = 2; i < record.operands.size(); ++i)
            {
                const Record* color = findResultRecord(record.operands[i]);
                if (!color || color->opcode != HGBCOpcode::GraphicsColorTargetState)
                    return fail(record, "requires ordered color-target state records");
            }
            return true;
        }
        const Record* type = findTypeRecord(record.operands[0]);
        const Record* state = findResultRecord(record.operands[4]);
        const Record* attachments = findResultRecord(record.operands[5]);
        if (!type || type->opcode != HGBCOpcode::FuncType ||
            !isTypeOpcode(type->operands[1], HGBCOpcode::VoidType) ||
            !isEntryPoint(record.operands[2]) || !isEntryPoint(record.operands[3]) || !state ||
            state->opcode != HGBCOpcode::GraphicsPipelineState || !attachments ||
            attachments->opcode != HGBCOpcode::GraphicsAttachments)
            return fail(record, "has an invalid graphics pipeline dependency");
        bool hasDepth = attachments->operands[1] != uint32_t(HGBCResourceFormat::Undefined);
        uint32_t drawCount = record.operands[6];
        if (attachments->operands.size() != 3 || drawCount != (hasDepth ? 4u : 3u) ||
            type->operands.size() < 2 + drawCount)
            return fail(record, "uses an unsupported attachment/draw signature");
        const Record* color = findResultRecord(attachments->operands[2]);
        if (!color || color->operands[1] != type->operands[2] ||
            (hasDepth &&
             (!isTextureType(type->operands[3]) ||
              uint32_t(getTextureFormat(type->operands[3])) != attachments->operands[1])) ||
            !isTypeOpcode(type->operands[drawCount], HGBCOpcode::GraphicsDrawType) ||
            !isTypeOpcode(type->operands[drawCount + 1], HGBCOpcode::GraphicsDrawParametersType))
            return fail(record, "callable target and draw types do not match the pipeline");
        using O = HGBCGraphicsPipelineStateOperands;
        if (!hasDepth && (state->operands[O::depthTestEnable] || state->operands[O::stencilEnable]))
            return fail(record, "enables depth/stencil testing without a depth attachment");
        size_t argumentCount = type->operands.size() - 2 - drawCount;
        if (record.operands.size() != 8 + argumentCount || record.operands[7] > argumentCount)
            return fail(record, "has an inconsistent shader argument count");
        for (size_t i = 0; i < argumentCount; ++i)
        {
            const Record* converter = findLayoutConverterRecord(record.operands[8 + i]);
            const Record* source =
                converter ? findTypeLayoutRecord(converter->operands[1]) : nullptr;
            const Record* destination =
                converter ? findTypeLayoutRecord(converter->operands[2]) : nullptr;
            if (!source || !destination ||
                source->operands[1] != type->operands[2 + drawCount + i] ||
                HGBCLayoutDomain(source->operands[2]) != HGBCLayoutDomain::Host ||
                HGBCLayoutDomain(destination->operands[2]) != HGBCLayoutDomain::ShaderObject ||
                destination->operands[4] != record.operands[1] ||
                (destination->operands.size() == 7 ? destination->operands[6] : 0) !=
                    (i < record.operands[7] ? 0u : 1u))
                return fail(record, "argument converter does not match its stage and parameter");
        }
        return true;
    }

    bool validateRayTracer(const Record& record)
    {
        SLANG_RELEASE_ASSERT(record.operands.size() == 7);
        const Record* type = findTypeRecord(record.operands[0]);
        if (!type || type->opcode != HGBCOpcode::FuncType || type->operands.size() < 3 ||
            !isTypeOpcode(type->operands[1], HGBCOpcode::VoidType) ||
            !isUInt3Type(type->operands[2]))
        {
            return fail(record, "must be a void callable beginning with a uint3 dispatch size");
        }
        if (!findTypeRecord(record.operands[2]))
            return fail(record, "does not reference a payload type");
        const HGBC::TypeSizeDescriptor* payloadLayout = m_program.findTypeSize(record.operands[2]);
        if (!payloadLayout || !payloadLayout->byteSize)
            return fail(record, "payload type must have a nonzero byte layout");
        if (record.operands[3] < 1)
            return fail(record, "has an invalid recursion depth");
        if (!isEntryPoint(record.operands[4]) || !isEntryPoint(record.operands[5]) ||
            !isEntryPoint(record.operands[6]))
        {
            return fail(
                record,
                "does not reference ray-generation, closest-hit, and miss entry points");
        }
        return true;
    }

    /// Checks the stage roles declared by a hit group without imposing an interpreter's supported
    /// subset. An any-hit-only triangle group and an intersection-only procedural group are valid
    /// HGBC even if a particular runtime cannot execute them yet.
    bool validateRayHitGroup(const Record& record)
    {
        HashSet<uint32_t> roles;
        for (size_t i = 4; i < record.operands.size(); i += 2)
        {
            if (!roles.add(record.operands[i]))
                return fail(record, "declares a duplicate hit-group stage role");
            if (!isEntryPoint(record.operands[i + 1]))
                return fail(record, "hit-group stage does not reference an entry point");
        }
        bool hasIntersection = roles.contains(uint32_t(HGBCRayStageRole::Intersection));
        if (HGBCRayGeometryKind(record.operands[1]) == HGBCRayGeometryKind::Triangles)
        {
            if (hasIntersection)
                return fail(record, "a triangle hit group cannot declare an intersection stage");
        }
        else if (!hasIntersection)
        {
            return fail(record, "a procedural hit group requires an intersection stage");
        }
        return true;
    }

    /// Checks the typed uniform records shared by ray registration and dispatch. Parameter indices
    /// identify reflected shader parameters, so gaps are allowed after empty parameters are erased.
    /// Hit-group parameters are keyed by stage role as well as index: closest-hit parameter zero
    /// and any-hit parameter zero are distinct bindings, regardless of their record order.
    bool validateRayUniformArguments(
        const Record& record,
        size_t firstArgument,
        const Record* hitGroup = nullptr)
    {
        HashSet<uint32_t> roles;
        if (hitGroup)
        {
            for (size_t i = 4; i < hitGroup->operands.size(); i += 2)
                roles.add(hitGroup->operands[i]);
        }
        HashSet<uint64_t> parameters;
        for (size_t i = firstArgument; i < record.operands.size();)
        {
            uint32_t role = 0;
            if (hitGroup)
            {
                role = record.operands[i++];
                if (!roles.contains(role))
                    return fail(record, "uniform argument refers to an undeclared hit-group stage");
            }
            uint32_t parameterIndex = record.operands[i++];
            if (!parameters.add((uint64_t(role) << 32) | parameterIndex))
                return fail(record, "declares a duplicate uniform parameter binding");
            HGBCId argumentType = record.operands[i++];
            HGBCId argument = record.operands[i++];
            // These records copy the declared value itself. Unlike an OpLoad, registration does
            // not implicitly dereference an address whose pointee happens to have this type.
            if (!doesValueTypeMatch(argumentType, getValueTypeId(argument)))
                return fail(record, "uniform argument value does not match its declared type");
        }
        return true;
    }

    bool validateRayRegistration(const Record& record)
    {
        if (!isTypeOpcode(getValueTypeId(record.operands[0]), HGBCOpcode::RaytracingPipelineType))
        {
            return fail(record, "requires a ray-tracing pipeline operand");
        }
        size_t keyCount = record.opcode == HGBCOpcode::SetRayMiss ? 1 : 2;
        for (size_t i = 1; i <= keyCount; ++i)
        {
            if (!isTypeOpcode(getValueTypeId(record.operands[i]), HGBCOpcode::UIntType))
                return fail(record, "registration slots must be uint32 values");
        }
        if (record.opcode == HGBCOpcode::SetRayMiss)
        {
            if (!isEntryPoint(record.operands[2]))
                return fail(record, "miss shader does not reference an entry point");
            return validateRayUniformArguments(record, 4);
        }
        const Record* hitGroup = findResultRecord(record.operands[3]);
        if (!hitGroup || hitGroup->opcode != HGBCOpcode::RayHitGroup)
            return fail(record, "does not reference a hit-group descriptor");
        return validateRayUniformArguments(record, 4, hitGroup);
    }

    bool validateRayDispatch(const Record& record)
    {
        if (!isEntryPoint(record.operands[0]))
            return fail(record, "ray-generation shader does not reference an entry point");
        if (!isUInt3Type(getValueTypeId(record.operands[1])))
            return fail(record, "dispatch extent must be a uint3 value");
        if (!record.operands[2])
            return fail(record, "has an invalid recursion depth");
        return validateRayUniformArguments(record, 3);
    }

    /// Finds an AS descriptor field by its schema role. Struct field order is not an ABI: a
    /// legal descriptor may declare `indices` before `vertices`, for example, without changing
    /// which resource supplies vertex data to the native build.
    HGBCId findAccelerationStructureFieldType(
        const HGBC::StructTypeDescriptor& descriptor,
        const char* name) const
    {
        auto field = descriptor.fieldIndices.find(name);
        return field == descriptor.fieldIndices.end() ? 0 : descriptor.fields[field->second].typeId;
    }

    bool validateAccelerationStructureBuild(const Record& record)
    {
        const auto kind = HGBCAccelerationStructureKind(record.operands[0]);
        const auto inputKind = HGBCAccelerationStructureInputKind(record.operands[1]);
        const bool bottomLevel = kind == HGBCAccelerationStructureKind::BottomLevel;
        if (bottomLevel == (inputKind == HGBCAccelerationStructureInputKind::Instances))
            return fail(record, "build input kind does not match the acceleration-structure kind");
        HGBCOpcode structureType = bottomLevel
                                       ? HGBCOpcode::RaytracingBottomLevelAccelerationStructureType
                                       : HGBCOpcode::RaytracingAccelerationStructureType;
        if (!isTypeOpcode(getValueTypeId(record.operands[3]), structureType))
            return fail(record, "build target does not match the acceleration-structure kind");
        const Record* arrayType = findTypeRecord(getValueTypeId(record.operands[4]));
        if (!arrayType || arrayType->opcode != HGBCOpcode::RuntimeArrayType ||
            !doesValueTypeMatch(record.operands[2], arrayType->operands[1]))
            return fail(record, "build inputs must be a runtime array of the declared input type");
        if (!isTypeOpcode(getValueTypeId(record.operands[5]), HGBCOpcode::UIntType) ||
            !isTypeOpcode(getValueTypeId(record.operands[6]), HGBCOpcode::UIntType))
            return fail(record, "build flags and mode must be uint32 values");

        const auto* descriptor = m_program.findStructType(record.operands[2]);
        const bool aabbs = inputKind == HGBCAccelerationStructureInputKind::AABBs;
        if (!descriptor || descriptor->fields.size() != size_t(aabbs ? 4 : bottomLevel ? 7 : 5))
            return fail(record, "build input type has an invalid descriptor shape");
        if (aabbs)
        {
            auto buffer = findAccelerationStructureFieldType(*descriptor, "bounds");
            if (!isBufferType(buffer))
                return fail(record, "procedural bounds must be a structured buffer");
            auto bounds = m_program.findStructType(getBufferElementTypeId(buffer));
            if (!bounds || bounds->fields.size() != 6)
                return fail(record, "procedural AABBs must contain six packed float32 scalars");
            for (const auto& field : bounds->fields)
                if (!isTypeOpcode(field.typeId, HGBCOpcode::Float32Type))
                    return fail(record, "procedural AABBs must contain six packed float32 scalars");
            for (const char* name : {"primitiveOffset", "primitiveCount", "flags"})
                if (!isTypeOpcode(
                        findAccelerationStructureFieldType(*descriptor, name),
                        HGBCOpcode::UIntType))
                    return fail(record, "procedural ranges and flags must be uint32 fields");
        }
        else if (bottomLevel)
        {
            HGBCId vertices = findAccelerationStructureFieldType(*descriptor, "vertices");
            HGBCId indices = findAccelerationStructureFieldType(*descriptor, "indices");
            if (!isBufferType(vertices) || !isFloat3Type(getBufferElementTypeId(vertices)))
                return fail(record, "triangle vertices must be a structured buffer of float3");
            if (!isBufferType(indices) ||
                !isTypeOpcode(getBufferElementTypeId(indices), HGBCOpcode::UIntType))
                return fail(record, "triangle indices must be a structured buffer of uint32");
            for (const char* name :
                 {"vertexOffset", "vertexCount", "indexOffset", "indexCount", "flags"})
            {
                if (!isTypeOpcode(
                        findAccelerationStructureFieldType(*descriptor, name),
                        HGBCOpcode::UIntType))
                    return fail(record, "triangle ranges and flags must be uint32 fields");
            }
        }
        else
        {
            if (!isTypeOpcode(
                    findAccelerationStructureFieldType(*descriptor, "blas"),
                    HGBCOpcode::RaytracingBottomLevelAccelerationStructureType))
                return fail(record, "instance blas must be a bottom-level acceleration structure");
            NumericType transform;
            if (!tryGetNumericType(
                    findAccelerationStructureFieldType(*descriptor, "transform"),
                    transform) ||
                transform.kind != NumericType::Kind::Matrix || transform.rowCount != 3 ||
                transform.columnCount != 4 ||
                !isTypeOpcode(transform.scalarTypeId, HGBCOpcode::Float32Type))
                return fail(record, "instance transform must be a float3x4 matrix");
            for (const char* name : {"instanceID", "mask", "flags"})
            {
                if (!isTypeOpcode(
                        findAccelerationStructureFieldType(*descriptor, name),
                        HGBCOpcode::UIntType))
                    return fail(record, "instance ID, mask, and flags must be uint32 fields");
            }
        }
        return true;
    }

    bool validateRaytracingScene(const Record& record)
    {
        SLANG_RELEASE_ASSERT(record.operands.size() == 9);
        if (!isTypeOpcode(record.operands[0], HGBCOpcode::RaytracingAccelerationStructureType))
            return fail(record, "must produce a ray-tracing acceleration structure");

        if (!m_program.getIndex().uintTypeIds.contains(
                getEffectiveValueTypeId(record.operands[5])) ||
            !m_program.getIndex().uintTypeIds.contains(getEffectiveValueTypeId(record.operands[7])))
        {
            return fail(record, "vertex and index counts must be uint values");
        }

        HGBCId vertexBufferType = getEffectiveValueTypeId(record.operands[6]);
        HGBCId indexBufferType = getEffectiveValueTypeId(record.operands[8]);
        if (!isBufferType(vertexBufferType) ||
            !isFloat3Type(getBufferElementTypeId(vertexBufferType)))
        {
            return fail(record, "vertex buffer must contain float3 positions");
        }
        if (!isBufferType(indexBufferType) ||
            !m_program.getIndex().uintTypeIds.contains(getBufferElementTypeId(indexBufferType)))
        {
            return fail(record, "index buffer must contain uint32 indices");
        }
        return true;
    }

    bool validateCall(const Record& record)
    {
        const Record* callee = findResultRecord(record.operands[2]);
        HGBCId calleeTypeId = callee ? getValueTypeId(record.operands[2]) : 0;
        const Record* calleeType = findTypeRecord(calleeTypeId);
        if (!calleeType || calleeType->opcode != HGBCOpcode::FuncType ||
            calleeType->operands.size() < 2)
        {
            return fail(record, "callee does not have a function type");
        }

        if (!doesValueTypeMatch(calleeType->operands[1], record.operands[0]))
            return fail(record, "result type does not match the callee return type");

        size_t argumentCount = record.operands.size() - 3;
        size_t parameterCount = calleeType->operands.size() - 2;
        if (argumentCount != parameterCount)
            return fail(record, "argument count does not match the callee function type");

        for (size_t i = 0; i < argumentCount; ++i)
        {
            HGBCId expectedTypeId = calleeType->operands[i + 2];
            HGBCId argumentId = record.operands[i + 3];
            if (!isValueCompatible(expectedTypeId, argumentId))
            {
                HGBCId actualTypeId = getValueTypeId(argumentId);
                const Record* argument = findResultRecord(argumentId);
                const Record* actualType = findTypeRecord(actualTypeId);
                const Record* expectedType = findTypeRecord(expectedTypeId);
                return fail(
                    record,
                    "argument " + std::to_string(i) + " from " +
                        (argument ? getHGBCOpcodeName(argument->opcode) : "<missing>") +
                        " has type %" + std::to_string(actualTypeId) + " (" +
                        (actualType ? getHGBCOpcodeName(actualType->opcode) : "<missing>") +
                        "), but the callee parameter expects type %" +
                        std::to_string(expectedTypeId) + " (" +
                        (expectedType ? getHGBCOpcodeName(expectedType->opcode) : "<missing>") +
                        ")");
            }
        }
        return true;
    }

    bool validateMakeStruct(const Record& record)
    {
        const HGBC::StructTypeDescriptor* type = m_program.findStructType(record.operands[0]);
        if (!type || record.operands.size() - 2 != type->fields.size())
            return fail(record, "has malformed struct construction operands");

        for (size_t i = 0; i < type->fields.size(); ++i)
        {
            if (!isValueCompatible(type->fields[i].typeId, record.operands[i + 2]))
            {
                return fail(
                    record,
                    "field " + std::to_string(i) + " does not match the struct field type");
            }
        }
        return true;
    }

    bool validateMakeMatrix(const Record& record)
    {
        const Record* type = findTypeRecord(record.operands[0]);
        if (!type || type->opcode != HGBCOpcode::MatrixType || type->operands.size() != 5)
            return fail(record, "result type is malformed");

        for (size_t operandIndex = 2; operandIndex < record.operands.size(); ++operandIndex)
        {
            if (!isValueCompatible(type->operands[4], record.operands[operandIndex]))
                return fail(record, "element does not match the matrix element type");
        }
        if (record.operands.size() - 2 != size_t(type->operands[1]) * type->operands[2])
            return fail(record, "element count does not match the result type");
        return true;
    }

    bool validateMakeVector(const Record& record)
    {
        const Record* type = findTypeRecord(record.operands[0]);
        if (!type || type->opcode != HGBCOpcode::VectorType || type->operands.size() != 3)
            return fail(record, "result type is malformed");
        if (record.operands.size() - 2 != type->operands[1])
            return fail(record, "element count does not match the result type");
        for (size_t operandIndex = 2; operandIndex < record.operands.size(); ++operandIndex)
        {
            if (!isValueCompatible(type->operands[2], record.operands[operandIndex]))
                return fail(record, "element does not match the vector element type");
        }
        return true;
    }

    bool validateMakeArray(const Record& record)
    {
        const Record* type = findTypeRecord(record.operands[0]);
        if (!type || type->opcode != HGBCOpcode::ArrayType || type->operands.size() != 3)
            return fail(record, "result type is malformed");
        if (record.operands.size() - 2 != type->operands[1])
            return fail(record, "element count does not match the result type");
        for (size_t operandIndex = 2; operandIndex < record.operands.size(); ++operandIndex)
        {
            if (!isValueCompatible(type->operands[2], record.operands[operandIndex]))
                return fail(record, "element does not match the array element type");
        }
        return true;
    }

    // Ensures that local storage exposes an address-typed result.
    //
    // Consider `%local : Ptr<uint> = OpVar`: the result contains an address, while `uint` is the
    // separately allocated value type. This is an HGBC representation rule shared by every
    // interpreter, not a HGVM capability restriction.
    bool validateVar(const Record& record)
    {
        if (!getAddressValueTypeId(record.operands[0]))
            return fail(record, "result type is not an address type");
        return true;
    }

    // Ensures that a raw load's result type is the value type addressed by its operand.
    //
    // For example, loading `Ptr<uint>` produces `uint`; loading `Ptr<Ptr<uint>>` may itself
    // produce an address. The type structure determines both cases without inspecting the
    // operation that originally produced the pointer operand.
    bool validateLoad(const Record& record)
    {
        HGBCId addressedTypeId = getAddressedTypeId(record.operands[2]);
        if (!addressedTypeId || !doesValueTypeMatch(record.operands[0], addressedTypeId))
            return fail(record, "pointer does not address the result type");
        return true;
    }

    /// Checks the image format shared by strict and recoverable surface acquisition.
    bool validateAcquiredSurfaceTexture(
        const Record& record,
        HGBCId textureTypeId,
        HGBCId surfaceValueId)
    {
        const Record* surfaceType = findTypeRecord(getEffectiveValueTypeId(surfaceValueId));
        if (!isTypeOpcode(textureTypeId, HGBCOpcode::RenderTarget2DType) || !surfaceType ||
            surfaceType->opcode != HGBCOpcode::SurfaceType || surfaceType->operands.size() != 2 ||
            getTextureFormat(textureTypeId) != HGBCResourceFormat(surfaceType->operands[1]))
        {
            return fail(record, "result texture format must match the Surface operand");
        }
        return true;
    }

    // Ensures that a raw store's source matches the value type addressed by its destination.
    //
    // Consider `OpStore %pairAddress %pairValue`: validation resolves `Pair` from the address
    // wrapper once. Execution can then remain a fixed-size copy with no producer-opcode lookup.
    bool validateStore(const Record& record)
    {
        HGBCId addressedTypeId = getAddressedTypeId(record.operands[0]);
        if (!addressedTypeId || !isValueCompatible(addressedTypeId, record.operands[1]))
            return fail(record, "value does not match the pointer's addressed type");
        return true;
    }

    // Ensures that a field address has an address result wrapping the selected field type.
    //
    // Consider `%weight : Ptr<float> = OpFieldAddress %sample %weightName`: the base wrapper
    // identifies `Sample`, and its field table identifies `float`. Neither the validator nor an
    // interpreter needs to infer address-ness from `OpFieldAddress` itself.
    bool validateFieldAddress(const Record& record)
    {
        HGBCId baseTypeId = getAddressedTypeId(record.operands[2]);
        const HGBC::StructTypeDescriptor* baseType = findAggregateType(baseTypeId);
        const std::string* fieldName = m_program.findString(record.operands[3]);
        if (!baseType || !fieldName)
            return fail(record, "base is not an aggregate storage reference");
        auto fieldIt = baseType->fieldIndices.find(*fieldName);
        if (fieldIt == baseType->fieldIndices.end())
            return fail(record, "field does not exist in the base aggregate");
        HGBCId resultValueTypeId = getAddressValueTypeId(record.operands[0]);
        if (!resultValueTypeId ||
            !doesValueTypeMatch(resultValueTypeId, baseType->fields[fieldIt->second].typeId))
        {
            return fail(record, "result type does not match the aggregate field type");
        }
        return true;
    }

    bool validateFieldExtract(const Record& record)
    {
        HGBCId baseTypeId = getValueTypeId(record.operands[2]);
        const HGBC::StructTypeDescriptor* baseType = findAggregateType(baseTypeId);
        const std::string* fieldName = m_program.findString(record.operands[3]);
        if (!baseType || !fieldName)
            return fail(record, "base is not an aggregate value");
        auto fieldIt = baseType->fieldIndices.find(*fieldName);
        if (fieldIt == baseType->fieldIndices.end())
            return fail(record, "field does not exist in the base aggregate");
        if (!doesValueTypeMatch(record.operands[0], baseType->fields[fieldIt->second].typeId))
            return fail(record, "result type does not match the aggregate field type");
        return true;
    }

    /// Validates construction while the payload still has its concrete ownership-bearing type.
    bool validateBox(const Record& record)
    {
        if (!isTypeOpcode(record.operands[0], HGBCOpcode::BoxType))
            return fail(record, "result type must be a box");
        if (!isOwnedBoxPayloadType(record.operands[2]))
            return fail(record, "payload must be an owning value type without borrowed addresses");
        // These compiler-owned operations consume values. A storage reference requires an
        // explicit OpLoad, otherwise staging would interpret the address token as payload bytes.
        if (!doesValueTypeMatch(record.operands[2], getValueTypeId(record.operands[3])))
            return fail(record, "payload does not match its declared concrete type");
        return true;
    }

    /// Leaves the concrete-type check to execution, so safety does not depend on control flow.
    bool validateUnbox(const Record& record)
    {
        if (!isOwnedBoxPayloadType(record.operands[0]))
            return fail(record, "result must be an owning value type without borrowed addresses");
        if (!isTypeOpcode(getValueTypeId(record.operands[2]), HGBCOpcode::BoxType))
            return fail(record, "operand must be a box");
        return true;
    }

    /// Requires opaque inputs; execution couples the selected case to the box's concrete type.
    bool validateMakeTaggedUnion(const Record& record)
    {
        if (!m_program.findTaggedUnionType(record.operands[0]))
            return fail(record, "result must be a tagged union type");
        if (!m_program.getIndex().uintTypeIds.contains(getValueTypeId(record.operands[2])))
            return fail(record, "case tag must be uint");
        if (!isTypeOpcode(getValueTypeId(record.operands[3]), HGBCOpcode::BoxType))
            return fail(record, "payload must be a box");
        return true;
    }

    bool validateGetTaggedUnionValue(const Record& record)
    {
        if (!m_program.findTaggedUnionType(getValueTypeId(record.operands[2])))
            return fail(record, "operand must be a tagged union value");
        if (record.opcode == HGBCOpcode::GetTaggedUnionCase)
        {
            if (!m_program.getIndex().uintTypeIds.contains(record.operands[0]))
                return fail(record, "case result must be uint");
        }
        else if (!isTypeOpcode(record.operands[0], HGBCOpcode::BoxType))
            return fail(record, "payload result must be a box");
        return true;
    }

    /// Checks the requested case without assuming that a preceding branch selected it.
    bool validateExtractTaggedUnion(const Record& record)
    {
        const auto* type = m_program.findTaggedUnionType(getValueTypeId(record.operands[2]));
        if (!type)
            return fail(record, "operand must be a tagged union value");
        auto caseIt = type->caseIndices.find(record.operands[3]);
        if (caseIt == type->caseIndices.end())
            return fail(record, "case tag is not declared by the union type");
        HGBCId payloadTypeId = type->cases[caseIt->second].payloadTypeId;
        if (record.operands[0] != payloadTypeId)
            return fail(record, "result does not match the case's concrete payload type");
        return true;
    }

    /// Requires a type-preserving case map. Execution rejects an unmapped active case when
    /// narrowing.
    bool validateConvertTaggedUnion(const Record& record)
    {
        const auto* destination = m_program.findTaggedUnionType(record.operands[0]);
        const auto* source = m_program.findTaggedUnionType(getValueTypeId(record.operands[2]));
        if (!source || !destination)
            return fail(record, "source and destination must be tagged union types");
        std::unordered_set<uint32_t> mappedTags;
        for (size_t i = 3; i < record.operands.size(); i += 2)
        {
            uint32_t sourceTag = record.operands[i];
            auto sourceIt = source->caseIndices.find(sourceTag);
            auto destinationIt = destination->caseIndices.find(record.operands[i + 1]);
            if (sourceIt == source->caseIndices.end() ||
                destinationIt == destination->caseIndices.end())
            {
                return fail(record, "case map references a tag not declared by its union type");
            }
            if (!mappedTags.insert(sourceTag).second)
                return fail(record, "case map contains a source tag more than once");
            if (source->cases[sourceIt->second].payloadTypeId !=
                destination->cases[destinationIt->second].payloadTypeId)
            {
                return fail(record, "case map must preserve each concrete payload type");
            }
        }
        return true;
    }

    bool validateAllocateObject(const Record& record)
    {
        if (!isTypeOpcode(record.operands[0], HGBCOpcode::ClassType))
            return fail(record, "result type is not a class");
        return true;
    }

    bool validateAllocateRuntimeArray(const Record& record)
    {
        if (!isTypeOpcode(record.operands[0], HGBCOpcode::RuntimeArrayType))
            return fail(record, "result type is not a runtime array");
        HGBCId countTypeId = getEffectiveValueTypeId(record.operands[2]);
        const auto& index = m_program.getIndex();
        if (!index.uintTypeIds.contains(countTypeId) && !index.intTypeIds.contains(countTypeId))
            return fail(record, "element count must be an int or uint");
        return true;
    }

    bool validateRuntimeArrayGetCount(const Record& record)
    {
        if (!m_program.getIndex().intTypeIds.contains(record.operands[0]))
            return fail(record, "result type must be int");
        if (!isTypeOpcode(record.operands[2], HGBCOpcode::RuntimeArrayType) ||
            !isValueCompatible(record.operands[2], record.operands[3]))
        {
            return fail(record, "array operand does not match the runtime array type");
        }
        return true;
    }

    HGBCId getArrayCopyElementTypeId(HGBCId endpointTypeId) const
    {
        const Record* endpointType = findTypeRecord(endpointTypeId);
        if (!endpointType || endpointType->operands.size() != 2)
            return 0;
        switch (endpointType->opcode)
        {
        case HGBCOpcode::RuntimeArrayType:
        case HGBCOpcode::StructuredBufferType:
        case HGBCOpcode::RWStructuredBufferType:
            return endpointType->operands[1];
        default:
            return 0;
        }
    }

    HGBCLayoutDomain getArrayCopyEndpointDomain(HGBCId endpointTypeId) const
    {
        return isTypeOpcode(endpointTypeId, HGBCOpcode::RuntimeArrayType)
                   ? HGBCLayoutDomain::Host
                   : HGBCLayoutDomain::DeviceMemory;
    }

    bool validateCopyArrayRange(const Record& record)
    {
        HGBCId destinationTypeId = record.operands[0];
        HGBCId sourceTypeId = record.operands[1];
        HGBCId destinationElementTypeId = getArrayCopyElementTypeId(destinationTypeId);
        HGBCId sourceElementTypeId = getArrayCopyElementTypeId(sourceTypeId);
        if (!destinationElementTypeId || !sourceElementTypeId ||
            !doesValueTypeMatch(destinationElementTypeId, sourceElementTypeId))
        {
            return fail(record, "copy endpoints must have the same supported element type");
        }
        if (!isValueCompatible(destinationTypeId, record.operands[3]) ||
            !isValueCompatible(sourceTypeId, record.operands[5]))
        {
            return fail(record, "copy values do not match their declared endpoint types");
        }

        const HGBC::BasicIndex& index = m_program.getIndex();
        if (!index.uintTypeIds.contains(getEffectiveValueTypeId(record.operands[4])) ||
            !index.uintTypeIds.contains(getEffectiveValueTypeId(record.operands[6])) ||
            !index.uintTypeIds.contains(getEffectiveValueTypeId(record.operands[7])))
        {
            return fail(record, "copy offsets and element count must be uint values");
        }

        const Record* converter = findLayoutConverterRecord(record.operands[2]);
        const Record* sourceLayout =
            converter ? findTypeLayoutRecord(converter->operands[1]) : nullptr;
        const Record* destinationLayout =
            converter ? findTypeLayoutRecord(converter->operands[2]) : nullptr;
        HGBCLayoutDomain sourceDomain = getArrayCopyEndpointDomain(sourceTypeId);
        HGBCLayoutDomain destinationDomain = getArrayCopyEndpointDomain(destinationTypeId);
        HGBCLayoutUsage sourceUsage = sourceDomain == HGBCLayoutDomain::Host
                                          ? HGBCLayoutUsage::Value
                                          : HGBCLayoutUsage::StructuredBufferElement;
        HGBCLayoutUsage destinationUsage = destinationDomain == HGBCLayoutDomain::Host
                                               ? HGBCLayoutUsage::Value
                                               : HGBCLayoutUsage::StructuredBufferElement;
        if (!sourceLayout || !destinationLayout ||
            !doesValueTypeMatch(sourceLayout->operands[1], sourceElementTypeId) ||
            !doesValueTypeMatch(destinationLayout->operands[1], destinationElementTypeId) ||
            HGBCLayoutDomain(sourceLayout->operands[2]) != sourceDomain ||
            HGBCLayoutDomain(destinationLayout->operands[2]) != destinationDomain ||
            HGBCLayoutUsage(sourceLayout->operands[3]) != sourceUsage ||
            HGBCLayoutUsage(destinationLayout->operands[3]) != destinationUsage)
        {
            return fail(record, "copy converter does not match its endpoint layouts");
        }

        if ((sourceDomain == HGBCLayoutDomain::DeviceMemory ||
             destinationDomain == HGBCLayoutDomain::DeviceMemory) &&
            !isByteSerializableType(sourceElementTypeId))
        {
            return fail(record, "device copy endpoints require byte-serializable elements");
        }
        if (sourceDomain == destinationDomain && converter->operands[1] != converter->operands[2])
            return fail(record, "same-space copies require identical physical layouts");
        return true;
    }

    bool validateElementAddress(const Record& record)
    {
        HGBCId baseTypeId = getAddressedTypeId(record.operands[2]);
        if (!baseTypeId)
        {
            baseTypeId = getEffectiveValueTypeId(record.operands[2]);
            const Record* runtimeArrayType = findTypeRecord(baseTypeId);
            if (!runtimeArrayType || runtimeArrayType->opcode != HGBCOpcode::RuntimeArrayType ||
                runtimeArrayType->operands.size() != 2)
            {
                return fail(record, "base is not an indexable storage reference");
            }

            HGBCId resultValueTypeId = getAddressValueTypeId(record.operands[0]);
            if (!resultValueTypeId ||
                !doesValueTypeMatch(resultValueTypeId, runtimeArrayType->operands[1]))
            {
                return fail(record, "result type does not match the runtime array element type");
            }

            HGBCId indexTypeId = getEffectiveValueTypeId(record.operands[3]);
            const auto& index = m_program.getIndex();
            if (!index.uintTypeIds.contains(indexTypeId) && !index.intTypeIds.contains(indexTypeId))
            {
                return fail(record, "runtime array index must be an int or uint");
            }
            return true;
        }

        const Record* baseType = findTypeRecord(baseTypeId);
        HGBCId resultValueTypeId = getAddressValueTypeId(record.operands[0]);
        if (!baseType || !resultValueTypeId)
            return fail(record, "base or result is not an indexable storage reference");

        bool resultMatches = false;
        switch (baseType->opcode)
        {
        case HGBCOpcode::ArrayType:
        case HGBCOpcode::VectorType:
            resultMatches = baseType->operands.size() == 3 &&
                            doesValueTypeMatch(resultValueTypeId, baseType->operands[2]);
            break;
        case HGBCOpcode::MatrixType:
            {
                NumericType baseNumericType;
                NumericType resultNumericType;
                resultMatches = tryGetNumericType(baseTypeId, baseNumericType) &&
                                tryGetNumericType(resultValueTypeId, resultNumericType) &&
                                resultNumericType.kind == NumericType::Kind::Vector &&
                                resultNumericType.columnCount == baseNumericType.columnCount &&
                                areScalarTypesCompatible(
                                    resultNumericType.scalarTypeId,
                                    baseNumericType.scalarTypeId);
            }
            break;
        default:
            return fail(record, "base is not an indexable storage reference");
        }
        if (!resultMatches)
            return fail(record, "result type does not match the indexed element type");
        if (!m_program.getIndex().uintTypeIds.contains(getEffectiveValueTypeId(record.operands[3])))
        {
            return fail(record, "index must be a uint");
        }
        return true;
    }

    bool validateElementExtract(const Record& record)
    {
        HGBCId baseTypeId = getValueTypeId(record.operands[2]);
        const Record* baseType = findTypeRecord(baseTypeId);
        if (!baseType)
            return fail(record, "base does not have an indexable type");

        bool resultMatches = false;
        switch (baseType->opcode)
        {
        case HGBCOpcode::ArrayType:
        case HGBCOpcode::VectorType:
            resultMatches = baseType->operands.size() == 3 &&
                            doesValueTypeMatch(record.operands[0], baseType->operands[2]);
            break;
        case HGBCOpcode::MatrixType:
            {
                NumericType baseNumericType;
                NumericType resultNumericType;
                resultMatches = tryGetNumericType(baseTypeId, baseNumericType) &&
                                tryGetNumericType(record.operands[0], resultNumericType) &&
                                resultNumericType.kind == NumericType::Kind::Vector &&
                                resultNumericType.columnCount == baseNumericType.columnCount &&
                                areScalarTypesCompatible(
                                    resultNumericType.scalarTypeId,
                                    baseNumericType.scalarTypeId);
            }
            break;
        default:
            return fail(record, "base does not have an indexable type");
        }
        if (!resultMatches)
            return fail(record, "result type does not match the indexed element type");
        if (!m_program.getIndex().uintTypeIds.contains(getValueTypeId(record.operands[3])))
        {
            return fail(record, "index must be a uint");
        }
        return true;
    }

    bool validateComparison(const Record& record)
    {
        NumericType resultType;
        NumericType operandType;
        if (!tryGetNumericType(record.operands[0], resultType) ||
            !isBoolScalarType(resultType.scalarTypeId))
        {
            return fail(record, "result must have a bool scalar, vector, or matrix type");
        }
        HGBCId operandTypeId = record.operands[2];
        if (!tryGetNumericType(operandTypeId, operandType) ||
            !haveSameNumericShape(resultType, operandType))
        {
            return fail(record, "result and operand shapes do not match");
        }
        bool isBool = isBoolScalarType(operandType.scalarTypeId);
        bool isInteger = isIntegerScalarType(operandType.scalarTypeId);
        bool isFloat = isFloatScalarType(operandType.scalarTypeId);
        if (!isBool && !isInteger && !isFloat)
            return fail(record, "operand type must have bool, integer, or float lanes");
        if ((record.opcode != HGBCOpcode::Eql && record.opcode != HGBCOpcode::Neq) && isBool)
            return fail(record, "ordered comparison operands must be numeric");
        if (!isValueCompatible(operandTypeId, record.operands[3]) ||
            !isValueCompatible(operandTypeId, record.operands[4]))
        {
            return fail(record, "values do not match the declared operand type");
        }
        return true;
    }

    // Ensures an arithmetic record's result and operation types agree and describe both inputs.
    // Consider `%sum : %uint = OpAdd %uint %left %right`: operand two identifies the operation
    // type just as it does for comparisons, while operand zero remains the generic result type.
    // `OpFDiv`, `OpPow`, and `OpAtan2` require a floating type, while `OpIDiv`, `OpIRem`, and
    // `OpLsh` require an integer type.
    bool validateArithmetic(const Record& record)
    {
        HGBCId resultTypeId = record.operands[0];
        HGBCId operandTypeId = record.operands[2];
        if (!doesValueTypeMatch(resultTypeId, operandTypeId))
            return fail(record, "result and operand types do not match");
        NumericType operandType;
        if (!tryGetNumericType(operandTypeId, operandType))
            return fail(record, "operand type is not a scalar, vector, or matrix");
        bool isInteger = isIntegerScalarType(operandType.scalarTypeId);
        bool isFloat = isFloatScalarType(operandType.scalarTypeId);
        bool supportsType = false;
        switch (record.opcode)
        {
        case HGBCOpcode::Add:
        case HGBCOpcode::Sub:
        case HGBCOpcode::Min:
        case HGBCOpcode::Max:
        case HGBCOpcode::Mul:
            supportsType = isInteger || isFloat;
            break;
        case HGBCOpcode::IDiv:
        case HGBCOpcode::IRem:
        case HGBCOpcode::Lsh:
            supportsType = isInteger;
            break;
        case HGBCOpcode::FDiv:
        case HGBCOpcode::Pow:
        case HGBCOpcode::Atan2:
            supportsType = isFloat;
            break;
        default:
            SLANG_UNREACHABLE("non-arithmetic opcode reached arithmetic validation");
        }
        if (!supportsType)
            return fail(record, "has an unsupported declared arithmetic type");
        if (!isValueCompatible(operandTypeId, record.operands[3]) ||
            !isValueCompatible(operandTypeId, record.operands[4]))
        {
            return fail(record, "values do not match the declared arithmetic type");
        }
        return true;
    }

    // Ensures unary operation legality before any interpreter stages or executes the record.
    // Consider `%inverse = OpNot %bool %condition`: the result and input must both be booleans.
    // Numeric negation and transcendental operations currently use the HGBC float type instead.
    bool validateUnary(const Record& record)
    {
        HGBCId typeId = record.operands[0];
        NumericType type;
        if (!tryGetNumericType(typeId, type))
            return fail(record, "result type is not a scalar, vector, or matrix");
        bool supportsType = false;
        switch (record.opcode)
        {
        case HGBCOpcode::Not:
            supportsType = isBoolScalarType(type.scalarTypeId);
            break;
        case HGBCOpcode::Neg:
            supportsType = m_program.getIndex().intTypeIds.contains(type.scalarTypeId) ||
                           isFloatScalarType(type.scalarTypeId);
            break;
        case HGBCOpcode::Sin:
        case HGBCOpcode::Cos:
        case HGBCOpcode::Tan:
        case HGBCOpcode::Asin:
        case HGBCOpcode::Sqrt:
        case HGBCOpcode::Ceil:
        case HGBCOpcode::Floor:
        case HGBCOpcode::Exp:
        case HGBCOpcode::Exp2:
        case HGBCOpcode::Log:
        case HGBCOpcode::Log2:
            supportsType = isFloatScalarType(type.scalarTypeId);
            break;
        default:
            SLANG_UNREACHABLE("non-unary opcode reached unary validation");
        }
        if (!supportsType)
            return fail(record, "has an unsupported declared unary type");
        if (!isValueCompatible(typeId, record.operands[2]))
            return fail(record, "value does not match the declared unary type");
        return true;
    }

    /// Validates a scalar Boolean reduction over one numeric aggregate.
    bool validateAny(const Record& record)
    {
        if (!m_program.getIndex().boolTypeIds.contains(record.operands[0]))
            return fail(record, "result must be a bool scalar");

        HGBCId operandTypeId = record.operands[2];
        NumericType operandType;
        if (!tryGetNumericType(operandTypeId, operandType))
            return fail(record, "operand must be a scalar, vector, or matrix");
        if (!isBoolScalarType(operandType.scalarTypeId) &&
            !isIntegerScalarType(operandType.scalarTypeId) &&
            !isFloatScalarType(operandType.scalarTypeId))
        {
            return fail(record, "operand must have bool, integer, or float lanes");
        }
        if (!isValueCompatible(operandTypeId, record.operands[3]))
            return fail(record, "value does not match the declared operand type");
        return true;
    }

    // Validates the common representation used by all casts between basic scalar types.
    //
    // Consider `float(i)` and `bool(u)`: both records explicitly identify their source and
    // destination scalar types, while the value must agree with the source. Adding another scalar
    // width only extends the type classification; it does not require another cast opcode.
    bool validateScalarTypeCast(const Record& record)
    {
        SLANG_RELEASE_ASSERT(record.operands.size() == 4);
        HGBCId sourceTypeId = record.operands[1];
        HGBCId destinationTypeId = record.operands[2];
        if (!isBasicScalarType(sourceTypeId) || !isBasicScalarType(destinationTypeId))
            return fail(record, "source and destination must be basic scalar types");
        if (!isValueCompatible(sourceTypeId, record.operands[3]))
            return fail(record, "value does not match the declared source type");
        return true;
    }

    bool validateFormatString(const Record& record)
    {
        if (!m_program.getIndex().stringTypeIds.contains(record.operands[0]) ||
            !isValueCompatible(record.operands[0], record.operands[2]))
        {
            return fail(record, "result and pattern must be strings");
        }
        for (size_t operandIndex = 3; operandIndex < record.operands.size(); operandIndex += 2)
        {
            HGBCId argumentTypeId = record.operands[operandIndex];
            if (!isPrintableScalarType(argumentTypeId) ||
                !isValueCompatible(argumentTypeId, record.operands[operandIndex + 1]))
            {
                return fail(record, "argument does not match a supported declared type");
            }
        }
        return true;
    }

    bool validateBufferReadback(const Record& record)
    {
        HGBCId bufferTypeId = record.operands[2];
        HGBCId elementTypeId = getBufferElementTypeId(bufferTypeId);
        if (!elementTypeId || !isValueCompatible(bufferTypeId, record.operands[4]))
            return fail(record, "buffer does not match its declared type");
        const Record* converter = findLayoutConverterRecord(record.operands[3]);
        const Record* source = converter ? findTypeLayoutRecord(converter->operands[1]) : nullptr;
        const Record* destination =
            converter ? findTypeLayoutRecord(converter->operands[2]) : nullptr;
        if (!source || !destination || !doesValueTypeMatch(source->operands[1], elementTypeId) ||
            HGBCLayoutDomain(source->operands[2]) != HGBCLayoutDomain::DeviceMemory ||
            HGBCLayoutDomain(destination->operands[2]) != HGBCLayoutDomain::Host)
        {
            return fail(record, "readback converter does not map the buffer element to host");
        }
        if (record.opcode == HGBCOpcode::ReadBackBufferElement)
        {
            if (!doesValueTypeMatch(elementTypeId, record.operands[0]))
                return fail(record, "result type does not match the buffer element type");
            return true;
        }
        const Record* resultType = findTypeRecord(record.operands[0]);
        if (!resultType || resultType->opcode != HGBCOpcode::DeferType ||
            resultType->operands.size() != 2 ||
            !doesValueTypeMatch(elementTypeId, resultType->operands[1]))
        {
            return fail(record, "result must defer the buffer element type");
        }
        return true;
    }

    bool validatePollInputEvent(const Record& record)
    {
        const HGBC::StructTypeDescriptor* type = m_program.findStructType(record.operands[0]);
        if (!type || type->fields.size() != 6)
            return fail(record, "result must have six fields");

        static const char* expectedNames[] = {"type", "key", "button", "pos", "scroll", "size"};
        for (size_t i = 0; i < 6; ++i)
        {
            const HGBC::StructFieldDescriptor& field = type->fields[i];
            if (field.name != expectedNames[i])
                return fail(record, "result field " + std::to_string(i) + " has the wrong name");
            bool validType = i < 3   ? m_program.getIndex().uintTypeIds.contains(field.typeId)
                             : i < 5 ? isFloat2Type(field.typeId)
                                     : isUInt2Type(field.typeId);
            if (!validType)
                return fail(record, "result field " + std::to_string(i) + " has the wrong type");
        }
        return true;
    }

    size_t getBlockParameterCount(const Block& block) const
    {
        size_t count = 0;
        for (size_t recordIndex : block.recordIndices)
        {
            if (m_decodedProgram.records[recordIndex].opcode == HGBCOpcode::Param)
                ++count;
        }
        return count;
    }

    // Validated HGBC blocks have one control transfer as their final executable instruction.
    // Consider this block:
    //
    // ```text
    // OpPrint %string %message
    // OpBranch %next
    // OpDebugLine ...
    // ```
    //
    // The trailing debug record is harmless, but another executable record after `OpBranch` would
    // be unreachable and a block without the branch would have no defined successor. This is an
    // HGBC control-flow invariant shared by every consumer, not a HGVM staging restriction.
    bool validateBlockTerminator(const Block& block)
    {
        const Record* terminator = nullptr;
        for (size_t recordIndex : block.recordIndices)
        {
            const Record& record = m_decodedProgram.records[recordIndex];
            if (hasHGBCOpcodeFlag(record.opcode, HGBCOpcodeFlags::Debug))
                continue;
            if (terminator)
                return fail(record, "appears after the block terminator");

            switch (record.opcode)
            {
            case HGBCOpcode::Return:
            case HGBCOpcode::Branch:
            case HGBCOpcode::BranchConditional:
                terminator = &record;
                break;
            default:
                break;
            }
        }

        if (!terminator)
        {
            return fail(
                m_decodedProgram.records[block.recordIndex],
                "defines a block without a control-flow terminator");
        }
        return true;
    }

    bool validateFunction(const Function& function, const Record& record)
    {
        const Record* functionType = findTypeRecord(function.typeId);
        if (!functionType || functionType->opcode != HGBCOpcode::FuncType)
            return fail(record, "does not have a function type");
        if (function.blocks.empty())
            return fail(record, "defines a function without a block");

        // The entry block's OpParam records are ordinary function parameters. A later OpParam is
        // an SSA phi. For example, `merge(%selected)` must reach HGBC as predecessor OpStore
        // records and an OpLoad in `merge`, so every HGBC consumer sees the same non-SSA CFG.
        for (size_t blockIndex = 0; blockIndex < function.blocks.size(); ++blockIndex)
        {
            const Block& block = function.blocks[blockIndex];
            if (!validateBlockTerminator(block))
                return false;
            if (blockIndex != 0)
            {
                for (size_t recordIndex : block.recordIndices)
                {
                    const Record& instruction = m_decodedProgram.records[recordIndex];
                    if (instruction.opcode == HGBCOpcode::Param)
                    {
                        return fail(instruction, "defines a phi parameter in non-SSA HGBC");
                    }
                }
            }
        }

        const Block& entryBlock = function.blocks.front();
        size_t expectedParameterCount = functionType->operands.size() - 2;
        if (getBlockParameterCount(entryBlock) != expectedParameterCount)
            return fail(record, "parameters do not match its function type");

        size_t parameterIndex = 0;
        for (size_t recordIndex : entryBlock.recordIndices)
        {
            const Record& parameter = m_decodedProgram.records[recordIndex];
            if (parameter.opcode != HGBCOpcode::Param)
                continue;
            if (!isValueCompatible(
                    functionType->operands[parameterIndex + 2],
                    parameter.operands[1]))
            {
                return fail(parameter, "type does not match the function parameter type");
            }
            ++parameterIndex;
        }
        return true;
    }

    bool validateBranch(const Function& function, const Record& record, HGBCId targetBlockId)
    {
        auto blockIt = function.blockIndices.find(targetBlockId);
        if (blockIt == function.blockIndices.end())
            return fail(record, "references a missing block");

        const Block& target = function.blocks[blockIt->second];
        if (getBlockParameterCount(target) != 0)
            return fail(record, "targets the parameterized function entry block");
        return true;
    }

    bool validateReturn(const Function& function, const Record& record)
    {
        const Record* functionType = findTypeRecord(function.typeId);
        SLANG_RELEASE_ASSERT(
            functionType && functionType->opcode == HGBCOpcode::FuncType &&
            functionType->operands.size() >= 2);

        HGBCId resultTypeId = functionType->operands[1];
        bool isVoid = isTypeOpcode(resultTypeId, HGBCOpcode::VoidType);
        if (record.operands.size() != size_t(!isVoid))
            return fail(record, "value does not match the function result type");
        if (!isVoid && !isValueCompatible(resultTypeId, record.operands[0]))
            return fail(record, "value type does not match the function result type");
        return true;
    }

    bool validateInstruction(const Function& function, const Record& record)
    {
        if (hasHGBCOpcodeFlag(record.opcode, HGBCOpcodeFlags::Debug))
            return true;
        switch (record.opcode)
        {
        case HGBCOpcode::Param:
        case HGBCOpcode::UploadBufferUploadTo:
        case HGBCOpcode::TextureGetDimensions:
        case HGBCOpcode::ResetTimer:
            return true;

        case HGBCOpcode::CloseWindow:
            if (!isTypeOpcode(getEffectiveValueTypeId(record.operands[0]), HGBCOpcode::WindowType))
            {
                return fail(record, "requires a Window operand");
            }
            return true;
        case HGBCOpcode::SetWindowTitle:
            if (!isTypeOpcode(
                    getEffectiveValueTypeId(record.operands[0]),
                    HGBCOpcode::WindowType) ||
                !m_program.getIndex().stringTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[1])))
            {
                return fail(record, "requires Window and String operands");
            }
            return true;
        case HGBCOpcode::PresentSurface:
            if (!isTypeOpcode(getEffectiveValueTypeId(record.operands[0]), HGBCOpcode::SurfaceType))
            {
                return fail(record, "requires a Surface operand");
            }
            return true;

        case HGBCOpcode::ByteStreamFromFile:
            if (!isTypeOpcode(record.operands[0], HGBCOpcode::ByteStreamType) ||
                !m_program.getIndex().stringTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[3])))
            {
                return fail(record, "requires a byte-stream result and String path");
            }
            return true;
        case HGBCOpcode::CloseByteStream:
            if (!isTypeOpcode(
                    getEffectiveValueTypeId(record.operands[0]),
                    HGBCOpcode::ByteStreamType))
            {
                return fail(record, "requires a byte-stream operand");
            }
            return true;
        case HGBCOpcode::ByteStreamGetSize:
            if (!m_program.getIndex().uintTypeIds.contains(record.operands[0]) ||
                !isTypeOpcode(
                    getEffectiveValueTypeId(record.operands[2]),
                    HGBCOpcode::ByteStreamType))
            {
                return fail(record, "requires a uint result and byte-stream operand");
            }
            return true;

        case HGBCOpcode::StringGetByteCount:
            if (!m_program.getIndex().uintTypeIds.contains(record.operands[0]) ||
                !m_program.getIndex().stringTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[2])))
            {
                return fail(record, "requires a uint result and String operand");
            }
            return true;
        case HGBCOpcode::StringReadByte:
            if (!m_program.getIndex().uintTypeIds.contains(record.operands[0]) ||
                !m_program.getIndex().stringTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[2])) ||
                !m_program.getIndex().uintTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[3])))
            {
                return fail(record, "requires a uint result, String value, and uint index");
            }
            return true;
        case HGBCOpcode::Fail:
            if (!m_program.getIndex().stringTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[0])))
            {
                return fail(record, "message must be a String");
            }
            return true;

        case HGBCOpcode::AllocateResource:
            return validateAllocateResource(record);
        case HGBCOpcode::AllocateObject:
            return validateAllocateObject(record);
        case HGBCOpcode::Box:
            return validateBox(record);
        case HGBCOpcode::Unbox:
            return validateUnbox(record);
        case HGBCOpcode::MakeTaggedUnion:
            return validateMakeTaggedUnion(record);
        case HGBCOpcode::GetTaggedUnionCase:
        case HGBCOpcode::GetTaggedUnionPayload:
            return validateGetTaggedUnionValue(record);
        case HGBCOpcode::ExtractTaggedUnion:
            return validateExtractTaggedUnion(record);
        case HGBCOpcode::ConvertTaggedUnion:
            return validateConvertTaggedUnion(record);
        case HGBCOpcode::AllocateRuntimeArray:
            return validateAllocateRuntimeArray(record);
        case HGBCOpcode::RuntimeArrayGetCount:
            return validateRuntimeArrayGetCount(record);
        case HGBCOpcode::CopyArrayRange:
            return validateCopyArrayRange(record);
        case HGBCOpcode::ResourceToDescriptorHandle:
            {
                const Record* handleType = findTypeRecord(record.operands[0]);
                if (!handleType || handleType->opcode != HGBCOpcode::DescriptorHandleType ||
                    handleType->operands.size() != 2 ||
                    !isValueCompatible(handleType->operands[1], record.operands[2]))
                {
                    return fail(record, "result handle and resource operand types do not match");
                }
                return true;
            }
        case HGBCOpcode::DescriptorHandleToResource:
            {
                HGBCId handleTypeId = getEffectiveValueTypeId(record.operands[2]);
                const Record* handleType = findTypeRecord(handleTypeId);
                if (!handleType || handleType->opcode != HGBCOpcode::DescriptorHandleType ||
                    handleType->operands.size() != 2 ||
                    handleType->operands[1] != record.operands[0])
                {
                    return fail(record, "resource result and handle operand types do not match");
                }
                return true;
            }

        case HGBCOpcode::Var:
            return validateVar(record);
        case HGBCOpcode::Load:
            return validateLoad(record);
        case HGBCOpcode::Store:
            return validateStore(record);
        case HGBCOpcode::FieldAddress:
            return validateFieldAddress(record);
        case HGBCOpcode::FieldExtract:
            return validateFieldExtract(record);

        case HGBCOpcode::ElementAddress:
            return validateElementAddress(record);
        case HGBCOpcode::ExtractElement:
            return validateElementExtract(record);

        case HGBCOpcode::ImageDecode:
        case HGBCOpcode::ImageEncode:
            return validateImageOperation(record);

        case HGBCOpcode::Eql:
        case HGBCOpcode::Neq:
        case HGBCOpcode::Less:
        case HGBCOpcode::Greater:
        case HGBCOpcode::Leq:
        case HGBCOpcode::Geq:
            return validateComparison(record);
        case HGBCOpcode::Add:
        case HGBCOpcode::Sub:
        case HGBCOpcode::Min:
        case HGBCOpcode::Max:
        case HGBCOpcode::Mul:
        case HGBCOpcode::IDiv:
        case HGBCOpcode::IRem:
        case HGBCOpcode::Lsh:
        case HGBCOpcode::FDiv:
        case HGBCOpcode::Pow:
        case HGBCOpcode::Atan2:
            return validateArithmetic(record);
        case HGBCOpcode::Neg:
        case HGBCOpcode::Not:
        case HGBCOpcode::Sin:
        case HGBCOpcode::Cos:
        case HGBCOpcode::Tan:
        case HGBCOpcode::Asin:
        case HGBCOpcode::Sqrt:
        case HGBCOpcode::Ceil:
        case HGBCOpcode::Floor:
        case HGBCOpcode::Exp:
        case HGBCOpcode::Exp2:
        case HGBCOpcode::Log:
        case HGBCOpcode::Log2:
            return validateUnary(record);
        case HGBCOpcode::Any:
            return validateAny(record);
        case HGBCOpcode::ScalarTypeCast:
            return validateScalarTypeCast(record);
        case HGBCOpcode::MakeVector:
            return validateMakeVector(record);
        case HGBCOpcode::MakeArray:
            return validateMakeArray(record);
        case HGBCOpcode::StructuredBufferGetDimensions:
            if (!isUInt2Type(record.operands[0]) || !isBufferType(record.operands[2]) ||
                !isValueCompatible(record.operands[2], record.operands[3]))
            {
                return fail(record, "types do not match the buffer dimension operation");
            }
            return true;
        case HGBCOpcode::MakeGraphicsDraw:
            if (!isTypeOpcode(record.operands[0], HGBCOpcode::GraphicsDrawType))
                return fail(record, "must produce graphics draw arguments");
            for (size_t i = 2; i < 6; ++i)
                if (!isTypeOpcode(
                        getEffectiveValueTypeId(record.operands[i]),
                        HGBCOpcode::UIntType))
                    return fail(record, "draw counts and offsets must be uint32 values");
            // HGBC integer literals carry a 32-bit pattern; the consuming operation determines
            // signedness. Base vertices interpret that pattern as int32, including literal zero.
            if (!isIntegerScalarType(getEffectiveValueTypeId(record.operands[6])))
                return fail(record, "base vertex must be a 32-bit integer value");
            if (record.operands.size() == 8)
            {
                HGBCId indices = getEffectiveValueTypeId(record.operands[7]);
                if (!isBufferType(indices) ||
                    !isTypeOpcode(getBufferElementTypeId(indices), HGBCOpcode::UIntType))
                    return fail(record, "index buffer must contain uint32 indices");
            }
            return true;
        case HGBCOpcode::MakeGraphicsDrawParameters:
            if (!isTypeOpcode(record.operands[0], HGBCOpcode::GraphicsDrawParametersType))
                return fail(record, "must produce graphics draw parameters");
            for (size_t i = 2; i < record.operands.size(); ++i)
            {
                HGBCOpcode expected = i < 4    ? HGBCOpcode::BoolType
                                      : i < 10 ? HGBCOpcode::Float32Type
                                               : HGBCOpcode::UIntType;
                if (!isTypeOpcode(getEffectiveValueTypeId(record.operands[i]), expected))
                    return fail(record, "dynamic draw parameter has an incorrect scalar type");
            }
            return true;
        case HGBCOpcode::CreateRaytracingScene:
            return validateRaytracingScene(record);
        case HGBCOpcode::AllocAccelerationStructure:
            if (!isTypeOpcode(
                    record.operands[0],
                    HGBCAccelerationStructureKind(record.operands[2]) ==
                            HGBCAccelerationStructureKind::BottomLevel
                        ? HGBCOpcode::RaytracingBottomLevelAccelerationStructureType
                        : HGBCOpcode::RaytracingAccelerationStructureType))
                return fail(
                    record,
                    "allocation result does not match the acceleration-structure kind");
            return true;
        case HGBCOpcode::BuildAccelerationStructure:
            return validateAccelerationStructureBuild(record);
        case HGBCOpcode::MakeRaytracingPipeline:
            if (!isTypeOpcode(record.operands[0], HGBCOpcode::RaytracingPipelineType))
                return fail(record, "must produce a ray-tracing pipeline");
            if (!isTypeOpcode(getValueTypeId(record.operands[2]), HGBCOpcode::UIntType))
                return fail(record, "ray-type count must be a uint32 value");
            return true;
        case HGBCOpcode::SetRayMiss:
        case HGBCOpcode::SetRayHitGroup:
            return validateRayRegistration(record);
        case HGBCOpcode::DispatchRays:
            return validateRayDispatch(record);
        case HGBCOpcode::ReadBackBufferElement:
        case HGBCOpcode::AsyncReadBackBufferElement:
            return validateBufferReadback(record);
        case HGBCOpcode::WaitDefer:
            {
                HGBCId deferTypeId = getEffectiveValueTypeId(record.operands[2]);
                const Record* deferType = findTypeRecord(deferTypeId);
                if (!deferType || deferType->opcode != HGBCOpcode::DeferType ||
                    deferType->operands.size() != 2 ||
                    !doesValueTypeMatch(record.operands[0], deferType->operands[1]))
                {
                    return fail(record, "deferred value does not match the result type");
                }
                return true;
            }
        case HGBCOpcode::ByteStreamRead:
        case HGBCOpcode::ByteStreamWrite:
            return validateByteStreamTransfer(record);
        case HGBCOpcode::UploadBufferStore:
            if (!isTypeOpcode(
                    getEffectiveValueTypeId(record.operands[1]),
                    HGBCOpcode::UploadBufferStorageType) ||
                !isValueCompatible(record.operands[0], record.operands[3]))
            {
                return fail(record, "types do not match the upload-buffer store operation");
            }
            return true;
        case HGBCOpcode::FormatString:
            return validateFormatString(record);
        case HGBCOpcode::Print:
            if (!isPrintableScalarType(record.operands[0]) ||
                !isValueCompatible(record.operands[0], record.operands[1]))
            {
                return fail(record, "value does not match a printable declared type");
            }
            return true;

        case HGBCOpcode::Return:
            return validateReturn(function, record);
        case HGBCOpcode::Branch:
            return validateBranch(function, record, record.operands[0]);
        case HGBCOpcode::BranchConditional:
            if (!m_program.getIndex().boolTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[0])))
            {
                return fail(record, "condition must be a bool");
            }
            return validateBranch(function, record, record.operands[1]) &&
                   validateBranch(function, record, record.operands[2]);
        case HGBCOpcode::Call:
            return validateCall(record);
        case HGBCOpcode::MakeStruct:
            return validateMakeStruct(record);
        case HGBCOpcode::ClearTexture:
            return validateTextureClear(record);
        case HGBCOpcode::BlitTexture2D:
            if (!isTextureType(record.operands[0]) || !isTextureType(record.operands[1]) ||
                !isValueCompatible(record.operands[0], record.operands[2]) ||
                !isValueCompatible(record.operands[1], record.operands[3]))
            {
                return fail(record, "declared texture types do not match the blit operands");
            }
            return true;
        case HGBCOpcode::CreateWindow:
            if (!isTypeOpcode(record.operands[0], HGBCOpcode::WindowType) ||
                !m_program.getIndex().stringTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[2])) ||
                !m_program.getIndex().uintTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[3])) ||
                !m_program.getIndex().uintTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[4])))
            {
                return fail(record, "requires a Window result, String title, and uint dimensions");
            }
            return true;
        case HGBCOpcode::ProcessWindowEvents:
            if (!m_program.getIndex().boolTypeIds.contains(record.operands[0]) ||
                !isTypeOpcode(getEffectiveValueTypeId(record.operands[2]), HGBCOpcode::WindowType))
            {
                return fail(record, "requires a bool result and Window operand");
            }
            return true;
        case HGBCOpcode::PollInputEvent:
            return validatePollInputEvent(record);
        case HGBCOpcode::CreateSurface:
            if (!isTypeOpcode(record.operands[0], HGBCOpcode::SurfaceType) ||
                !isTypeOpcode(
                    getEffectiveValueTypeId(record.operands[2]),
                    HGBCOpcode::WindowType) ||
                !m_program.getIndex().uintTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[3])))
            {
                return fail(
                    record,
                    "requires a Surface result, Window operand, and uint image count");
            }
            return true;
        case HGBCOpcode::AcquireSurfaceTexture:
            return validateAcquiredSurfaceTexture(record, record.operands[0], record.operands[2]);
        case HGBCOpcode::TryAcquireSurfaceTexture:
            if (!m_program.getIndex().boolTypeIds.contains(record.operands[0]))
                return fail(record, "requires a bool result");
            return validateAcquiredSurfaceTexture(
                record,
                getAddressedTypeId(record.operands[3]),
                record.operands[2]);
        case HGBCOpcode::GetSurfaceDimensions:
            if (!isUInt2Type(record.operands[0]) ||
                !isTypeOpcode(getEffectiveValueTypeId(record.operands[2]), HGBCOpcode::SurfaceType))
                return fail(record, "requires a uint2 result and Surface operand");
            return true;
        case HGBCOpcode::WaitForSurface:
        case HGBCOpcode::TryPresentSurface:
            if (!m_program.getIndex().boolTypeIds.contains(record.operands[0]) ||
                !isTypeOpcode(getEffectiveValueTypeId(record.operands[2]), HGBCOpcode::SurfaceType))
                return fail(record, "requires a bool result and Surface operand");
            return true;
        case HGBCOpcode::ResizeSurface:
            if (!m_program.getIndex().boolTypeIds.contains(record.operands[0]) ||
                !isTypeOpcode(
                    getEffectiveValueTypeId(record.operands[2]),
                    HGBCOpcode::SurfaceType) ||
                !m_program.getIndex().uintTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[3])) ||
                !m_program.getIndex().uintTypeIds.contains(
                    getEffectiveValueTypeId(record.operands[4])))
                return fail(record, "requires a bool result, Surface operand, and uint dimensions");
            return true;
        case HGBCOpcode::CreateTimer:
            if (!isTypeOpcode(record.operands[0], HGBCOpcode::TimerType))
                return fail(record, "result must be a timer");
            return true;
        case HGBCOpcode::TimerElapsed:
            if (!isTypeOpcode(record.operands[0], HGBCOpcode::Float32Type))
                return fail(record, "result must be a float");
            return true;
        case HGBCOpcode::MakeMatrix:
            return validateMakeMatrix(record);
        default:
            return fail(record, "is not supported by the interpreter");
        }
    }
};

} // namespace

SlangResult validateProgram(const Program& program, String& outError)
{
    return ProgramValidator(program).validate(outError);
}

} // namespace Slang::HGVM
