// slang-emit-hgbc.cpp

#include "slang-emit-hgbc.h"

#include "../compiler-core/slang-artifact-util.h"
#include "../compiler-core/slang-diagnostic-sink.h"
#include "../core/slang-blob.h"
#include "../core/slang-dictionary.h"
#include "../core/slang-io.h"
#include "core/slang-math.h"
#include "slang-code-gen.h"
#include "slang-hgbc.h"
#include "slang-ir-hgbc-owned-erasure.h"
#include "slang-ir-layout.h"
#include "slang-ir-link.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir-util.h"
#include "slang-module.h"
#include "slang-rich-diagnostics.h"
#include "slang-serialize-container.h"

#include <limits>

namespace Slang
{
namespace
{

// HGBC resource-format operands deliberately use the compiler's ImageFormat numbering. These
// checks keep the independently generated wire spelling table from drifting from the shared list.
static_assert(uint32_t(ImageFormat::unknown) == uint32_t(HGBCResourceFormat::Undefined));
static_assert(uint32_t(ImageFormat::rgba32f) == uint32_t(HGBCResourceFormat::RGBA32Float));
static_assert(uint32_t(ImageFormat::rgba16f) == uint32_t(HGBCResourceFormat::RGBA16Float));
static_assert(uint32_t(ImageFormat::rg32f) == uint32_t(HGBCResourceFormat::RG32Float));
static_assert(uint32_t(ImageFormat::rg16f) == uint32_t(HGBCResourceFormat::RG16Float));
static_assert(
    uint32_t(ImageFormat::r11f_g11f_b10f) == uint32_t(HGBCResourceFormat::R11G11B10Float));
static_assert(uint32_t(ImageFormat::r32f) == uint32_t(HGBCResourceFormat::R32Float));
static_assert(uint32_t(ImageFormat::r16f) == uint32_t(HGBCResourceFormat::R16Float));
static_assert(uint32_t(ImageFormat::rgba16) == uint32_t(HGBCResourceFormat::RGBA16Unorm));
static_assert(uint32_t(ImageFormat::rgb10_a2) == uint32_t(HGBCResourceFormat::RGB10A2Unorm));
static_assert(uint32_t(ImageFormat::rgba8) == uint32_t(HGBCResourceFormat::RGBA8Unorm));
static_assert(uint32_t(ImageFormat::rg16) == uint32_t(HGBCResourceFormat::RG16Unorm));
static_assert(uint32_t(ImageFormat::rg8) == uint32_t(HGBCResourceFormat::RG8Unorm));
static_assert(uint32_t(ImageFormat::r16) == uint32_t(HGBCResourceFormat::R16Unorm));
static_assert(uint32_t(ImageFormat::r8) == uint32_t(HGBCResourceFormat::R8Unorm));
static_assert(uint32_t(ImageFormat::rgba16_snorm) == uint32_t(HGBCResourceFormat::RGBA16Snorm));
static_assert(uint32_t(ImageFormat::rgba8_snorm) == uint32_t(HGBCResourceFormat::RGBA8Snorm));
static_assert(uint32_t(ImageFormat::rg16_snorm) == uint32_t(HGBCResourceFormat::RG16Snorm));
static_assert(uint32_t(ImageFormat::rg8_snorm) == uint32_t(HGBCResourceFormat::RG8Snorm));
static_assert(uint32_t(ImageFormat::r16_snorm) == uint32_t(HGBCResourceFormat::R16Snorm));
static_assert(uint32_t(ImageFormat::r8_snorm) == uint32_t(HGBCResourceFormat::R8Snorm));
static_assert(uint32_t(ImageFormat::rgba32i) == uint32_t(HGBCResourceFormat::RGBA32Sint));
static_assert(uint32_t(ImageFormat::rgba16i) == uint32_t(HGBCResourceFormat::RGBA16Sint));
static_assert(uint32_t(ImageFormat::rgba8i) == uint32_t(HGBCResourceFormat::RGBA8Sint));
static_assert(uint32_t(ImageFormat::rg32i) == uint32_t(HGBCResourceFormat::RG32Sint));
static_assert(uint32_t(ImageFormat::rg16i) == uint32_t(HGBCResourceFormat::RG16Sint));
static_assert(uint32_t(ImageFormat::rg8i) == uint32_t(HGBCResourceFormat::RG8Sint));
static_assert(uint32_t(ImageFormat::r32i) == uint32_t(HGBCResourceFormat::R32Sint));
static_assert(uint32_t(ImageFormat::r16i) == uint32_t(HGBCResourceFormat::R16Sint));
static_assert(uint32_t(ImageFormat::r8i) == uint32_t(HGBCResourceFormat::R8Sint));
static_assert(uint32_t(ImageFormat::rgba32ui) == uint32_t(HGBCResourceFormat::RGBA32Uint));
static_assert(uint32_t(ImageFormat::rgba16ui) == uint32_t(HGBCResourceFormat::RGBA16Uint));
static_assert(uint32_t(ImageFormat::rgb10_a2ui) == uint32_t(HGBCResourceFormat::RGB10A2Uint));
static_assert(uint32_t(ImageFormat::rgba8ui) == uint32_t(HGBCResourceFormat::RGBA8Uint));
static_assert(uint32_t(ImageFormat::rg32ui) == uint32_t(HGBCResourceFormat::RG32Uint));
static_assert(uint32_t(ImageFormat::rg16ui) == uint32_t(HGBCResourceFormat::RG16Uint));
static_assert(uint32_t(ImageFormat::rg8ui) == uint32_t(HGBCResourceFormat::RG8Uint));
static_assert(uint32_t(ImageFormat::r32ui) == uint32_t(HGBCResourceFormat::R32Uint));
static_assert(uint32_t(ImageFormat::r16ui) == uint32_t(HGBCResourceFormat::R16Uint));
static_assert(uint32_t(ImageFormat::r8ui) == uint32_t(HGBCResourceFormat::R8Uint));
static_assert(uint32_t(ImageFormat::r64ui) == uint32_t(HGBCResourceFormat::R64Uint));
static_assert(uint32_t(ImageFormat::r64i) == uint32_t(HGBCResourceFormat::R64Sint));
static_assert(uint32_t(ImageFormat::bgra8) == uint32_t(HGBCResourceFormat::BGRA8Unorm));
static_assert(uint32_t(ImageFormat::rgba8_srgb) == uint32_t(HGBCResourceFormat::RGBA8UnormSrgb));
static_assert(uint32_t(ImageFormat::d32f) == uint32_t(HGBCResourceFormat::D32Float));
static_assert(uint32_t(ImageFormat::d16) == uint32_t(HGBCResourceFormat::D16Unorm));
static_assert(uint32_t(ImageFormat::d32f_s8) == uint32_t(HGBCResourceFormat::D32FloatS8Uint));

void diagnoseRaw(DiagnosticSink* sink, const String& message)
{
    if (sink)
        sink->diagnoseRaw(Severity::Error, message.getUnownedSlice());
}

struct HGBCBuilder
{
    HGBC::Program program;

    HGBCId nextId = 1;
    Dictionary<String, HGBCId> stringLiteralIds;
    Dictionary<String, HGBCId> intLiteralIds;
    Dictionary<String, HGBCId> floatLiteralIds;
    Dictionary<HGBCId, HGBCId> nullLiteralIds;
    Dictionary<HGBCId, bool> emittedTypeSizes;
    Dictionary<String, HGBCId> typeLayoutIds;
    Dictionary<String, HGBCId> layoutConverterIds;
    Dictionary<String, HGBCId> rayHitGroupIds;
    HGBCId voidTypeId = 0;
    HGBCId uintTypeId = 0;
    HGBCId intTypeId = 0;
    HGBCId boolTypeId = 0;
    HGBCId float16TypeId = 0;
    HGBCId float32TypeId = 0;
    HGBCId float64TypeId = 0;
    HGBCId stringTypeId = 0;
    HGBCId falseLiteralId = 0;
    HGBCId trueLiteralId = 0;
    Dictionary<uint32_t, HGBCId> opaqueTypeIds;
    Dictionary<String, HGBCId> structuredBufferTypeIds;
    Dictionary<String, HGBCId> rwStructuredBufferTypeIds;
    Dictionary<IRDescriptorHandleType*, HGBCId> descriptorHandleTypeIds;
    Dictionary<HGBCId, HGBCId> structuredBufferElementTypeIds;
    Dictionary<HGBCId, HGBCId> runtimeArrayElementTypeIds;
    Dictionary<String, HGBCId> texture2DTypeIds;
    Dictionary<String, HGBCId> renderTarget2DTypeIds;
    Dictionary<uint32_t, HGBCId> surfaceTypeIds;
    Dictionary<IRStructType*, HGBCId> structTypeIds;
    Dictionary<IRClassType*, HGBCId> classTypeIds;
    Dictionary<HGBCId, bool> aggregateTypeIds;
    Dictionary<HGBCId, bool> slangTypeNameIds;
    Dictionary<IRHGVMRuntimeArrayType*, HGBCId> runtimeArrayTypeIds;
    Dictionary<IRType*, HGBCId> taggedUnionTypeIds;
    Dictionary<String, HGBCId> vectorTypeIds;
    Dictionary<String, HGBCId> matrixTypeIds;
    Dictionary<String, HGBCId> arrayTypeIds;
    Dictionary<HGBCId, HGBCId> deferTypeIds;
    Dictionary<uint64_t, HGBCId> addressTypeIds;
    Dictionary<String, HGBCId> funcTypeIds;
    Dictionary<String, HGBCId> computeDispatcherIds;
    Dictionary<String, HGBCId> graphicsPipelineIds;
    Dictionary<String, HGBCId> rayTracerIds;
    HGBCBuilder()
    {
        for (auto section : kHGBCRecordSections)
            addSection(section);
    }

    HGBCId allocId() { return nextId++; }

    void addSection(HGBCSectionTag tag)
    {
        HGBC::Section section;
        section.tag = tag;
        program.sections.add(std::move(section));
    }

    void emitInst(
        HGBCOpcode opcode,
        const uint32_t* operands,
        Index operandCount,
        const void* payload = nullptr,
        Index payloadSize = 0)
    {
        HGBC::addRecord(program, opcode, operands, operandCount, payload, payloadSize);
    }

    HGBCId getStringLiteralId(const UnownedStringSlice& slice)
    {
        String text(slice);
        if (auto existingId = stringLiteralIds.tryGetValue(text))
            return *existingId;

        HGBCId id = allocId();
        stringLiteralIds.add(text, id);

        uint32_t operands[] = {getStringTypeId(), id, uint32_t(slice.getLength())};
        emitInst(
            HGBCOpcode::StringLit,
            operands,
            SLANG_COUNT_OF(operands),
            slice.begin(),
            slice.getLength());
        return id;
    }

    HGBCId emitBinaryLiteral(const void* data, Index size)
    {
        HGBCId id = allocId();
        uint32_t operands[] = {id, uint32_t(size)};
        emitInst(HGBCOpcode::BinaryLit, operands, SLANG_COUNT_OF(operands), data, size);
        return id;
    }

    HGBCId getIntLiteralId(HGBCId typeId, uint32_t value)
    {
        StringBuilder keyBuilder;
        keyBuilder << typeId;
        keyBuilder << ":";
        keyBuilder << value;
        String key = keyBuilder.produceString();

        if (auto existingId = intLiteralIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        intLiteralIds.add(key, id);

        uint32_t operands[] = {typeId, id, value};
        emitInst(HGBCOpcode::IntLit, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getBoolLiteralId(bool value)
    {
        HGBCId& cachedId = value ? trueLiteralId : falseLiteralId;
        if (cachedId)
            return cachedId;

        cachedId = allocId();
        uint32_t operands[] = {getBoolTypeId(), cachedId, value ? 1u : 0u};
        emitInst(HGBCOpcode::BoolLit, operands, SLANG_COUNT_OF(operands));
        return cachedId;
    }

    HGBCId getFloatLiteralId(float value)
    {
        uint32_t bits = bitCast<uint32_t>(value);

        StringBuilder keyBuilder;
        keyBuilder << "f32:";
        keyBuilder << bits;
        String key = keyBuilder.produceString();
        if (auto existingId = floatLiteralIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        floatLiteralIds.add(key, id);

        uint32_t operands[] = {getFloat32TypeId(), id, bits};
        emitInst(HGBCOpcode::FloatLit, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    /// Emits a binary64 literal as low and high words without rounding through binary32.
    HGBCId getFloat64LiteralId(double value)
    {
        uint64_t bits = bitCast<uint64_t>(value);
        StringBuilder keyBuilder;
        keyBuilder << "f64:" << bits;
        String key = keyBuilder.produceString();
        if (auto existingId = floatLiteralIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        floatLiteralIds.add(key, id);
        uint32_t operands[] = {getFloat64TypeId(), id, uint32_t(bits), uint32_t(bits >> 32)};
        emitInst(HGBCOpcode::FloatLit, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getFloat16LiteralId(float value)
    {
        uint32_t bits = uint32_t(FloatToHalf(value));

        StringBuilder keyBuilder;
        keyBuilder << "f16:";
        keyBuilder << bits;
        String key = keyBuilder.produceString();
        if (auto existingId = floatLiteralIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        floatLiteralIds.add(key, id);

        uint32_t operands[] = {getFloat16TypeId(), id, bits};
        emitInst(HGBCOpcode::FloatLit, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getVoidTypeId()
    {
        if (voidTypeId)
            return voidTypeId;

        voidTypeId = allocId();
        uint32_t operands[] = {voidTypeId};
        emitInst(HGBCOpcode::VoidType, operands, SLANG_COUNT_OF(operands));
        return voidTypeId;
    }

    HGBCId getUIntTypeId()
    {
        if (uintTypeId)
            return uintTypeId;

        uintTypeId = allocId();
        uint32_t operands[] = {uintTypeId};
        emitInst(HGBCOpcode::UIntType, operands, SLANG_COUNT_OF(operands));
        return uintTypeId;
    }

    HGBCId getIntTypeId()
    {
        if (intTypeId)
            return intTypeId;

        intTypeId = allocId();
        uint32_t operands[] = {intTypeId};
        emitInst(HGBCOpcode::IntType, operands, SLANG_COUNT_OF(operands));
        return intTypeId;
    }

    HGBCId getBoolTypeId()
    {
        if (boolTypeId)
            return boolTypeId;

        boolTypeId = allocId();
        uint32_t operands[] = {boolTypeId};
        emitInst(HGBCOpcode::BoolType, operands, SLANG_COUNT_OF(operands));
        return boolTypeId;
    }

    HGBCId getFloat32TypeId()
    {
        if (float32TypeId)
            return float32TypeId;

        float32TypeId = allocId();
        uint32_t operands[] = {float32TypeId};
        emitInst(HGBCOpcode::Float32Type, operands, SLANG_COUNT_OF(operands));
        return float32TypeId;
    }

    /// Emits and caches the binary64 scalar type.
    HGBCId getFloat64TypeId()
    {
        if (float64TypeId)
            return float64TypeId;

        float64TypeId = allocId();
        uint32_t operands[] = {float64TypeId};
        emitInst(HGBCOpcode::Float64Type, operands, SLANG_COUNT_OF(operands));
        return float64TypeId;
    }

    HGBCId getFloat16TypeId()
    {
        if (float16TypeId)
            return float16TypeId;

        float16TypeId = allocId();
        uint32_t operands[] = {float16TypeId};
        emitInst(HGBCOpcode::Float16Type, operands, SLANG_COUNT_OF(operands));
        return float16TypeId;
    }

    /// Emits and caches an opaque type selected by its schema opcode.
    HGBCId getOpaqueTypeId(HGBCOpcode opcode)
    {
        SLANG_ASSERT(hasHGBCOpcodeFlag(opcode, HGBCOpcodeFlags::OpaqueType));
        uint32_t opcodeValue = uint32_t(opcode);
        if (auto existingId = opaqueTypeIds.tryGetValue(opcodeValue))
            return *existingId;

        HGBCId id = allocId();
        opaqueTypeIds.add(opcodeValue, id);
        uint32_t operands[] = {id};
        emitInst(opcode, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getStringTypeId()
    {
        if (stringTypeId)
            return stringTypeId;

        stringTypeId = allocId();
        uint32_t operands[] = {stringTypeId};
        emitInst(HGBCOpcode::StringType, operands, SLANG_COUNT_OF(operands));
        return stringTypeId;
    }

    void emitTypeSize(HGBCId typeId, uint32_t byteSize, uint32_t byteAlignment, uint32_t byteStride)
    {
        if (emittedTypeSizes.containsKey(typeId))
            return;

        emittedTypeSizes.add(typeId, true);
        uint32_t operands[] = {
            allocId(),
            typeId,
            byteSize,
            byteAlignment,
            byteStride,
        };
        emitInst(HGBCOpcode::TypeSize, operands, SLANG_COUNT_OF(operands));
    }

    /// Returns a const-heap description of one logical type in one layout domain.
    ///
    /// Consider a host `Material` copied into a structured buffer. The host and device records
    /// both name `Material`, but the device record asks Slang for structured-buffer offsets while
    /// the host record uses the offsets that staging already assigned to stack values.
    HGBCId getTypeLayoutId(
        HGBCId logicalTypeId,
        HGBCLayoutDomain domain,
        HGBCLayoutUsage usage,
        HGBCId contextId = 0,
        uint32_t contextIndex = 0,
        uint32_t entryPointIndex = 0)
    {
        StringBuilder keyBuilder;
        keyBuilder << logicalTypeId << ":" << uint32_t(domain) << ":" << uint32_t(usage) << ":"
                   << contextId << ":" << contextIndex << ":" << entryPointIndex;
        String key = keyBuilder.produceString();
        if (auto existingId = typeLayoutIds.tryGetValue(key))
            return *existingId;

        HGBCId resultId = allocId();
        typeLayoutIds.add(key, resultId);
        uint32_t operands[] = {
            resultId,
            logicalTypeId,
            uint32_t(domain),
            uint32_t(usage),
            contextId,
            contextIndex,
            entryPointIndex,
        };
        emitInst(HGBCOpcode::TypeLayout, operands, entryPointIndex ? 7 : 6);
        return resultId;
    }

    /// Returns a lazy conversion recipe between two layouts of the same logical type.
    ///
    /// For example, a `bool` may occupy one byte in a staged host struct and four bytes in a
    /// structured buffer. The converter is resolved once and then reused by every copy.
    HGBCId getLayoutConverterId(HGBCId sourceLayoutId, HGBCId destinationLayoutId)
    {
        StringBuilder keyBuilder;
        keyBuilder << sourceLayoutId << ":" << destinationLayoutId;
        String key = keyBuilder.produceString();
        if (auto existingId = layoutConverterIds.tryGetValue(key))
            return *existingId;

        HGBCId resultId = allocId();
        layoutConverterIds.add(key, resultId);
        uint32_t operands[] = {resultId, sourceLayoutId, destinationLayoutId};
        emitInst(HGBCOpcode::LayoutConverter, operands, SLANG_COUNT_OF(operands));
        return resultId;
    }

    HGBCId getRWStructuredBufferTypeId(HGBCId elementType, uint32_t elementStride)
    {
        StringBuilder keyBuilder;
        keyBuilder << elementType;
        keyBuilder << ":";
        keyBuilder << elementStride;
        String key = keyBuilder.produceString();

        if (auto existingId = rwStructuredBufferTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        rwStructuredBufferTypeIds.add(key, id);
        structuredBufferElementTypeIds.add(id, elementType);

        uint32_t operands[] = {id, elementType};
        emitInst(HGBCOpcode::RWStructuredBufferType, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getStructuredBufferTypeId(HGBCId elementType, uint32_t elementStride)
    {
        StringBuilder keyBuilder;
        keyBuilder << elementType;
        keyBuilder << ":";
        keyBuilder << elementStride;
        String key = keyBuilder.produceString();

        if (auto existingId = structuredBufferTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        structuredBufferTypeIds.add(key, id);
        structuredBufferElementTypeIds.add(id, elementType);

        uint32_t operands[] = {id, elementType};
        emitInst(HGBCOpcode::StructuredBufferType, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getStructuredBufferElementTypeId(HGBCId bufferTypeId)
    {
        auto elementTypeId = structuredBufferElementTypeIds.tryGetValue(bufferTypeId);
        SLANG_RELEASE_ASSERT(elementTypeId);
        return *elementTypeId;
    }

    /// Emits the ordinary value type used to pass one resource through a descriptor heap.
    HGBCId getDescriptorHandleTypeId(
        IRDescriptorHandleType* type,
        HGBCId resourceTypeId,
        HGBCId reflectionNameId)
    {
        if (auto existingId = descriptorHandleTypeIds.tryGetValue(type))
            return *existingId;

        HGBCId id = allocId();
        // Buffer storage types may share an HGBC representation after their source layout
        // parameter is erased. Descriptor handles retain the original semantic identity.
        descriptorHandleTypeIds.add(type, id);
        uint32_t operands[] = {id, resourceTypeId};
        emitInst(HGBCOpcode::DescriptorHandleType, operands, SLANG_COUNT_OF(operands));
        if (reflectionNameId)
            emitSlangTypeName(id, reflectionNameId);
        return id;
    }

    /// Resolves the logical element and physical layout domain of one bulk-copy endpoint type.
    bool tryGetArrayCopyEndpointLayout(
        HGBCId endpointTypeId,
        HGBCId& outElementTypeId,
        HGBCLayoutDomain& outDomain,
        HGBCLayoutUsage& outUsage)
    {
        if (auto elementTypeId = runtimeArrayElementTypeIds.tryGetValue(endpointTypeId))
        {
            outElementTypeId = *elementTypeId;
            outDomain = HGBCLayoutDomain::Host;
            outUsage = HGBCLayoutUsage::Value;
            return true;
        }
        if (auto elementTypeId = structuredBufferElementTypeIds.tryGetValue(endpointTypeId))
        {
            outElementTypeId = *elementTypeId;
            outDomain = HGBCLayoutDomain::DeviceMemory;
            outUsage = HGBCLayoutUsage::StructuredBufferElement;
            return true;
        }
        return false;
    }

    /// Emits one texture-handle type with independent shader-value and storage-format operands.
    ///
    /// Consider `RenderTarget2D<float4, RGBA8UnormSrgb>`: `float4` describes values crossing the
    /// shader boundary, while the explicit format records four-byte normalized sRGB storage.
    HGBCId getRenderTarget2DTypeId(HGBCId elementType, HGBCResourceFormat format)
    {
        StringBuilder keyBuilder;
        keyBuilder << elementType << ":" << uint32_t(format);
        String key = keyBuilder.produceString();

        if (auto existingId = renderTarget2DTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        renderTarget2DTypeIds.add(key, id);

        uint32_t operands[] = {id, elementType, uint32_t(format)};
        emitInst(HGBCOpcode::RenderTarget2DType, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    /// Emits one ordinary 2D texture type without discarding its source-level access or format.
    HGBCId getTexture2DTypeId(
        HGBCId elementType,
        HGBCTextureAccess access,
        HGBCResourceFormat format)
    {
        StringBuilder keyBuilder;
        keyBuilder << elementType << ":" << uint32_t(access) << ":" << uint32_t(format);
        String key = keyBuilder.produceString();
        if (auto existingId = texture2DTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        texture2DTypeIds.add(key, id);

        uint32_t operands[] = {id, elementType, uint32_t(access), uint32_t(format)};
        emitInst(HGBCOpcode::Texture2DType, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    /// Emits one presentation-surface type whose requested logical format survives specialization.
    ///
    /// For example, every `Surface<RGBA8UnormSrgb>` shares one HGBC type record, while an HDR
    /// `Surface<RGBA16Float>` receives a distinct record and cannot be mixed with it accidentally.
    HGBCId getSurfaceTypeId(HGBCResourceFormat format)
    {
        uint32_t key = uint32_t(format);
        if (auto existingId = surfaceTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        surfaceTypeIds.add(key, id);
        uint32_t operands[] = {id, key};
        emitInst(HGBCOpcode::SurfaceType, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getVectorTypeId(uint32_t elementCount, HGBCId elementType)
    {
        StringBuilder keyBuilder;
        keyBuilder << elementCount;
        keyBuilder << ":";
        keyBuilder << elementType;
        String key = keyBuilder.produceString();

        if (auto existingId = vectorTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        vectorTypeIds.add(key, id);

        uint32_t operands[] = {id, elementCount, elementType};
        emitInst(HGBCOpcode::VectorType, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getMatrixTypeId(
        uint32_t rowCount,
        uint32_t columnCount,
        uint32_t layout,
        HGBCId elementType)
    {
        StringBuilder keyBuilder;
        keyBuilder << rowCount << ":" << columnCount << ":" << layout << ":" << elementType;
        String key = keyBuilder.produceString();

        if (auto existingId = matrixTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        matrixTypeIds.add(key, id);

        uint32_t operands[] = {id, rowCount, columnCount, layout, elementType};
        emitInst(HGBCOpcode::MatrixType, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getArrayTypeId(uint32_t elementCount, HGBCId elementType)
    {
        StringBuilder keyBuilder;
        keyBuilder << elementCount << ":" << elementType;
        String key = keyBuilder.produceString();

        if (auto existingId = arrayTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        arrayTypeIds.add(key, id);

        uint32_t operands[] = {id, elementCount, elementType};
        emitInst(HGBCOpcode::ArrayType, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    // Returns the canonical HGBC address-wrapper type for a value type.
    //
    // Consider `Ptr<uint>` and `out uint`: both contain the same value type, but their wrapper
    // opcodes remain distinct in the key. Consumers can therefore classify either as an address
    // while preserving the source-level passing convention in the type structure.
    HGBCId getAddressTypeId(HGBCOpcode opcode, HGBCId valueType)
    {
        SLANG_ASSERT(hasHGBCOpcodeFlag(opcode, HGBCOpcodeFlags::AddressType));
        uint64_t key = (uint64_t(opcode) << 32) | valueType;
        if (auto existingId = addressTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        addressTypeIds.add(key, id);
        uint32_t operands[] = {id, valueType};
        emitInst(opcode, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId getDeferTypeId(HGBCId valueType)
    {
        if (auto existingId = deferTypeIds.tryGetValue(valueType))
            return *existingId;

        HGBCId id = allocId();
        deferTypeIds.add(valueType, id);
        uint32_t operands[] = {id, valueType};
        emitInst(HGBCOpcode::DeferType, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    HGBCId beginStructTypeId(IRStructType* type)
    {
        if (auto existingId = structTypeIds.tryGetValue(type))
            return *existingId;

        HGBCId id = allocId();
        structTypeIds.add(type, id);
        aggregateTypeIds.add(id, true);
        return id;
    }

    void emitStructType(HGBCId resultId, const List<HGBCId>& fieldNameAndTypeIds)
    {
        List<uint32_t> operands;
        operands.add(resultId);
        for (auto id : fieldNameAndTypeIds)
            operands.add(id);
        emitInst(HGBCOpcode::StructType, operands.getBuffer(), operands.getCount());
    }

    HGBCId beginClassTypeId(IRClassType* type)
    {
        if (auto existingId = classTypeIds.tryGetValue(type))
            return *existingId;

        HGBCId id = allocId();
        classTypeIds.add(type, id);
        aggregateTypeIds.add(id, true);
        return id;
    }

    void emitClassType(HGBCId resultId, const List<HGBCId>& fieldNameAndTypeIds)
    {
        List<uint32_t> operands;
        operands.add(resultId);
        for (auto id : fieldNameAndTypeIds)
            operands.add(id);
        emitInst(HGBCOpcode::ClassType, operands.getBuffer(), operands.getCount());
    }

    void emitAllocateObject(HGBCId typeId, HGBCId resultId)
    {
        uint32_t operands[] = {typeId, resultId};
        emitInst(HGBCOpcode::AllocateObject, operands, SLANG_COUNT_OF(operands));
    }

    HGBCId beginRuntimeArrayTypeId(IRHGVMRuntimeArrayType* type)
    {
        if (auto existingId = runtimeArrayTypeIds.tryGetValue(type))
            return *existingId;

        HGBCId id = allocId();
        runtimeArrayTypeIds.add(type, id);
        return id;
    }

    HGBCId getNullLiteralId(HGBCId typeId)
    {
        if (auto existingId = nullLiteralIds.tryGetValue(typeId))
            return *existingId;

        HGBCId id = allocId();
        nullLiteralIds.add(typeId, id);

        uint32_t operands[] = {typeId, id};
        emitInst(HGBCOpcode::NullLit, operands, SLANG_COUNT_OF(operands));
        return id;
    }

    void emitRuntimeArrayType(HGBCId resultId, HGBCId elementTypeId)
    {
        runtimeArrayElementTypeIds.add(resultId, elementTypeId);
        uint32_t operands[] = {resultId, elementTypeId};
        emitInst(HGBCOpcode::RuntimeArrayType, operands, SLANG_COUNT_OF(operands));
    }

    HGBCId getFuncTypeId(HGBCId resultType, const List<HGBCId>& paramTypes)
    {
        StringBuilder keyBuilder;
        keyBuilder << resultType;
        keyBuilder << ":";
        for (auto paramType : paramTypes)
        {
            keyBuilder << paramType;
            keyBuilder << ",";
        }
        String key = keyBuilder.produceString();

        if (auto existingId = funcTypeIds.tryGetValue(key))
            return *existingId;

        HGBCId id = allocId();
        funcTypeIds.add(key, id);

        List<uint32_t> operands;
        operands.add(id);
        operands.add(resultType);
        for (auto paramType : paramTypes)
            operands.add(paramType);
        emitInst(HGBCOpcode::FuncType, operands.getBuffer(), operands.getCount());
        return id;
    }

    void emitExport(HGBCExportKind kind, HGBCId nameId, HGBCId targetId, HGBCId ownerTypeId = 0)
    {
        List<uint32_t> operands;
        operands.add(allocId());
        operands.add(uint32_t(kind));
        operands.add(nameId);
        operands.add(targetId);
        if (ownerTypeId)
            operands.add(ownerTypeId);
        emitInst(HGBCOpcode::Export, operands.getBuffer(), operands.getCount());
    }

    void emitDecorate(HGBCDecorationKind kind, HGBCId targetId, HGBCId operandId)
    {
        uint32_t operands[] = {
            allocId(),
            uint32_t(kind),
            targetId,
            operandId,
        };
        emitInst(HGBCOpcode::Decorate, operands, SLANG_COUNT_OF(operands));
    }

    void emitNameHint(HGBCId targetId, HGBCId nameId)
    {
        emitDecorate(HGBCDecorationKind::NameHint, targetId, nameId);
    }

    void emitSlangTypeName(HGBCId targetId, HGBCId nameId)
    {
        slangTypeNameIds.add(targetId, true);
        emitDecorate(HGBCDecorationKind::SlangTypeName, targetId, nameId);
    }

    bool isAggregateTypeWithoutSlangTypeName(HGBCId typeId)
    {
        return aggregateTypeIds.containsKey(typeId) && !slangTypeNameIds.containsKey(typeId);
    }

    void emitDebugSource(HGBCId resultId, HGBCId pathId, HGBCId contentId)
    {
        uint32_t operands[] = {resultId, pathId, contentId};
        emitInst(HGBCOpcode::DebugSource, operands, SLANG_COUNT_OF(operands));
    }

    void emitDebugFunction(
        HGBCId resultId,
        HGBCId nameId,
        HGBCId sourceId,
        uint32_t line,
        uint32_t column)
    {
        uint32_t operands[] = {resultId, nameId, sourceId, line, column};
        emitInst(HGBCOpcode::DebugFunction, operands, SLANG_COUNT_OF(operands));
    }

    void emitDebugInlinedAt(
        HGBCId resultId,
        HGBCId scopeId,
        HGBCId sourceId,
        uint32_t line,
        uint32_t column,
        HGBCId parentId)
    {
        uint32_t operands[] = {resultId, scopeId, sourceId, line, column, parentId};
        emitInst(HGBCOpcode::DebugInlinedAt, operands, SLANG_COUNT_OF(operands));
    }

    void emitDebugVariable(
        HGBCId resultId,
        HGBCId nameId,
        HGBCId typeId,
        HGBCId sourceId,
        uint32_t line,
        uint32_t column,
        HGBCId scopeId,
        uint32_t argumentIndexPlusOne)
    {
        List<uint32_t> operands;
        operands.add(resultId);
        operands.add(nameId);
        operands.add(typeId);
        operands.add(sourceId);
        operands.add(line);
        operands.add(column);
        operands.add(scopeId);
        if (argumentIndexPlusOne)
            operands.add(argumentIndexPlusOne);
        emitInst(HGBCOpcode::DebugVariable, operands.getBuffer(), operands.getCount());
    }

    void emitDebugValue(HGBCId variableId, HGBCId valueId, const List<uint32_t>& path)
    {
        List<uint32_t> operands;
        operands.add(variableId);
        if (valueId)
        {
            operands.add(valueId);
            operands.addRange(path);
        }
        emitInst(HGBCOpcode::DebugValue, operands.getBuffer(), operands.getCount());
    }

    void emitDebugLexicalBlock(
        HGBCId resultId,
        HGBCId parentId,
        HGBCId sourceId,
        uint32_t lineStart,
        uint32_t columnStart,
        uint32_t lineEnd,
        uint32_t columnEnd)
    {
        uint32_t operands[] = {
            resultId,
            parentId,
            sourceId,
            lineStart,
            columnStart,
            lineEnd,
            columnEnd,
        };
        emitInst(HGBCOpcode::DebugLexicalBlock, operands, SLANG_COUNT_OF(operands));
    }

    void emitDebugLine(
        HGBCId sourceId,
        uint32_t lineStart,
        uint32_t lineEnd,
        uint32_t columnStart,
        uint32_t columnEnd)
    {
        uint32_t operands[] = {sourceId, lineStart, lineEnd, columnStart, columnEnd};
        emitInst(HGBCOpcode::DebugLine, operands, SLANG_COUNT_OF(operands));
    }

    void emitDebugScope(HGBCId scopeId, HGBCId inlinedAtId)
    {
        uint32_t operands[] = {scopeId, inlinedAtId};
        emitInst(HGBCOpcode::DebugScope, operands, inlinedAtId ? 2 : 1);
    }

    void emitDebugNoScope() { emitInst(HGBCOpcode::DebugNoScope, nullptr, 0); }

    void emitDebugBuildIdentifier(HGBCId resultId, HGBCId identifierId, uint32_t flags)
    {
        uint32_t operands[] = {resultId, identifierId, flags};
        emitInst(HGBCOpcode::DebugBuildIdentifier, operands, SLANG_COUNT_OF(operands));
    }

    void emitDebugCompilationUnit(HGBCId resultId, HGBCId sourceId)
    {
        uint32_t operands[] = {resultId, sourceId};
        emitInst(HGBCOpcode::DebugCompilationUnit, operands, SLANG_COUNT_OF(operands));
    }

    void emitDebugInlinedVariable(HGBCId resultId, HGBCId variableId, HGBCId inlinedAtId)
    {
        uint32_t operands[] = {resultId, variableId, inlinedAtId};
        emitInst(HGBCOpcode::DebugInlinedVariable, operands, SLANG_COUNT_OF(operands));
    }

    void emitFunc(HGBCId typeId, HGBCId resultId)
    {
        uint32_t operands[] = {typeId, resultId};
        emitInst(HGBCOpcode::Func, operands, SLANG_COUNT_OF(operands));
    }

    void emitFunctionEnd() { emitInst(HGBCOpcode::FunctionEnd, nullptr, 0); }

    void emitBlock(HGBCId id)
    {
        uint32_t operands[] = {id};
        emitInst(HGBCOpcode::Block, operands, SLANG_COUNT_OF(operands));
    }

    HGBCId emitBlock()
    {
        HGBCId id = allocId();
        emitBlock(id);
        return id;
    }

    void emitBlockEnd() { emitInst(HGBCOpcode::BlockEnd, nullptr, 0); }

    void emitParam(HGBCId typeId, HGBCId resultId)
    {
        uint32_t operands[] = {typeId, resultId};
        emitInst(HGBCOpcode::Param, operands, SLANG_COUNT_OF(operands));
    }

    void emitVar(HGBCId typeId, HGBCId resultId)
    {
        uint32_t operands[] = {typeId, resultId};
        emitInst(HGBCOpcode::Var, operands, SLANG_COUNT_OF(operands));
    }

    void emitLoad(HGBCId typeId, HGBCId resultId, HGBCId ptrId)
    {
        uint32_t operands[] = {typeId, resultId, ptrId};
        emitInst(HGBCOpcode::Load, operands, SLANG_COUNT_OF(operands));
    }

    void emitDescriptorHandleToResource(HGBCId typeId, HGBCId resultId, HGBCId handleId)
    {
        uint32_t operands[] = {typeId, resultId, handleId};
        emitInst(HGBCOpcode::DescriptorHandleToResource, operands, SLANG_COUNT_OF(operands));
    }

    void emitStore(HGBCId ptrId, HGBCId valueId)
    {
        uint32_t operands[] = {ptrId, valueId};
        emitInst(HGBCOpcode::Store, operands, SLANG_COUNT_OF(operands));
    }

    void emitFieldAddress(HGBCId typeId, HGBCId resultId, HGBCId baseId, HGBCId fieldNameId)
    {
        uint32_t operands[] = {typeId, resultId, baseId, fieldNameId};
        emitInst(HGBCOpcode::FieldAddress, operands, SLANG_COUNT_OF(operands));
    }

    void emitElementAddress(HGBCId typeId, HGBCId resultId, HGBCId baseId, HGBCId indexId)
    {
        uint32_t operands[] = {typeId, resultId, baseId, indexId};
        emitInst(HGBCOpcode::ElementAddress, operands, SLANG_COUNT_OF(operands));
    }

    void emitStructuredBufferGetDimensions(
        HGBCId typeId,
        HGBCId resultId,
        HGBCId bufferTypeId,
        HGBCId bufferId)
    {
        uint32_t operands[] = {typeId, resultId, bufferTypeId, bufferId};
        emitInst(HGBCOpcode::StructuredBufferGetDimensions, operands, SLANG_COUNT_OF(operands));
    }

    void emitTextureGetDimensions(HGBCId typeId, HGBCId resultId, HGBCId textureId)
    {
        uint32_t operands[] = {typeId, resultId, textureId};
        emitInst(HGBCOpcode::TextureGetDimensions, operands, SLANG_COUNT_OF(operands));
    }

    void emitExtractElement(HGBCId typeId, HGBCId resultId, HGBCId baseId, HGBCId indexId)
    {
        uint32_t operands[] = {typeId, resultId, baseId, indexId};
        emitInst(HGBCOpcode::ExtractElement, operands, SLANG_COUNT_OF(operands));
    }

    void emitPrint(HGBCId valueTypeId, HGBCId valueId)
    {
        uint32_t operands[] = {valueTypeId, valueId};
        emitInst(HGBCOpcode::Print, operands, SLANG_COUNT_OF(operands));
    }

    void emitUnary(HGBCOpcode opcode, HGBCId typeId, HGBCId resultId, HGBCId valueId)
    {
        uint32_t operands[] = {typeId, resultId, valueId};
        emitInst(opcode, operands, SLANG_COUNT_OF(operands));
    }

    // Emits the single scalar-cast representation shared by every basic source/destination pair.
    //
    // Consider both `float(i)` and `uint(f)`: the serialized operation data is
    // `{sourceType, destinationType, sourceValue}` after its result ID. HGVM stages those two type
    // IDs into scalar codes once instead of selecting behavior by inspecting the runtime value.
    void emitScalarTypeCast(
        HGBCId resultId,
        HGBCId sourceTypeId,
        HGBCId destinationTypeId,
        HGBCId valueId)
    {
        uint32_t operands[] = {resultId, sourceTypeId, destinationTypeId, valueId};
        emitInst(HGBCOpcode::ScalarTypeCast, operands, SLANG_COUNT_OF(operands));
    }

    // Emits the common typed-binary record used by arithmetic and comparison operations.
    //
    // Consider `a + b` and `a < b`: both place the operand type before the two values. Addition
    // repeats that type as its result type, while comparison supplies `bool` as the result type.
    void emitTypedBinaryOp(
        HGBCOpcode opcode,
        HGBCId resultTypeId,
        HGBCId resultId,
        HGBCId operandTypeId,
        HGBCId leftId,
        HGBCId rightId)
    {
        uint32_t operands[] = {resultTypeId, resultId, operandTypeId, leftId, rightId};
        emitInst(opcode, operands, SLANG_COUNT_OF(operands));
    }

    void emitNot(HGBCId typeId, HGBCId resultId, HGBCId valueId)
    {
        uint32_t operands[] = {typeId, resultId, valueId};
        emitInst(HGBCOpcode::Not, operands, SLANG_COUNT_OF(operands));
    }

    void emitMakeVector(HGBCId typeId, HGBCId resultId, const List<HGBCId>& elementIds)
    {
        List<uint32_t> operands;
        operands.add(typeId);
        operands.add(resultId);
        for (auto elementId : elementIds)
            operands.add(elementId);
        emitInst(HGBCOpcode::MakeVector, operands.getBuffer(), operands.getCount());
    }

    void emitMakeMatrix(HGBCId typeId, HGBCId resultId, const List<HGBCId>& elementIds)
    {
        List<uint32_t> operands;
        operands.add(typeId);
        operands.add(resultId);
        for (auto elementId : elementIds)
            operands.add(elementId);
        emitInst(HGBCOpcode::MakeMatrix, operands.getBuffer(), operands.getCount());
    }

    void emitMakeArray(HGBCId typeId, HGBCId resultId, const List<HGBCId>& elementIds)
    {
        List<uint32_t> operands;
        operands.add(typeId);
        operands.add(resultId);
        for (auto elementId : elementIds)
            operands.add(elementId);
        emitInst(HGBCOpcode::MakeArray, operands.getBuffer(), operands.getCount());
    }

    void emitMakeStruct(HGBCId typeId, HGBCId resultId, const List<HGBCId>& fieldValueIds)
    {
        List<uint32_t> operands;
        operands.add(typeId);
        operands.add(resultId);
        for (auto fieldValueId : fieldValueIds)
            operands.add(fieldValueId);
        emitInst(HGBCOpcode::MakeStruct, operands.getBuffer(), operands.getCount());
    }

    void emitFieldExtract(HGBCId typeId, HGBCId resultId, HGBCId baseId, HGBCId fieldNameId)
    {
        uint32_t operands[] = {typeId, resultId, baseId, fieldNameId};
        emitInst(HGBCOpcode::FieldExtract, operands, SLANG_COUNT_OF(operands));
    }

    HGBCId getComputeDispatcherId(
        HGBCId typeId,
        HGBCId entryPointId,
        const List<HGBCId>& argumentTypeIds,
        const List<uint32_t>& parameterIndices)
    {
        SLANG_RELEASE_ASSERT(parameterIndices.getCount() + 1 == argumentTypeIds.getCount());
        StringBuilder keyBuilder;
        keyBuilder << typeId;
        keyBuilder << ":";
        keyBuilder << entryPointId;
        for (auto index : parameterIndices)
            keyBuilder << ":" << index;
        String key = keyBuilder.produceString();

        if (auto existingId = computeDispatcherIds.tryGetValue(key))
            return *existingId;

        HGBCId resultId = allocId();
        computeDispatcherIds.add(key, resultId);

        List<uint32_t> operands;
        operands.add(typeId);
        operands.add(resultId);
        operands.add(uint32_t(HGBCDispatchStage::Compute));
        operands.add(entryPointId);

        // The dispatch size controls execution. Each remaining argument retains its original
        // shader parameter index, even when system values are omitted from the host call or
        // legalization erases empty arguments. Resource consumption does not change this identity.
        for (Index argumentIndex = 1; argumentIndex < argumentTypeIds.getCount(); ++argumentIndex)
        {
            HGBCId logicalTypeId = argumentTypeIds[argumentIndex];
            HGBCId hostLayoutId =
                getTypeLayoutId(logicalTypeId, HGBCLayoutDomain::Host, HGBCLayoutUsage::Value);
            HGBCId shaderLayoutId = getTypeLayoutId(
                logicalTypeId,
                HGBCLayoutDomain::ShaderObject,
                HGBCLayoutUsage::ShaderParameter,
                resultId,
                parameterIndices[argumentIndex - 1]);
            operands.add(getLayoutConverterId(hostLayoutId, shaderLayoutId));
        }
        emitInst(HGBCOpcode::MakeDispatcher, operands.getBuffer(), operands.getCount());
        return resultId;
    }

    HGBCId getRayTracerId(
        HGBCId typeId,
        HGBCId payloadTypeId,
        uint32_t maxRecursion,
        HGBCId rayGenerationEntryPointId,
        HGBCId closestHitEntryPointId,
        HGBCId missEntryPointId)
    {
        StringBuilder keyBuilder;
        keyBuilder << typeId << ":" << payloadTypeId << ":" << maxRecursion << ":"
                   << rayGenerationEntryPointId << ":" << closestHitEntryPointId << ":"
                   << missEntryPointId;
        String key = keyBuilder.produceString();
        if (auto existingId = rayTracerIds.tryGetValue(key))
            return *existingId;

        HGBCId resultId = allocId();
        rayTracerIds.add(key, resultId);
        uint32_t operands[] = {
            typeId,
            resultId,
            payloadTypeId,
            maxRecursion,
            rayGenerationEntryPointId,
            closestHitEntryPointId,
            missEntryPointId,
        };
        emitInst(HGBCOpcode::MakeRayTracer, operands, SLANG_COUNT_OF(operands));
        return resultId;
    }

    void emitCall(HGBCId resultTypeId, HGBCId resultId, HGBCId calleeId, const List<HGBCId>& argIds)
    {
        List<uint32_t> operands;
        operands.add(resultTypeId);
        operands.add(resultId);
        operands.add(calleeId);
        for (auto argId : argIds)
            operands.add(argId);
        emitInst(HGBCOpcode::Call, operands.getBuffer(), operands.getCount());
    }

    void emitReturn() { emitInst(HGBCOpcode::Return, nullptr, 0); }

    void emitReturn(HGBCId valueId)
    {
        uint32_t operands[] = {valueId};
        emitInst(HGBCOpcode::Return, operands, SLANG_COUNT_OF(operands));
    }

    void emitBranch(HGBCId targetBlockId)
    {
        uint32_t operands[] = {targetBlockId};
        emitInst(HGBCOpcode::Branch, operands, SLANG_COUNT_OF(operands));
    }

    void emitBranchConditional(HGBCId conditionId, HGBCId trueBlockId, HGBCId falseBlockId)
    {
        uint32_t operands[] = {conditionId, trueBlockId, falseBlockId};
        emitInst(HGBCOpcode::BranchConditional, operands, SLANG_COUNT_OF(operands));
    }

    void emitSlangModule(
        HGBCId resultId,
        HGBCId nameId,
        HGBCId binaryLiteralId,
        const List<HGBCId>& dependencyIds)
    {
        List<uint32_t> operands;
        operands.add(resultId);
        operands.add(nameId);
        operands.add(binaryLiteralId);
        for (auto dependencyId : dependencyIds)
            operands.add(dependencyId);
        emitInst(HGBCOpcode::SlangModule, operands.getBuffer(), operands.getCount());
    }

    void emitSlangEntryPoint(HGBCId resultId, HGBCId moduleId, HGBCId entryPointNameId)
    {
        uint32_t operands[] = {resultId, moduleId, entryPointNameId};
        emitInst(HGBCOpcode::SlangEntryPoint, operands, SLANG_COUNT_OF(operands));
    }

    void emitSpecializeSlangEntryPoint(
        HGBCId resultId,
        HGBCId baseEntryPointId,
        const List<uint32_t>& specializationOperands)
    {
        List<uint32_t> operands;
        operands.add(resultId);
        operands.add(baseEntryPointId);
        operands.addRange(specializationOperands.getBuffer(), specializationOperands.getCount());
        emitInst(HGBCOpcode::SpecializeSlangEntryPoint, operands.getBuffer(), operands.getCount());
    }

    SlangResult writeContainer(ComPtr<ISlangBlob>& outBlob)
    {
        HGBC::Vector<uint8_t> bytes;
        std::string error;
        if (!HGBC::writeProgram(program, bytes, &error))
            return SLANG_FAIL;

        outBlob = RawBlob::create(bytes.getBuffer(), bytes.getCount());
        if (!outBlob)
            return SLANG_E_OUT_OF_MEMORY;
        return SLANG_OK;
    }
};

String getDebugName(IRInst* inst, const char* fallback)
{
    if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
        return String(nameHint->getName());

    if (auto linkage = inst->findDecoration<IRLinkageDecoration>())
        return String(linkage->getMangledName());

    return fallback;
}

// Returns whether an instruction is part of the callable API emitted by HGBC. HGBCExportDecoration
// preserves source-level `public` visibility solely for HGBC reflection, while HLSLExportDecoration
// represents an explicit cross-target export request.
bool isHGBCExportedAPI(IRInst* inst)
{
    return inst->findDecorationImpl(kIROp_HGBCExportDecoration) ||
           inst->findDecorationImpl(kIROp_HLSLExportDecoration);
}

bool isDispatchEntryPoint(IRFunc* func)
{
    auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
    return entryPointDecor && entryPointDecor->getProfile().getStage() == Stage::Dispatch;
}

bool isEmptyAggregate(IRInst* inst)
{
    if (!inst)
        return true;

    switch (inst->getOp())
    {
    case kIROp_MakeStruct:
    case kIROp_MakeTuple:
        return inst->getOperandCount() == 0;
    default:
        return false;
    }
}

bool isBuiltinModuleName(const String& name)
{
    return name == "core" || name == "hlsl" || name == "glsl" || name == "spirv" ||
           name == "cuda" || name == "metal" || name == "wgsl";
}

struct SerializedModuleRecord
{
    Module* module = nullptr;
    String name;
    String path;
    List<Module*> dependencies;
    ComPtr<ISlangBlob> blob;
    HGBCId binaryLiteralId = 0;
    HGBCId moduleId = 0;
};

struct HGBCEmitter
{
    CodeGenContext* codeGenContext = nullptr;
    DiagnosticSink* sink = nullptr;
    HGBCBuilder builder;
    Dictionary<IRInst*, HGBCId> valueIds;
    Dictionary<IRFunc*, HGBCId> funcIds;
    Dictionary<IRBlock*, HGBCId> blockIds;
    Dictionary<IRFunc*, bool> emittedFuncs;
    Dictionary<IRStructType*, bool> exportedStructTypes;
    Dictionary<IRClassType*, bool> exportedClassTypes;
    Dictionary<String, bool> exportedFunctionSignatures;
    List<IRFunc*> pendingFuncs;
    List<SerializedModuleRecord> serializedModules;
    Dictionary<IRStructKey*, HGBCId> fieldNameIds;
    Dictionary<String, HGBCId> slangEntryPointIds;
    Dictionary<IRInst*, HGBCId> debugSourceIds;
    Dictionary<IRInst*, HGBCId> debugFunctionIds;
    HashSet<IRInst*> emittedDebugLambdas;
    Dictionary<IRInst*, HGBCId> debugInlinedAtIds;
    Dictionary<IRInst*, HGBCId> debugLexicalBlockIds;
    Dictionary<IRInst*, HGBCId> debugVariableIds;
    Dictionary<IRInst*, HGBCId> debugInlinedVariableIds;
    Count anonymousFieldNameCounter = 0;

    bool hasSerializedModuleRecord(Module* module)
    {
        for (auto& moduleRecord : serializedModules)
        {
            if (moduleRecord.module == module)
                return true;
        }
        return false;
    }

    SerializedModuleRecord* findSerializedModuleRecord(Module* module)
    {
        for (auto& moduleRecord : serializedModules)
        {
            if (moduleRecord.module == module)
                return &moduleRecord;
        }
        return nullptr;
    }

    SlangResult fail(IRInst* inst, const char* feature)
    {
        SLANG_RELEASE_ASSERT(inst);

        if (sink)
        {
            sink->diagnose(Diagnostics::HgbcUnsupportedFeature{
                .feature = feature,
                .location = inst->sourceLoc,
            });
        }
        return SLANG_FAIL;
    }

    SlangResult requireDeviceMemorySlangTypeName(IRInst* inst, HGBCId typeId)
    {
        if (builder.isAggregateTypeWithoutSlangTypeName(typeId))
            return fail(inst, "device-memory aggregate types without reflection names");
        return SLANG_OK;
    }

    /// Rejects structured-buffer layout policies that HGBC cannot reproduce at runtime.
    ///
    /// HGBC currently resolves device storage with `DefaultStructuredBuffer` reflection rules and
    /// does not serialize the `L` argument from `StructuredBuffer<T, L>`. `ScalarDataLayout` is
    /// retained for scalar-record buffers such as the packed PLY loader; the remaining policies
    /// require an explicit layout operand in the HGBC schema before they can be used safely.
    SlangResult requireSimpleStructuredBufferLayout(
        IRInst* diagnosticInst,
        IRHLSLStructuredBufferTypeBase* bufferType)
    {
        IRType* dataLayout = bufferType->getDataLayout();
        if (!dataLayout)
            return SLANG_OK;

        switch (dataLayout->getOp())
        {
        case kIROp_DefaultBufferLayoutType:
        case kIROp_ScalarBufferLayoutType:
            return SLANG_OK;
        default:
            return fail(
                diagnosticInst,
                "structured-buffer data layouts other than DefaultDataLayout and "
                "ScalarDataLayout");
        }
    }

    uint32_t getDebugInteger(IRInst* inst)
    {
        auto value = as<IRIntLit>(inst);
        SLANG_RELEASE_ASSERT(value);
        SLANG_RELEASE_ASSERT(
            value->getValue() >= 0 && value->getValue() <= IRIntegerValue(UINT32_MAX));
        return uint32_t(value->getValue());
    }

    HGBCId getDebugSourceId(IRInst* inst)
    {
        if (!inst)
            return 0;
        if (auto id = debugSourceIds.tryGetValue(inst))
            return *id;

        auto debugSource = as<IRDebugSource>(inst);
        SLANG_RELEASE_ASSERT(debugSource);
        auto path = as<IRStringLit>(debugSource->getFileName());
        auto content = as<IRStringLit>(debugSource->getSource());
        SLANG_RELEASE_ASSERT(path && content);

        HGBCId id = builder.allocId();
        debugSourceIds.add(inst, id);
        builder.emitDebugSource(
            id,
            builder.getStringLiteralId(path->getStringSlice()),
            builder.getStringLiteralId(content->getStringSlice()));
        return id;
    }

    HGBCId getDebugFunctionId(IRInst* inst)
    {
        if (!inst)
            return 0;
        if (auto id = debugFunctionIds.tryGetValue(inst))
            return *id;

        auto debugFunction = as<IRDebugFunction>(inst);
        SLANG_RELEASE_ASSERT(debugFunction);
        auto name = as<IRStringLit>(debugFunction->getName());
        SLANG_RELEASE_ASSERT(name);

        HGBCId id = builder.allocId();
        debugFunctionIds.add(inst, id);
        builder.emitDebugFunction(
            id,
            builder.getStringLiteralId(name->getStringSlice()),
            getDebugSourceId(debugFunction->getFile()),
            getDebugInteger(debugFunction->getLine()),
            getDebugInteger(debugFunction->getCol()));
        return id;
    }

    /// Emits the name-to-definition association without requiring a runtime closure type.
    void emitDebugLambda(IRInst* inst)
    {
        if (!inst || !emittedDebugLambdas.add(inst))
            return;
        uint32_t operands[] = {
            builder.allocId(),
            builder.getStringLiteralId(as<IRStringLit>(inst->getOperand(0))->getStringSlice()),
            builder.getStringLiteralId(as<IRStringLit>(inst->getOperand(1))->getStringSlice()),
            getDebugSourceId(inst->getOperand(2)),
            getDebugInteger(inst->getOperand(3)),
            getDebugInteger(inst->getOperand(4))};
        builder.emitInst(HGBCOpcode::DebugLambda, operands, SLANG_COUNT_OF(operands));
    }

    HGBCId getDebugScopeId(IRInst* inst)
    {
        if (!inst)
            return 0;
        if (auto debugFunction = as<IRDebugFunction>(inst))
            return getDebugFunctionId(debugFunction);
        if (auto id = debugLexicalBlockIds.tryGetValue(inst))
            return *id;

        auto lexicalScope = as<IRDebugLexicalBlock>(inst);
        SLANG_RELEASE_ASSERT(lexicalScope);
        HGBCId id = builder.allocId();
        debugLexicalBlockIds.add(inst, id);
        builder.emitDebugLexicalBlock(
            id,
            getDebugScopeId(lexicalScope->getParentScope()),
            getDebugSourceId(lexicalScope->getSource()),
            getDebugInteger(lexicalScope->getLineStart()),
            getDebugInteger(lexicalScope->getColumnStart()),
            getDebugInteger(lexicalScope->getLineEnd()),
            getDebugInteger(lexicalScope->getColumnEnd()));
        return id;
    }

    HGBCId getDebugInlineSiteId(IRInst* inst)
    {
        if (!inst)
            return 0;
        if (auto id = debugInlinedAtIds.tryGetValue(inst))
            return *id;

        auto inlineSite = as<IRDebugInlinedAt>(inst);
        SLANG_RELEASE_ASSERT(inlineSite);

        HGBCId id = builder.allocId();
        debugInlinedAtIds.add(inst, id);
        builder.emitDebugInlinedAt(
            id,
            getDebugScopeId(inlineSite->getScope()),
            getDebugSourceId(inlineSite->getFile()),
            getDebugInteger(inlineSite->getLine()),
            getDebugInteger(inlineSite->getCol()),
            getDebugInlineSiteId(inlineSite->getOuterInlinedAt()));
        return id;
    }

    IRDebugFunction* getDebugFunction(IRFunc* func)
    {
        if (auto decoration = func->findDecoration<IRDebugFuncDecoration>())
            return as<IRDebugFunction>(decoration->getDebugFunc());
        return nullptr;
    }

    SlangResult emitDebugVariable(IRDebugVar* debugVariable)
    {
        if (!debugVariable->getSource() || !debugVariable->getScope())
            return SLANG_OK;

        IRBuilder irBuilder(debugVariable);
        IRType* variableType = tryGetPointedToType(&irBuilder, debugVariable->getDataType());
        if (!variableType)
            return SLANG_OK;

        HGBCId typeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(variableType, typeId));

        uint32_t argumentIndexPlusOne = 0;
        if (auto argumentIndex = as<IRIntLit>(debugVariable->getArgIndex()))
        {
            if (argumentIndex->getValue() < 0 ||
                argumentIndex->getValue() >= IRIntegerValue(UINT32_MAX))
            {
                return fail(debugVariable, "debug variable argument indices");
            }
            argumentIndexPlusOne = uint32_t(argumentIndex->getValue()) + 1;
        }

        HGBCId variableId = builder.allocId();
        debugVariableIds.add(debugVariable, variableId);
        String name = getDebugName(debugVariable, "unnamed");
        builder.emitDebugVariable(
            variableId,
            builder.getStringLiteralId(name.getUnownedSlice()),
            typeId,
            getDebugSourceId(debugVariable->getSource()),
            getDebugInteger(debugVariable->getLine()),
            getDebugInteger(debugVariable->getCol()),
            getDebugScopeId(debugVariable->getScope()),
            argumentIndexPlusOne);
        return SLANG_OK;
    }

    HGBCId getDebugVariableId(IRInst* variable)
    {
        if (auto variableId = debugVariableIds.tryGetValue(variable))
            return *variableId;
        if (auto aliasId = debugInlinedVariableIds.tryGetValue(variable))
            return *aliasId;

        auto inlinedVariable = as<IRDebugInlinedVariable>(variable);
        if (!inlinedVariable)
            return 0;

        HGBCId baseVariableId = getDebugVariableId(inlinedVariable->getVariable());
        if (!baseVariableId)
            return 0;

        HGBCId id = builder.allocId();
        debugInlinedVariableIds.add(variable, id);
        builder.emitDebugInlinedVariable(
            id,
            baseVariableId,
            getDebugInlineSiteId(inlinedVariable->getInlinedAt()));
        return id;
    }

    bool tryGetDebugValueId(IRInst* value, HGBCId& outValueId)
    {
        // Debug metadata must not make an otherwise unused graph value executable. Reuse values
        // that ordinary lowering has already emitted, while allowing literal values because they
        // live in LITR and have no runtime side effects.
        if (auto valueId = valueIds.tryGetValue(value))
        {
            outValueId = *valueId;
            return true;
        }

        switch (value->getOp())
        {
        case kIROp_IntLit:
        case kIROp_BoolLit:
        case kIROp_FloatLit:
        case kIROp_StringLit:
            return SLANG_SUCCEEDED(emitInstResultId(value, outValueId));
        default:
            return false;
        }
    }

    SlangResult emitDebugValue(IRDebugValue* debugValue)
    {
        List<IRInst*> accessChain;
        List<IRInst*> accessTypes;
        IRInst* rootVariable = getRootAddr(debugValue->getDebugVar(), accessChain, &accessTypes);
        HGBCId variableId = getDebugVariableId(rootVariable);
        if (!variableId)
            return SLANG_OK;

        HGBCId valueId = 0;
        List<uint32_t> path;
        if (tryGetDebugValueId(debugValue->getValue(), valueId))
        {
            IRBuilder irBuilder(debugValue);
            for (Index i = 0; i < accessChain.getCount(); ++i)
            {
                HGBCId selector = 0;
                if (auto field = as<IRStructKey>(accessChain[i]))
                    selector = getFieldNameId(field);
                else if (!tryGetDebugValueId(accessChain[i], selector))
                {
                    valueId = 0;
                    break;
                }
                HGBCId selectedType = 0;
                SLANG_RETURN_ON_FAIL(lowerType(
                    tryGetPointedToType(&irBuilder, cast<IRType>(accessTypes[i])),
                    selectedType));
                path.add(selectedType);
                path.add(selector);
            }
        }
        builder.emitDebugValue(variableId, valueId, path);
        return SLANG_OK;
    }

    /// Returns whether an address exists only to select a fragment of an IR debug variable.
    ///
    /// Consider an `IRFieldAddress` rooted at `IRDebugVar` for `value.field`. It is metadata for an
    /// `IRDebugValue`, not storage used by the host program. Its selectors are serialized with
    /// the update, so the address itself must not become an executable HGBC graph value.
    bool isDebugVariableAddress(IRInst* inst)
    {
        if (!as<IRPtrTypeBase>(inst->getDataType()))
            return false;
        return as<IRDebugVar>(getRootAddr(inst)) != nullptr;
    }

    SlangResult processDebugInst(IRInst* inst, bool& outProcessed)
    {
        outProcessed = true;
        switch (inst->getOp())
        {
        case kIROp_DebugLine:
            {
                auto debugLine = as<IRDebugLine>(inst);
                builder.emitDebugLine(
                    getDebugSourceId(debugLine->getSource()),
                    getDebugInteger(debugLine->getLineStart()),
                    getDebugInteger(debugLine->getLineEnd()),
                    getDebugInteger(debugLine->getColStart()),
                    getDebugInteger(debugLine->getColEnd()));
                return SLANG_OK;
            }

        case kIROp_DebugScope:
            {
                auto debugScope = as<IRDebugScope>(inst);
                builder.emitDebugScope(
                    getDebugScopeId(debugScope->getScope()),
                    getDebugInlineSiteId(debugScope->getInlinedAt()));
                return SLANG_OK;
            }

        case kIROp_DebugNoScope:
            builder.emitDebugNoScope();
            return SLANG_OK;

        case kIROp_DebugLexicalBlock:
            return SLANG_OK;

        case kIROp_DebugVar:
            return emitDebugVariable(as<IRDebugVar>(inst));

        case kIROp_DebugValue:
            return emitDebugValue(as<IRDebugValue>(inst));

        case kIROp_DebugInlinedAt:
            getDebugInlineSiteId(inst);
            return SLANG_OK;

        case kIROp_DebugInlinedVariable:
            getDebugVariableId(inst);
            return SLANG_OK;

        default:
            if (isDebugVariableAddress(inst))
                return SLANG_OK;
            outProcessed = false;
            return SLANG_OK;
        }
    }

    /// Resolves an opaque type annotation through the generated opcode table.
    bool tryGetOpaqueTypeOpcode(IRInst* inst, HGBCOpcode& outOpcode)
    {
        return tryGetHGBCOpaqueTypeOpcode(inst, codeGenContext->getTargetCaps(), outOpcode);
    }

    SlangResult checkFunctionType(IRFunc* func)
    {
        SLANG_UNUSED(func);
        return SLANG_OK;
    }

    SlangResult toUInt32LayoutValue(
        IRInst* diagnosticInst,
        IRIntegerValue value,
        const char* description,
        uint32_t& outValue)
    {
        if (value < 0 || value > IRIntegerValue(UINT32_MAX))
        {
            StringBuilder message;
            message << description;
            message << " values larger than 32 bits";
            return fail(diagnosticInst, message.getBuffer());
        }

        outValue = uint32_t(value);
        return SLANG_OK;
    }

    SlangResult getSizeAndAlignmentForType(
        IRInst* diagnosticInst,
        IRType* type,
        IRTypeLayoutRules* rules,
        IRSizeAndAlignment& outSizeAndAlignment)
    {
        SLANG_RETURN_ON_FAIL(Slang::getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            rules,
            type,
            &outSizeAndAlignment));

        if (outSizeAndAlignment.size == IRSizeAndAlignment::kIndeterminateSize)
            return fail(diagnosticInst, "indeterminate graph type layouts");

        return SLANG_OK;
    }

    void tryEmitNaturalTypeLayout(IRType* type, HGBCId typeId)
    {
        IRSizeAndAlignment sizeAndAlignment;
        if (SLANG_FAILED(getNaturalSizeAndAlignment(
                codeGenContext->getTargetReq(),
                type,
                &sizeAndAlignment)))
        {
            return;
        }

        if (sizeAndAlignment.size == IRSizeAndAlignment::kIndeterminateSize)
            return;

        if (sizeAndAlignment.size < 0 || sizeAndAlignment.size > IRIntegerValue(UINT32_MAX))
            return;

        auto stride = sizeAndAlignment.getStride();
        if (stride < 0 || stride > IRIntegerValue(UINT32_MAX))
            return;

        builder.emitTypeSize(
            typeId,
            uint32_t(sizeAndAlignment.size),
            uint32_t(sizeAndAlignment.alignment),
            uint32_t(stride));
    }

    SlangResult getStructuredBufferElementStride(
        IRInst* bufferType,
        IRHLSLStructuredBufferTypeBase* structuredBufferType,
        uint32_t& outStride)
    {
        IRSizeAndAlignment elementSizeAndAlignment;
        SLANG_RETURN_ON_FAIL(getSizeAndAlignmentForType(
            bufferType,
            structuredBufferType->getElementType(),
            getTypeLayoutRuleForBuffer(
                codeGenContext->getTargetProgram(),
                cast<IRType>(bufferType)),
            elementSizeAndAlignment));

        return toUInt32LayoutValue(
            bufferType,
            elementSizeAndAlignment.getStride(),
            "structured-buffer element stride",
            outStride);
    }

    HGBCId getFuncId(IRFunc* func)
    {
        if (auto existingId = funcIds.tryGetValue(func))
            return *existingId;

        HGBCId funcId = builder.allocId();
        funcIds.add(func, funcId);
        return funcId;
    }

    HGBCId getBlockId(IRBlock* block)
    {
        if (auto existingId = blockIds.tryGetValue(block))
            return *existingId;

        HGBCId blockId = builder.allocId();
        blockIds.add(block, blockId);
        return blockId;
    }

    SlangResult getFuncTypeId(IRFunc* func, HGBCId& outTypeId)
    {
        List<HGBCId> paramTypes;
        for (auto param : func->getParams())
        {
            HGBCId paramTypeId = 0;
            SLANG_RETURN_ON_FAIL(lowerType(param->getDataType(), paramTypeId));
            paramTypes.add(paramTypeId);
        }

        HGBCId resultTypeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(func->getResultType(), resultTypeId));

        outTypeId = builder.getFuncTypeId(resultTypeId, paramTypes);
        return SLANG_OK;
    }

    SlangResult getFuncTypeId(IRFuncType* funcType, HGBCId& outTypeId)
    {
        List<HGBCId> paramTypes;
        for (UInt i = 0; i < funcType->getParamCount(); ++i)
        {
            HGBCId paramTypeId = 0;
            SLANG_RETURN_ON_FAIL(lowerType(funcType->getParamType(i), paramTypeId));
            paramTypes.add(paramTypeId);
        }

        HGBCId resultTypeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(funcType->getResultType(), resultTypeId));

        outTypeId = builder.getFuncTypeId(resultTypeId, paramTypes);
        return SLANG_OK;
    }

    void queueFuncForEmission(IRFunc* func)
    {
        if (emittedFuncs.containsKey(func))
            return;

        for (auto pendingFunc : pendingFuncs)
        {
            if (pendingFunc == func)
                return;
        }

        pendingFuncs.add(func);
    }

    /// Serializes an embedded module with the source locations needed by runtime compilation
    /// diagnostics.
    SlangResult serializeEmbeddedModule(Module* module, ComPtr<ISlangBlob>& outBlob)
    {
        SerialContainerUtil::WriteOptions options;
        options.sourceManagerToUseWhenSerializingSourceLocs = codeGenContext->getSourceManager();

        OwnedMemoryStream stream(FileAccess::Write);
        SLANG_RETURN_ON_FAIL(SerialContainerUtil::write(module, options, &stream));

        auto contents = stream.getContents();
        outBlob = RawBlob::create(contents.getBuffer(), size_t(contents.getCount()));
        return outBlob ? SLANG_OK : SLANG_E_OUT_OF_MEMORY;
    }

    SlangResult collectSerializedModules()
    {
        if (serializedModules.getCount() != 0)
            return SLANG_OK;

        auto program = codeGenContext->getProgram();
        SlangResult result = SLANG_OK;

        List<Module*> visitingModules;

        auto isVisiting = [&](Module* module)
        {
            for (auto visitingModule : visitingModules)
            {
                if (visitingModule == module)
                    return true;
            }
            return false;
        };

        auto addModule = [&](Module* module, bool allowBuiltin, auto&& addModuleRef) -> void
        {
            if (SLANG_FAILED(result))
                return;
            if (!module)
                return;
            if (hasSerializedModuleRecord(module))
                return;
            if (isVisiting(module))
                return;

            String name = module->getName() ? module->getName() : "";
            if (!allowBuiltin && isBuiltinModuleName(name))
                return;

            visitingModules.add(module);
            for (auto dependency : module->getModuleDependencies())
            {
                if (dependency == module)
                    continue;

                String dependencyName = dependency->getName() ? dependency->getName() : "";
                if (!allowBuiltin && isBuiltinModuleName(dependencyName))
                    continue;

                addModuleRef(dependency, allowBuiltin, addModuleRef);
            }
            visitingModules.removeLast();

            if (SLANG_FAILED(result))
                return;

            ComPtr<ISlangBlob> blob;
            result = serializeEmbeddedModule(module, blob);
            if (SLANG_FAILED(result))
                return;

            SerializedModuleRecord record;
            record.module = module;
            record.name = name.getLength() ? name : "module";
            record.path = module->getFilePath() ? module->getFilePath() : "";
            record.blob = blob;
            for (auto dependency : module->getModuleDependencies())
            {
                if (dependency == module)
                    continue;

                String dependencyName = dependency->getName() ? dependency->getName() : "";
                if (!allowBuiltin && isBuiltinModuleName(dependencyName))
                    continue;

                if (findSerializedModuleRecord(dependency))
                    record.dependencies.add(dependency);
            }
            serializedModules.add(record);
        };

        program->enumerateModules([&](Module* module) { addModule(module, false, addModule); });
        if (SLANG_FAILED(result))
            return result;

        if (serializedModules.getCount() == 0)
        {
            program->enumerateModules([&](Module* module) { addModule(module, true, addModule); });
            if (SLANG_FAILED(result))
                return result;
        }

        if (serializedModules.getCount() == 0)
        {
            diagnoseRaw(sink, "HGBC target could not find a Slang module to embed\n");
            return SLANG_FAIL;
        }

        return SLANG_OK;
    }

    SlangResult ensureSerializedModuleEmitted(SerializedModuleRecord& moduleRecord)
    {
        if (moduleRecord.moduleId)
            return SLANG_OK;

        List<HGBCId> dependencyIds;
        for (auto dependency : moduleRecord.dependencies)
        {
            auto dependencyRecord = findSerializedModuleRecord(dependency);
            if (!dependencyRecord)
                continue;

            SLANG_RETURN_ON_FAIL(ensureSerializedModuleEmitted(*dependencyRecord));
            dependencyIds.add(dependencyRecord->moduleId);
        }

        HGBCId moduleNameId = builder.getStringLiteralId(moduleRecord.name.getUnownedSlice());
        moduleRecord.binaryLiteralId = builder.emitBinaryLiteral(
            moduleRecord.blob->getBufferPointer(),
            Index(moduleRecord.blob->getBufferSize()));
        moduleRecord.moduleId = builder.allocId();
        builder.emitSlangModule(
            moduleRecord.moduleId,
            moduleNameId,
            moduleRecord.binaryLiteralId,
            dependencyIds);

        builder.emitNameHint(moduleRecord.moduleId, moduleNameId);
        builder.emitDecorate(HGBCDecorationKind::ModuleName, moduleRecord.moduleId, moduleNameId);

        if (moduleRecord.path.getLength() != 0)
        {
            HGBCId pathId = builder.getStringLiteralId(moduleRecord.path.getUnownedSlice());
            builder.emitDecorate(HGBCDecorationKind::OriginalPath, moduleRecord.moduleId, pathId);
        }

        return SLANG_OK;
    }

    /// Embeds every user module needed to reflect graph values at runtime.
    ///
    /// Dispatch emission already embeds the module containing each shader entry point. Graphs
    /// without a dispatch still need their modules for host-side reflection, for example when a
    /// public function accepts `RWStructuredBuffer<MyStruct>`.
    SlangResult emitSerializedModules()
    {
        SLANG_RETURN_ON_FAIL(collectSerializedModules());
        for (auto& moduleRecord : serializedModules)
            SLANG_RETURN_ON_FAIL(ensureSerializedModuleEmitted(moduleRecord));
        return SLANG_OK;
    }

    SerializedModuleRecord* findSerializedModuleRecord(const String& moduleName)
    {
        for (auto& moduleRecord : serializedModules)
        {
            if (moduleRecord.name == moduleName)
                return &moduleRecord;
        }

        return nullptr;
    }

    SlangResult getEmbeddedEntryPointId(
        IRInst* diagnosticInst,
        const String& moduleName,
        const String& entryPointName,
        HGBCId& outEntryPointId)
    {
        SLANG_RETURN_ON_FAIL(collectSerializedModules());

        StringBuilder keyBuilder;
        keyBuilder << moduleName;
        keyBuilder << ":";
        keyBuilder << entryPointName;
        String key = keyBuilder.produceString();

        if (auto existingId = slangEntryPointIds.tryGetValue(key))
        {
            outEntryPointId = *existingId;
            return SLANG_OK;
        }

        auto moduleRecord = findSerializedModuleRecord(moduleName);
        if (!moduleRecord)
            return fail(diagnosticInst, "dispatch callees from modules that were not serialized");

        SLANG_RETURN_ON_FAIL(ensureSerializedModuleEmitted(*moduleRecord));

        HGBCId nameId = builder.getStringLiteralId(entryPointName.getUnownedSlice());
        HGBCId entryPointId = builder.allocId();
        builder.emitSlangEntryPoint(entryPointId, moduleRecord->moduleId, nameId);
        builder.emitNameHint(entryPointId, nameId);

        slangEntryPointIds.add(key, entryPointId);
        outEntryPointId = entryPointId;
        return SLANG_OK;
    }

    SlangResult getEmbeddedSlangEntryPointId(IRSlangEntryPoint* entryPoint, HGBCId& outEntryPointId)
    {
        return getEmbeddedEntryPointId(
            entryPoint,
            String(entryPoint->getModuleNameOperand()->getStringSlice()),
            String(entryPoint->getEntryPointNameOperand()->getStringSlice()),
            outEntryPointId);
    }

    SlangResult getEmbeddedSpecializedSlangEntryPointId(
        IRSpecializedSlangEntryPoint* entryPoint,
        HGBCId& outEntryPointId)
    {
        HGBCId baseEntryPointId = 0;
        SLANG_RETURN_ON_FAIL(getEmbeddedEntryPointId(
            entryPoint,
            String(entryPoint->getModuleNameOperand()->getStringSlice()),
            String(entryPoint->getEntryPointNameOperand()->getStringSlice()),
            baseEntryPointId));

        // A generic dispatch can specialize a kernel from one imported helper module with a
        // type declared in another imported helper module. The base entry point only depends on
        // its defining module, so emitting just that module would leave the runtime without a
        // Slang reflection layout that can resolve the foreign type argument. Emit every
        // serialized module for specialized dispatches; the interpreter composes these modules
        // into the type-lookup context used for specialization.
        for (auto& moduleRecord : serializedModules)
            SLANG_RETURN_ON_FAIL(ensureSerializedModuleEmitted(moduleRecord));

        List<uint32_t> specializationOperands;
        StringBuilder keyBuilder;
        keyBuilder << baseEntryPointId;

        for (UInt i = 0; i < entryPoint->getSpecializationArgCount(); i++)
        {
            String kind(entryPoint->getSpecializationArgKind(i)->getStringSlice());
            HGBCSpecializationArgKind argKind;
            if (kind == "type")
                argKind = HGBCSpecializationArgKind::Type;
            else if (kind == "expr")
                argKind = HGBCSpecializationArgKind::Expr;
            else
                return fail(entryPoint, "unknown dispatch specialization argument kinds");

            auto valueInst = entryPoint->getSpecializationArgValue(i);
            String value;
            if (auto valueString = as<IRStringLit>(valueInst))
            {
                value = valueString->getStringSlice();
            }
            else if (argKind == HGBCSpecializationArgKind::Expr)
            {
                if (auto intValue = as<IRIntLit>(valueInst))
                    value = String(intValue->getValue());
            }

            if (value.getLength() == 0)
            {
                return fail(
                    entryPoint,
                    argKind == HGBCSpecializationArgKind::Type
                        ? "dispatch type specialization arguments require reflection names"
                        : "could not emit dispatch specialization argument value");
            }

            HGBCId valueId = builder.getStringLiteralId(value.getUnownedSlice());
            specializationOperands.add(uint32_t(argKind));
            specializationOperands.add(valueId);

            keyBuilder << ":";
            keyBuilder << kind;
            keyBuilder << ":";
            keyBuilder << valueId;
        }

        if (specializationOperands.getCount() == 0)
        {
            outEntryPointId = baseEntryPointId;
            return SLANG_OK;
        }

        String key = keyBuilder.produceString();
        if (auto existingId = slangEntryPointIds.tryGetValue(key))
        {
            outEntryPointId = *existingId;
            return SLANG_OK;
        }

        HGBCId specializedEntryPointId = builder.allocId();
        builder.emitSpecializeSlangEntryPoint(
            specializedEntryPointId,
            baseEntryPointId,
            specializationOperands);
        slangEntryPointIds.add(key, specializedEntryPointId);
        outEntryPointId = specializedEntryPointId;
        return SLANG_OK;
    }

    SlangResult getDispatchEntryPointId(IRInst* base, HGBCId& outEntryPointId)
    {
        if (auto entryPoint = as<IRSlangEntryPoint>(base))
            return getEmbeddedSlangEntryPointId(entryPoint, outEntryPointId);

        if (auto specializedEntryPoint = as<IRSpecializedSlangEntryPoint>(base))
            return getEmbeddedSpecializedSlangEntryPointId(specializedEntryPoint, outEntryPointId);

        return fail(base, "non-entry-point dispatch callees");
    }

    HGBCId getFieldNameId(IRStructKey* key)
    {
        if (auto existingId = fieldNameIds.tryGetValue(key))
            return *existingId;

        String fieldName;
        if (auto reflectionName = getReflectionNameLiteral(key))
            fieldName = reflectionName->getStringSlice();
        else
        {
            // Anonymous compiler-internal aggregates need only a local field-key identity.
            // lowerAggregateFields rejects this path for source-reflected aggregates, so an
            // internal ID can never be passed off as a source field name during marshalling.
            StringBuilder fieldNameBuilder;
            fieldNameBuilder << "$internalField";
            fieldNameBuilder << anonymousFieldNameCounter++;
            fieldName = fieldNameBuilder.produceString();
        }

        HGBCId fieldNameId = builder.getStringLiteralId(fieldName.getUnownedSlice());
        fieldNameIds.add(key, fieldNameId);
        return fieldNameId;
    }

    template<typename TAggregateType>
    SlangResult lowerAggregateFields(
        TAggregateType* aggregateType,
        List<HGBCId>& outFieldNameAndTypeIds)
    {
        for (auto field : aggregateType->getFields())
        {
            if (getReflectionNameLiteral(aggregateType) &&
                !getReflectionNameLiteral(field->getKey()))
            {
                return fail(field->getKey(), "reflected aggregate fields without semantic names");
            }
            HGBCId fieldNameId = getFieldNameId(field->getKey());

            HGBCId fieldTypeId = 0;
            SLANG_RETURN_ON_FAIL(lowerType(field->getFieldType(), fieldTypeId));

            outFieldNameAndTypeIds.add(fieldNameId);
            outFieldNameAndTypeIds.add(fieldTypeId);
        }
        return SLANG_OK;
    }

    /// Returns the semantic declaration name used for Slang reflection and runtime binding.
    IRStringLit* getReflectionNameLiteral(IRInst* type)
    {
        auto reflectionName = type->findDecoration<IRReflectionNameDecoration>();
        if (!reflectionName)
            return nullptr;

        auto name = as<IRStringLit>(reflectionName->getNameOperand());
        if (!name || name->getStringSlice().getLength() == 0)
            return nullptr;

        return name;
    }

    /// Emits semantic reflection identity separately from optional debug naming metadata.
    void emitAggregateTypeNames(IRInst* type, HGBCId typeId)
    {
        String runtimeName;
        if (getRuntimeTypeMangledName(type, runtimeName))
            builder.emitDecorate(
                HGBCDecorationKind::TypeMangledName,
                typeId,
                builder.getStringLiteralId(runtimeName.getUnownedSlice()));
        if (auto module = type->findDecoration<IRTypeModuleNameDecoration>())
            builder.emitDecorate(
                HGBCDecorationKind::ModuleName,
                typeId,
                builder.getStringLiteralId(module->getNameOperand()->getStringSlice()));
        if (auto reflectionName = getReflectionNameLiteral(type))
        {
            emitDebugLambda(preserveLambdaDebugInfo(type, reflectionName));
            HGBCId reflectionNameId = builder.getStringLiteralId(reflectionName->getStringSlice());
            builder.emitSlangTypeName(typeId, reflectionNameId);
        }

        if (auto debugName = type->findDecoration<IRNameHintDecoration>())
        {
            if (debugName->getName().getLength() != 0)
            {
                HGBCId debugNameId = builder.getStringLiteralId(debugName->getName());
                builder.emitNameHint(typeId, debugNameId);
            }
        }
    }

    SlangResult lowerStructType(IRStructType* structType, HGBCId& outTypeId)
    {
        HGBCOpcode opaqueTypeOpcode;
        if (tryGetOpaqueTypeOpcode(structType, opaqueTypeOpcode))
        {
            // The target annotation is the host representation. Declared fields can describe a
            // different device representation, such as a tracker's bound uint offset. Host field
            // access remains invalid because the emitted opaque type deliberately has no fields.
            outTypeId = builder.getOpaqueTypeId(opaqueTypeOpcode);
            return SLANG_OK;
        }

        if (auto existingId = builder.structTypeIds.tryGetValue(structType))
        {
            outTypeId = *existingId;
            return SLANG_OK;
        }

        outTypeId = builder.beginStructTypeId(structType);
        emitAggregateTypeNames(structType, outTypeId);

        List<HGBCId> fieldNameAndTypeIds;
        SLANG_RETURN_ON_FAIL(lowerAggregateFields(structType, fieldNameAndTypeIds));

        builder.emitStructType(outTypeId, fieldNameAndTypeIds);
        tryEmitNaturalTypeLayout(structType, outTypeId);
        return SLANG_OK;
    }

    SlangResult lowerClassType(IRClassType* classType, HGBCId& outTypeId)
    {
        if (auto existingId = builder.classTypeIds.tryGetValue(classType))
        {
            outTypeId = *existingId;
            return SLANG_OK;
        }

        // Reserve the ID before lowering fields so a class can directly or indirectly contain
        // another reference to itself. The recursive occurrence names the same handle type; only
        // the class payload layout is recursive.
        outTypeId = builder.beginClassTypeId(classType);
        emitAggregateTypeNames(classType, outTypeId);

        List<HGBCId> fieldNameAndTypeIds;
        SLANG_RETURN_ON_FAIL(lowerAggregateFields(classType, fieldNameAndTypeIds));
        builder.emitClassType(outTypeId, fieldNameAndTypeIds);
        return SLANG_OK;
    }

    SlangResult lowerRuntimeArrayType(IRHGVMRuntimeArrayType* arrayType, HGBCId& outTypeId)
    {
        if (auto existingId = builder.runtimeArrayTypeIds.tryGetValue(arrayType))
        {
            outTypeId = *existingId;
            return SLANG_OK;
        }

        // Publish the handle type before lowering its element. A class can contain a
        // RuntimeArray of itself without making either fixed-width value representation recursive.
        outTypeId = builder.beginRuntimeArrayTypeId(arrayType);
        HGBCId elementTypeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(arrayType->getElementType(), elementTypeId));
        builder.emitRuntimeArrayType(outTypeId, elementTypeId);
        return SLANG_OK;
    }

    /// Emits the case-to-payload mapping preserved by HGBC representation lowering.
    ///
    /// Consider Result<Texture, Texture>: success and error remain distinct cases even though
    /// their payload types match. The IR carries explicit case tags, so emission never derives
    /// case identity from a payload's type, size, or position in a deduplicated type set.
    SlangResult lowerTaggedUnionType(IRType* type, HGBCId& outTypeId)
    {
        if (auto existingId = builder.taggedUnionTypeIds.tryGetValue(type))
        {
            outTypeId = *existingId;
            return SLANG_OK;
        }

        SLANG_RELEASE_ASSERT(type->getOperandCount() % 2 == 0);
        outTypeId = builder.allocId();
        // Reserve the fixed-width managed representation before visiting its payload types.
        builder.taggedUnionTypeIds.add(type, outTypeId);
        List<uint32_t> operands;
        operands.add(outTypeId);
        for (UInt i = 0; i < type->getOperandCount(); i += 2)
        {
            auto caseTag = cast<IRIntLit>(type->getOperand(i))->getValue();
            SLANG_RELEASE_ASSERT(caseTag >= 0 && caseTag <= IRIntegerValue(UINT32_MAX));
            HGBCId payloadTypeId = 0;
            SLANG_RETURN_ON_FAIL(lowerType(type->getOperand(i + 1), payloadTypeId));
            operands.add(uint32_t(caseTag));
            operands.add(payloadTypeId);
        }
        builder.emitInst(HGBCOpcode::TaggedUnionType, operands.getBuffer(), operands.getCount());
        emitAggregateTypeNames(type, outTypeId);
        return SLANG_OK;
    }

    /// Returns the ABI element type stored by a resource without changing ordinary graph types.
    ///
    /// Consider `RWStructuredBuffer<Atomic<uint>>`: the resource stores four-byte `uint` elements,
    /// while the embedded shader retains `Atomic<uint>` so its loads and stores remain atomic. We
    /// remove that resource-access wrapper only at resource type and allocation boundaries. A
    /// future standalone host `Atomic<uint>` therefore still reaches `lowerType` as an atomic type
    /// and requires its own HGBC representation rather than silently becoming `uint`.
    static IRType* getResourceStorageElementType(IRType* elementType)
    {
        SLANG_RELEASE_ASSERT(elementType);
        if (auto atomicType = as<IRAtomicType>(elementType))
            return atomicType->getElementType();
        return elementType;
    }

    SlangResult lowerType(IRInst* type, HGBCId& outTypeId)
    {
        switch (type->getOp())
        {
        case kIROp_VoidType:
            outTypeId = builder.getVoidTypeId();
            tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
            return SLANG_OK;

        case kIROp_UIntType:
            outTypeId = builder.getUIntTypeId();
            tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
            return SLANG_OK;

        case kIROp_IntType:
            outTypeId = builder.getIntTypeId();
            tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
            return SLANG_OK;

        case kIROp_BoolType:
            outTypeId = builder.getBoolTypeId();
            tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
            return SLANG_OK;

        case kIROp_FloatType:
            outTypeId = builder.getFloat32TypeId();
            tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
            return SLANG_OK;

        case kIROp_HalfType:
            outTypeId = builder.getFloat16TypeId();
            tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
            return SLANG_OK;

        case kIROp_DoubleType:
            outTypeId = builder.getFloat64TypeId();
            tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
            return SLANG_OK;

        case kIROp_StringType:
            outTypeId = builder.getStringTypeId();
            return SLANG_OK;

        case kIROp_RaytracingAccelerationStructureType:
            outTypeId = builder.getOpaqueTypeId(HGBCOpcode::RaytracingAccelerationStructureType);
            return SLANG_OK;

        case kIROp_StructType:
            return lowerStructType(cast<IRStructType>(type), outTypeId);

        case kIROp_ClassType:
            return lowerClassType(cast<IRClassType>(type), outTypeId);

        case kIROp_HGVMRuntimeArrayType:
            return lowerRuntimeArrayType(cast<IRHGVMRuntimeArrayType>(type), outTypeId);

        case kIROp_HGVMBoxType:
            outTypeId = builder.getOpaqueTypeId(HGBCOpcode::BoxType);
            return SLANG_OK;

        case kIROp_HGVMTaggedUnionType:
            return lowerTaggedUnionType(cast<IRType>(type), outTypeId);

        case kIROp_PtrType:
        case kIROp_OutParamType:
        case kIROp_BorrowInOutParamType:
        case kIROp_RefParamType:
        case kIROp_BorrowInParamType:
            {
                auto parameterType = as<IRPtrTypeBase>(type);
                if (!parameterType)
                    return fail(type, "malformed address type");

                HGBCId valueTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(parameterType->getValueType(), valueTypeId));
                HGBCOpcode opcode = HGBCOpcode::PtrType;
                switch (type->getOp())
                {
                case kIROp_OutParamType:
                    opcode = HGBCOpcode::OutType;
                    break;
                case kIROp_BorrowInOutParamType:
                    opcode = HGBCOpcode::InOutType;
                    break;
                case kIROp_RefParamType:
                    opcode = HGBCOpcode::RefType;
                    break;
                case kIROp_BorrowInParamType:
                    opcode = HGBCOpcode::BorrowInType;
                    break;
                default:
                    break;
                }
                outTypeId = builder.getAddressTypeId(opcode, valueTypeId);
                return SLANG_OK;
            }

        case kIROp_VectorType:
            {
                if (type->getOperandCount() < 2)
                    return fail(type, "malformed vector types");

                HGBCId elementTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(type->getOperand(0), elementTypeId));

                auto elementCount = as<IRIntLit>(type->getOperand(1));
                if (!elementCount)
                    return fail(type, "non-constant vector sizes");

                outTypeId =
                    builder.getVectorTypeId(uint32_t(elementCount->getValue()), elementTypeId);
                tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
                return SLANG_OK;
            }

        case kIROp_MatrixType:
            {
                auto matrixType = as<IRMatrixType>(type);
                if (!matrixType)
                    return fail(type, "malformed matrix types");

                HGBCId elementTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(matrixType->getElementType(), elementTypeId));

                auto rowCount = as<IRIntLit>(matrixType->getRowCount());
                auto columnCount = as<IRIntLit>(matrixType->getColumnCount());
                auto layout = as<IRIntLit>(matrixType->getLayout());
                if (!rowCount || !columnCount || !layout)
                    return fail(type, "non-constant matrix dimensions or layout");

                outTypeId = builder.getMatrixTypeId(
                    uint32_t(rowCount->getValue()),
                    uint32_t(columnCount->getValue()),
                    uint32_t(layout->getValue()),
                    elementTypeId);
                tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
                return SLANG_OK;
            }

        case kIROp_ArrayType:
            {
                auto arrayType = as<IRArrayType>(type);
                if (!arrayType)
                    return fail(type, "malformed fixed-size array types");

                HGBCId elementTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(arrayType->getElementType(), elementTypeId));

                auto elementCount = as<IRIntLit>(arrayType->getElementCount());
                if (!elementCount || elementCount->getValue() < 0 ||
                    elementCount->getValue() > IRIntegerValue(UINT32_MAX))
                {
                    return fail(type, "non-constant or invalid fixed-size array dimensions");
                }

                outTypeId =
                    builder.getArrayTypeId(uint32_t(elementCount->getValue()), elementTypeId);
                tryEmitNaturalTypeLayout(cast<IRType>(type), outTypeId);
                return SLANG_OK;
            }

        case kIROp_DeferType:
            {
                auto deferType = as<IRDeferType>(type);
                if (!deferType)
                    return fail(type, "malformed deferred-value types");

                HGBCId valueTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(deferType->getValueType(), valueTypeId));
                outTypeId = builder.getDeferTypeId(valueTypeId);
                return SLANG_OK;
            }

        case kIROp_HGVMSurfaceType:
            {
                auto surfaceType = as<IRHGVMSurfaceType>(type);
                auto format = surfaceType ? as<IRIntLit>(surfaceType->getFormat()) : nullptr;
                if (!format || format->getValue() < 0 ||
                    format->getValue() > IRIntegerValue(HGBCResourceFormat::D32FloatS8Uint))
                {
                    return fail(type, "non-constant or invalid HGVM surface format");
                }
                outTypeId = builder.getSurfaceTypeId(HGBCResourceFormat(format->getValue()));
                return SLANG_OK;
            }

        case kIROp_HGVMRenderTargetType:
            {
                auto renderTargetType = as<IRHGVMRenderTargetType>(type);
                auto format =
                    renderTargetType ? as<IRIntLit>(renderTargetType->getFormat()) : nullptr;
                if (!renderTargetType || !format || format->getValue() < 0 ||
                    format->getValue() > IRIntegerValue(HGBCResourceFormat::D32FloatS8Uint))
                {
                    return fail(type, "non-constant or invalid HGVM render-target format");
                }

                HGBCId elementTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(renderTargetType->getElementType(), elementTypeId));
                outTypeId = builder.getRenderTarget2DTypeId(
                    elementTypeId,
                    HGBCResourceFormat(format->getValue()));
                return SLANG_OK;
            }

        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
        case kIROp_HLSLRasterizerOrderedStructuredBufferType:
            {
                auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type);
                if (!structuredBufferType)
                    return fail(type, "malformed structured buffer types");

                SLANG_RETURN_ON_FAIL(
                    requireSimpleStructuredBufferLayout(type, structuredBufferType));

                IRType* elementType =
                    getResourceStorageElementType(structuredBufferType->getElementType());

                HGBCId elementTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(elementType, elementTypeId));

                uint32_t elementStride = 0;
                SLANG_RETURN_ON_FAIL(
                    getStructuredBufferElementStride(type, structuredBufferType, elementStride));

                if (type->getOp() == kIROp_HLSLStructuredBufferType)
                    outTypeId = builder.getStructuredBufferTypeId(elementTypeId, elementStride);
                else
                    outTypeId = builder.getRWStructuredBufferTypeId(elementTypeId, elementStride);
                return SLANG_OK;
            }

        case kIROp_DescriptorHandleType:
            {
                auto handleType = as<IRDescriptorHandleType>(type);
                if (!handleType)
                    return fail(type, "malformed descriptor-handle types");

                HGBCId resourceTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(handleType->getResourceType(), resourceTypeId));
                HGBCId reflectionNameId = 0;
                if (auto reflectionName = getReflectionNameLiteral(type))
                    reflectionNameId = builder.getStringLiteralId(reflectionName->getStringSlice());
                outTypeId =
                    builder.getDescriptorHandleTypeId(handleType, resourceTypeId, reflectionNameId);
                return SLANG_OK;
            }

        case kIROp_TextureType:
            {
                auto textureType = as<IRTextureTypeBase>(type);
                if (!textureType)
                    return fail(type, "malformed texture types");

                if (textureType->getShape() != SLANG_TEXTURE_2D || textureType->isArray() ||
                    textureType->isMultisample())
                {
                    return fail(type, "non-2D render target texture types");
                }

                HGBCTextureAccess access;
                switch (textureType->getAccess())
                {
                case SLANG_RESOURCE_ACCESS_READ:
                    access = HGBCTextureAccess::ReadOnly;
                    break;
                case SLANG_RESOURCE_ACCESS_READ_WRITE:
                    access = HGBCTextureAccess::ReadWrite;
                    break;
                case SLANG_RESOURCE_ACCESS_WRITE:
                    access = HGBCTextureAccess::WriteOnly;
                    break;
                default:
                    return fail(type, "unsupported 2D texture access");
                }

                auto formatInst = as<IRIntLit>(textureType->getFormatInst());
                if (!formatInst || formatInst->getValue() < 0 ||
                    formatInst->getValue() > IRIntegerValue(HGBCResourceFormat::D32FloatS8Uint))
                {
                    return fail(type, "non-constant or invalid 2D texture format");
                }

                HGBCId elementTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(textureType->getElementType(), elementTypeId));
                outTypeId = builder.getTexture2DTypeId(
                    elementTypeId,
                    access,
                    HGBCResourceFormat(formatInst->getValue()));
                return SLANG_OK;
            }

        default:
            break;
        }

        StringBuilder message;
        message << "graph value type ";
        message << getIROpInfo(type->getOp()).name;
        return fail(type, message.getBuffer());
    }

    SlangResult emitVarInst(IRVar* var, HGBCId& outValueId)
    {
        HGBCId typeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(var->getDataType(), typeId));

        outValueId = builder.allocId();
        builder.emitVar(typeId, outValueId);
        registerValueId(var, outValueId);
        return SLANG_OK;
    }

    SlangResult emitLoadInst(IRLoad* load, HGBCId& outValueId)
    {
        HGBCId typeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(load->getDataType(), typeId));

        HGBCId ptrId = 0;
        SLANG_RETURN_ON_FAIL(emitInstResultId(load->getPtr(), ptrId));

        outValueId = builder.allocId();
        builder.emitLoad(typeId, outValueId, ptrId);
        registerValueId(load, outValueId);
        return SLANG_OK;
    }

    SlangResult emitStoreInst(IRStore* store)
    {
        HGBCId ptrId = 0;
        SLANG_RETURN_ON_FAIL(emitInstResultId(store->getPtr(), ptrId));

        HGBCId valueId = 0;
        SLANG_RETURN_ON_FAIL(emitInstResultId(store->getVal(), valueId));

        builder.emitStore(ptrId, valueId);
        return SLANG_OK;
    }

    SlangResult emitFieldAddressInst(IRFieldAddress* fieldAddress, HGBCId& outValueId)
    {
        auto fieldKey = as<IRStructKey>(fieldAddress->getField());
        if (!fieldKey)
            return fail(fieldAddress, "non-struct field addresses");

        HGBCId typeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(fieldAddress->getDataType(), typeId));

        HGBCId baseId = 0;
        SLANG_RETURN_ON_FAIL(emitInstResultId(fieldAddress->getBase(), baseId));

        outValueId = builder.allocId();
        builder.emitFieldAddress(typeId, outValueId, baseId, getFieldNameId(fieldKey));
        registerValueId(fieldAddress, outValueId);
        return SLANG_OK;
    }

    /// Converts a valid IR integer index to HGBC's unsigned index representation.
    /// For example, an unoptimized `for (int i = 0; i < N; ++i) a[i] = value` keeps a
    /// signed loop parameter. Both address and value indexing must convert it at the wire boundary.
    SlangResult emitIndexValue(IRInst* index, HGBCId& outId)
    {
        SLANG_RETURN_ON_FAIL(emitInstResultId(index, outId));
        auto type = index->getDataType();
        if (type->getOp() == kIROp_UIntType)
            return SLANG_OK;
        if (type->getOp() != kIROp_IntType)
            return fail(index, "non-32-bit integer indices");
        HGBCId sourceType = 0;
        SLANG_RETURN_ON_FAIL(lowerType(type, sourceType));
        HGBCId converted = builder.allocId();
        builder.emitScalarTypeCast(converted, sourceType, builder.getUIntTypeId(), outId);
        outId = converted;
        return SLANG_OK;
    }

    SlangResult emitElementAddressInst(
        IRInst* elementAddress,
        IRInst* base,
        IRInst* index,
        HGBCId& outValueId)
    {
        HGBCId typeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(elementAddress->getDataType(), typeId));

        HGBCId baseId = 0;
        SLANG_RETURN_ON_FAIL(emitInstResultId(base, baseId));

        HGBCId indexId = 0;
        SLANG_RETURN_ON_FAIL(emitIndexValue(index, indexId));

        outValueId = builder.allocId();
        builder.emitElementAddress(typeId, outValueId, baseId, indexId);
        registerValueId(elementAddress, outValueId);
        return SLANG_OK;
    }

    SlangResult emitElementAddressInst(IRGetElementPtr* elementAddress, HGBCId& outValueId)
    {
        return emitElementAddressInst(
            elementAddress,
            elementAddress->getBase(),
            elementAddress->getIndex(),
            outValueId);
    }

    void registerValueId(IRInst* inst, HGBCId valueId) { valueIds.add(inst, valueId); }

    // Returns the scalar leaf of a shader numeric or Boolean value type.
    //
    // Consider `float3x2`: this returns its `float` element type, while `MyStruct` returns null.
    // HGBC operation selection can consequently use one path for scalar, vector, and matrix values
    // without confusing their logical shape with their arithmetic category.
    IRType* tryGetNumericScalarType(IRInst* type)
    {
        if (!type)
            return nullptr;
        switch (type->getOp())
        {
        case kIROp_BoolType:
        case kIROp_IntType:
        case kIROp_UIntType:
        case kIROp_HalfType:
        case kIROp_FloatType:
        case kIROp_DoubleType:
            return as<IRType>(type);
        case kIROp_VectorType:
            return cast<IRVectorType>(type)->getElementType();
        case kIROp_MatrixType:
            return cast<IRMatrixType>(type)->getElementType();
        default:
            return nullptr;
        }
    }

    /// Produces an operand that belongs to serialized HGBC but is intentionally hidden from
    /// source-level `hgbc_asm`.
    SlangResult getImplicitHGBCAsmOperand(
        IRHGBCAsmInst* asmInst,
        HGBCOpcode opcode,
        const HGBCOperandInfo& operandInfo,
        const List<uint32_t>& sourceOperands,
        uint32_t& outOperand)
    {
        SLANG_ASSERT(operandInfo.isImplicit);

        switch (opcode)
        {
        case HGBCOpcode::AllocateResource:
            if (UnownedStringSlice(operandInfo.name) == UnownedStringSlice("layout"))
            {
                // Consider `allocateResource<RWStructuredBuffer<Particle>, Particle>(...)`. The
                // element type is source operand seven; the compiler turns it into a device-memory
                // layout handle before serializing the instruction.
                const HGBCId elementTypeId = sourceOperands[7];
                const HGBCResourceKind kind = HGBCResourceKind(sourceOperands[2]);
                if (kind == HGBCResourceKind::Buffer)
                {
                    SLANG_RETURN_ON_FAIL(requireDeviceMemorySlangTypeName(asmInst, elementTypeId));
                }
                outOperand = builder.getTypeLayoutId(
                    elementTypeId,
                    HGBCLayoutDomain::DeviceMemory,
                    kind == HGBCResourceKind::Buffer ? HGBCLayoutUsage::StructuredBufferElement
                                                     : HGBCLayoutUsage::Value);
                return SLANG_OK;
            }
            break;
        case HGBCOpcode::ReadBackBufferElement:
        case HGBCOpcode::AsyncReadBackBufferElement:
            if (UnownedStringSlice(operandInfo.name) == UnownedStringSlice("converter"))
            {
                // The result of an async read is `Defer<Particle>`, so the result type itself does
                // not identify the copied value. The structured-buffer wrapper in source operand
                // two does.
                const HGBCId elementTypeId =
                    builder.getStructuredBufferElementTypeId(sourceOperands[2]);
                SLANG_RETURN_ON_FAIL(requireDeviceMemorySlangTypeName(asmInst, elementTypeId));
                const HGBCId deviceLayoutId = builder.getTypeLayoutId(
                    elementTypeId,
                    HGBCLayoutDomain::DeviceMemory,
                    HGBCLayoutUsage::StructuredBufferElement);
                const HGBCId hostLayoutId = builder.getTypeLayoutId(
                    elementTypeId,
                    HGBCLayoutDomain::Host,
                    HGBCLayoutUsage::Value);
                outOperand = builder.getLayoutConverterId(deviceLayoutId, hostLayoutId);
                return SLANG_OK;
            }

        case HGBCOpcode::CopyArrayRange:
            if (UnownedStringSlice(operandInfo.name) == UnownedStringSlice("converter"))
            {
                HGBCId destinationElementTypeId = 0;
                HGBCLayoutDomain destinationDomain = HGBCLayoutDomain::Host;
                HGBCLayoutUsage destinationUsage = HGBCLayoutUsage::Value;
                HGBCId sourceElementTypeId = 0;
                HGBCLayoutDomain sourceDomain = HGBCLayoutDomain::Host;
                HGBCLayoutUsage sourceUsage = HGBCLayoutUsage::Value;
                if (!builder.tryGetArrayCopyEndpointLayout(
                        sourceOperands[0],
                        destinationElementTypeId,
                        destinationDomain,
                        destinationUsage) ||
                    !builder.tryGetArrayCopyEndpointLayout(
                        sourceOperands[1],
                        sourceElementTypeId,
                        sourceDomain,
                        sourceUsage) ||
                    destinationElementTypeId != sourceElementTypeId)
                {
                    return fail(asmInst, "array-copy endpoint types do not share an element type");
                }

                if (sourceDomain == HGBCLayoutDomain::DeviceMemory)
                {
                    SLANG_RETURN_ON_FAIL(
                        requireDeviceMemorySlangTypeName(asmInst, sourceElementTypeId));
                }
                if (destinationDomain == HGBCLayoutDomain::DeviceMemory)
                {
                    SLANG_RETURN_ON_FAIL(
                        requireDeviceMemorySlangTypeName(asmInst, destinationElementTypeId));
                }
                HGBCId sourceLayoutId =
                    builder.getTypeLayoutId(sourceElementTypeId, sourceDomain, sourceUsage);
                HGBCId destinationLayoutId = builder.getTypeLayoutId(
                    destinationElementTypeId,
                    destinationDomain,
                    destinationUsage);
                outOperand = builder.getLayoutConverterId(sourceLayoutId, destinationLayoutId);
                return SLANG_OK;
            }
            break;
        default:
            break;
        }

        return fail(asmInst, "unimplemented implicit hgbc_asm operands");
    }

    /// Converts structured HGBC assembly IR into ordinary descriptor-validated records.
    SlangResult emitHGBCAsm(IRHGBCAsm* asmBlock, HGBCId& outValueId)
    {
        Dictionary<String, HGBCId> localIds;
        HGBCId resultId = 0;

        for (IRHGBCAsmInst* asmInst : asmBlock->getInsts())
        {
            HGBCOpcode opcode = HGBCOpcode(asmInst->getOpcode());
            const HGBCOpcodeInfo* opcodeInfo = getHGBCOpcodeInfo(opcode);
            if (!opcodeInfo || !hasHGBCOpcodeFlag(opcode, HGBCOpcodeFlags::InlineAsm))
                return fail(asmInst, "invalid or compiler-owned hgbc_asm opcodes");

            auto irOperands = asmInst->getHGBCOperands();
            const Index inlineFixedOperandCount =
                Index(getHGBCInlineAsmFixedOperandCount(*opcodeInfo));
            bool hasTypedValuePack =
                irOperands.getCount() == inlineFixedOperandCount + 1 &&
                opcodeInfo->repeatedOperandCount == 2 &&
                opcodeInfo->operands[opcodeInfo->fixedOperandCount].kind ==
                    HGBCOperandKind::TypeId &&
                opcodeInfo->operands[opcodeInfo->fixedOperandCount + 1].kind ==
                    HGBCOperandKind::Id &&
                irOperands[irOperands.getCount() - 1]->getOp() == kIROp_HGBCAsmOperandValuePack;
            if (!hasTypedValuePack &&
                !isValidHGBCInlineAsmOperandCount(*opcodeInfo, size_t(irOperands.getCount())))
                return fail(asmInst, "malformed hgbc_asm operand lists");

            List<uint32_t> sourceOperands;
            for (Index operandIndex = 0; operandIndex < irOperands.getCount(); ++operandIndex)
            {
                IRHGBCAsmOperand* operand = irOperands[operandIndex];
                const HGBCOperandInfo* operandInfo =
                    getHGBCInlineAsmOperandInfo(*opcodeInfo, size_t(operandIndex));
                SLANG_RELEASE_ASSERT(operandInfo);

                uint32_t encodedOperand = 0;
                switch (operand->getOp())
                {
                case kIROp_HGBCAsmOperandLiteral:
                    encodedOperand = uint32_t(cast<IRIntLit>(operand->getValue())->getValue());
                    break;

                case kIROp_HGBCAsmOperandValue:
                    if (operandInfo->kind == HGBCOperandKind::Enum ||
                        operandInfo->kind == HGBCOperandKind::UInt ||
                        operandInfo->kind == HGBCOperandKind::ByteCount)
                    {
                        // Generic immediate expressions must specialize to literal words. For
                        // example, an allocation's `$MipCount` must be a concrete count here.
                        // Encoding a runtime value ID instead would change the wire meaning.
                        auto literal = as<IRIntLit>(operand->getValue());
                        if (!literal || literal->getValue() < 0 ||
                            literal->getValue() > IRIntegerValue(UINT32_MAX))
                        {
                            return fail(
                                asmInst,
                                "non-constant or out-of-range hgbc_asm immediate operands");
                        }
                        encodedOperand = uint32_t(literal->getValue());
                        if (operandInfo->kind == HGBCOperandKind::Enum &&
                            !getHGBCEnumValueName(operandInfo->enumKind, encodedOperand))
                            return fail(asmInst, "invalid specialized hgbc_asm enum operands");
                    }
                    else
                    {
                        SLANG_RETURN_ON_FAIL(emitInstResultId(operand->getValue(), encodedOperand));
                    }
                    break;

                case kIROp_HGBCAsmOperandType:
                    {
                        IRType* operandType = cast<IRType>(operand->getValue());
                        if (opcode == HGBCOpcode::AllocateResource &&
                            UnownedStringSlice(operandInfo->name) ==
                                UnownedStringSlice("elementType"))
                        {
                            operandType = getResourceStorageElementType(operandType);
                        }
                        SLANG_RETURN_ON_FAIL(lowerType(operandType, encodedOperand));
                    }
                    break;

                case kIROp_HGBCAsmOperandValuePack:
                    {
                        if (operand->getFullType()->getOp() != kIROp_VoidType)
                            return fail(asmInst, "unspecialized hgbc_asm value packs");

                        for (UInt elementIndex = 0; elementIndex < operand->getOperandCount();
                             ++elementIndex)
                        {
                            IRInst* element = operand->getOperand(elementIndex);
                            if (hasTypedValuePack)
                            {
                                HGBCId elementTypeId = 0;
                                SLANG_RETURN_ON_FAIL(
                                    lowerType(element->getDataType(), elementTypeId));
                                sourceOperands.add(elementTypeId);
                            }
                            HGBCId elementId = 0;
                            SLANG_RETURN_ON_FAIL(emitInstResultId(element, elementId));
                            sourceOperands.add(elementId);
                        }
                        continue;
                    }

                case kIROp_HGBCAsmOperandId:
                    {
                        String name =
                            String(cast<IRStringLit>(operand->getValue())->getStringSlice());
                        if (operandInfo->kind == HGBCOperandKind::ResultId)
                        {
                            if (localIds.containsKey(name))
                                return fail(asmInst, "duplicate hgbc_asm local ids");
                            encodedOperand = builder.allocId();
                            localIds.add(name, encodedOperand);
                        }
                        else
                        {
                            auto localId = localIds.tryGetValue(name);
                            if (!localId)
                                return fail(asmInst, "undefined hgbc_asm local ids");
                            encodedOperand = *localId;
                        }
                        break;
                    }

                case kIROp_HGBCAsmOperandResult:
                    if (operandInfo->kind != HGBCOperandKind::ResultId || resultId)
                        return fail(asmInst, "misplaced hgbc_asm result markers");
                    resultId = builder.allocId();
                    encodedOperand = resultId;
                    break;

                default:
                    return fail(asmInst, "unknown hgbc_asm operand wrappers");
                }

                sourceOperands.add(encodedOperand);
            }

            // Materialize the wire record by inserting compiler-owned operands into their schema
            // positions. Repeated source operands follow the fixed wire operands unchanged.
            List<uint32_t> wireOperands;
            Index sourceIndex = 0;
            for (Index wireIndex = 0; wireIndex < opcodeInfo->fixedOperandCount; ++wireIndex)
            {
                const HGBCOperandInfo& wireOperand = opcodeInfo->operands[wireIndex];
                if (wireOperand.isImplicit)
                {
                    uint32_t encodedOperand = 0;
                    SLANG_RETURN_ON_FAIL(getImplicitHGBCAsmOperand(
                        asmInst,
                        opcode,
                        wireOperand,
                        sourceOperands,
                        encodedOperand));
                    wireOperands.add(encodedOperand);
                }
                else
                {
                    // A direct draw omits the optional index-buffer operand. The semantic
                    // checker has already validated the source arity; only required operands
                    // must exist when materializing the fixed part of the wire record.
                    if (sourceIndex == sourceOperands.getCount())
                    {
                        SLANG_RELEASE_ASSERT(wireIndex >= opcodeInfo->requiredOperandCount);
                        break;
                    }
                    SLANG_ASSERT(sourceIndex < sourceOperands.getCount());
                    wireOperands.add(sourceOperands[sourceIndex++]);
                }
            }
            while (sourceIndex < sourceOperands.getCount())
                wireOperands.add(sourceOperands[sourceIndex++]);

            builder.emitInst(opcode, wireOperands.getBuffer(), wireOperands.getCount());
        }

        if (resultId)
        {
            registerValueId(asmBlock, resultId);
            outValueId = resultId;
            return SLANG_OK;
        }

        if (asmBlock->getDataType()->getOp() != kIROp_VoidType)
            return fail(asmBlock, "value-producing hgbc_asm blocks without a result marker");

        outValueId = 0;
        return SLANG_OK;
    }

    /// Reports whether one IR type has a direct scalar representation in HGBC.
    ///
    /// For example, `bool`, `int`, `uint`, and `float` can participate in
    /// `OpScalarTypeCast`. A vector does not reach this operation because it requires a separate
    /// lane-wise cast representation rather than pretending the aggregate is one scalar.
    static bool isBasicScalarCastType(IRType* type)
    {
        if (!type)
            return false;
        switch (type->getOp())
        {
        case kIROp_BoolType:
        case kIROp_IntType:
        case kIROp_UIntType:
        case kIROp_HalfType:
        case kIROp_FloatType:
        case kIROp_DoubleType:
            return true;
        default:
            return false;
        }
    }

    /// Emits one explicitly typed HGBC scalar cast from any basic scalar-cast IR opcode.
    ///
    /// Consider `float(i)` and `int(f)`: their IR opcodes differ, but both become
    /// `OpScalarTypeCast` with source and destination type IDs. A matrix `floatCast` that only
    /// changes row/column-major storage remains a no-op because HGBC matrices store logical lanes.
    SlangResult emitScalarTypeCastInst(IRInst* inst, HGBCId& outValueId)
    {
        if (inst->getOperandCount() != 1)
            return fail(inst, "malformed scalar cast");

        IRType* destinationType = inst->getDataType();
        IRType* sourceType = inst->getOperand(0)->getDataType();
        if (inst->getOp() == kIROp_FloatCast && !isBasicScalarCastType(destinationType))
        {
            auto destinationMatrixType = as<IRMatrixType>(destinationType);
            auto sourceMatrixType = as<IRMatrixType>(sourceType);
            auto destinationRowCount = destinationMatrixType
                                           ? as<IRIntLit>(destinationMatrixType->getRowCount())
                                           : nullptr;
            auto destinationColumnCount =
                destinationMatrixType ? as<IRIntLit>(destinationMatrixType->getColumnCount())
                                      : nullptr;
            auto sourceRowCount =
                sourceMatrixType ? as<IRIntLit>(sourceMatrixType->getRowCount()) : nullptr;
            auto sourceColumnCount =
                sourceMatrixType ? as<IRIntLit>(sourceMatrixType->getColumnCount()) : nullptr;
            if (!destinationMatrixType || !sourceMatrixType || !destinationRowCount ||
                !destinationColumnCount || !sourceRowCount || !sourceColumnCount ||
                destinationMatrixType->getElementType() != sourceMatrixType->getElementType() ||
                destinationRowCount->getValue() != sourceRowCount->getValue() ||
                destinationColumnCount->getValue() != sourceColumnCount->getValue())
            {
                return fail(inst, "non-scalar floating-point cast");
            }
            return emitInstResultId(inst->getOperand(0), outValueId);
        }

        if (!isBasicScalarCastType(sourceType) || !isBasicScalarCastType(destinationType))
            return fail(inst, "cast between non-scalar HGBC values");

        HGBCId sourceTypeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(sourceType, sourceTypeId));
        HGBCId destinationTypeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(destinationType, destinationTypeId));
        HGBCId valueId = 0;
        SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), valueId));

        outValueId = builder.allocId();
        builder.emitScalarTypeCast(outValueId, sourceTypeId, destinationTypeId, valueId);
        registerValueId(inst, outValueId);
        return SLANG_OK;
    }

    /// Emits typed owned-erasure operations without introducing raw payload reinterpretation.
    ///
    /// OpBox names the concrete source type so staging can build its cleanup recipe. Extraction
    /// and conversion retain their semantic case operands for validation instead of treating
    /// them as ordinary value IDs or permitting a byte-layout-based conversion.
    SlangResult emitOwnedErasureInst(IRInst* inst, HGBCId& outValueId)
    {
        HGBCOpcode opcode;
        UInt firstLiteralOperand = inst->getOperandCount();
        switch (inst->getOp())
        {
        case kIROp_HGVMBox:
            opcode = HGBCOpcode::Box;
            break;
        case kIROp_HGVMUnbox:
            opcode = HGBCOpcode::Unbox;
            break;
        case kIROp_HGVMMakeTaggedUnion:
            opcode = HGBCOpcode::MakeTaggedUnion;
            break;
        case kIROp_HGVMGetTaggedUnionCase:
            opcode = HGBCOpcode::GetTaggedUnionCase;
            break;
        case kIROp_HGVMGetTaggedUnionPayload:
            opcode = HGBCOpcode::GetTaggedUnionPayload;
            break;
        case kIROp_HGVMExtractTaggedUnion:
            opcode = HGBCOpcode::ExtractTaggedUnion;
            firstLiteralOperand = 1;
            break;
        case kIROp_HGVMConvertTaggedUnion:
            opcode = HGBCOpcode::ConvertTaggedUnion;
            firstLiteralOperand = 1;
            break;
        default:
            SLANG_UNEXPECTED("Expected an HGBC owned-erasure operation");
        }

        HGBCId typeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));
        outValueId = builder.allocId();
        List<uint32_t> operands;
        operands.add(typeId);
        operands.add(outValueId);
        if (opcode == HGBCOpcode::Box)
        {
            HGBCId payloadTypeId = 0;
            SLANG_RETURN_ON_FAIL(lowerType(inst->getOperand(0)->getDataType(), payloadTypeId));
            operands.add(payloadTypeId);
        }
        for (UInt i = 0; i < inst->getOperandCount(); ++i)
        {
            if (i >= firstLiteralOperand)
            {
                auto caseTag = cast<IRIntLit>(inst->getOperand(i))->getValue();
                SLANG_RELEASE_ASSERT(caseTag >= 0 && caseTag <= IRIntegerValue(UINT32_MAX));
                operands.add(uint32_t(caseTag));
            }
            else
            {
                HGBCId valueId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(i), valueId));
                operands.add(valueId);
            }
        }
        builder.emitInst(opcode, operands.getBuffer(), operands.getCount());
        registerValueId(inst, outValueId);
        return SLANG_OK;
    }

    // Emits scalar constructor operands in lane order. Slang IR permits mixed inputs such as
    // uint3(uint2(x, y), z) and matrices built from vector rows, while HGBC's MakeVector and
    // MakeMatrix instructions require one scalar value ID per element.
    SlangResult emitScalarConstructorOperands(IRInst* inst, List<HGBCId>& elementIds)
    {
        for (UInt i = 0; i < inst->getOperandCount(); ++i)
        {
            IRInst* operand = inst->getOperand(i);
            HGBCId elementId = 0;
            SLANG_RETURN_ON_FAIL(emitInstResultId(operand, elementId));

            auto vectorType = as<IRVectorType>(operand->getDataType());
            if (!vectorType)
            {
                elementIds.add(elementId);
                continue;
            }

            auto vectorElementCount = as<IRIntLit>(vectorType->getElementCount());
            if (!vectorElementCount || vectorElementCount->getValue() < 0 ||
                vectorElementCount->getValue() > IRIntegerValue(UINT32_MAX))
            {
                return fail(inst, "constructor operands with invalid vector dimensions");
            }

            HGBCId elementTypeId = 0;
            SLANG_RETURN_ON_FAIL(lowerType(vectorType->getElementType(), elementTypeId));
            for (uint32_t elementIndex = 0; elementIndex < uint32_t(vectorElementCount->getValue());
                 ++elementIndex)
            {
                HGBCId indexId = builder.getIntLiteralId(builder.getUIntTypeId(), elementIndex);
                HGBCId extractedId = builder.allocId();
                builder.emitExtractElement(elementTypeId, extractedId, elementId, indexId);
                elementIds.add(extractedId);
            }
        }
        return SLANG_OK;
    }

    SlangResult emitInstResultId(IRInst* inst, HGBCId& outValueId)
    {
        if (auto valueId = valueIds.tryGetValue(inst))
        {
            outValueId = *valueId;
            return SLANG_OK;
        }

        switch (inst->getOp())
        {
        case kIROp_HGBCAsm:
            return emitHGBCAsm(cast<IRHGBCAsm>(inst), outValueId);

        case kIROp_HGVMBox:
        case kIROp_HGVMUnbox:
        case kIROp_HGVMMakeTaggedUnion:
        case kIROp_HGVMGetTaggedUnionCase:
        case kIROp_HGVMGetTaggedUnionPayload:
        case kIROp_HGVMExtractTaggedUnion:
        case kIROp_HGVMConvertTaggedUnion:
            return emitOwnedErasureInst(inst, outValueId);

        case kIROp_StringLit:
            {
                auto stringLit = cast<IRStringLit>(inst);
                outValueId = builder.getStringLiteralId(stringLit->getStringSlice());
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_IntLit:
            {
                auto intLit = cast<IRIntLit>(inst);
                outValueId =
                    builder.getIntLiteralId(builder.getUIntTypeId(), uint32_t(intLit->getValue()));
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_BoolLit:
            {
                auto boolLit = cast<IRBoolLit>(inst);
                outValueId = builder.getBoolLiteralId(boolLit->getValue());
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_FloatLit:
            {
                IROp typeOp = inst->getDataType()->getOp();
                auto floatLit = cast<IRFloatLit>(inst);
                switch (typeOp)
                {
                case kIROp_HalfType:
                    outValueId = builder.getFloat16LiteralId(float(floatLit->getValue()));
                    break;
                case kIROp_FloatType:
                    outValueId = builder.getFloatLiteralId(float(floatLit->getValue()));
                    break;
                case kIROp_DoubleType:
                    outValueId = builder.getFloat64LiteralId(floatLit->getValue());
                    break;
                default:
                    return fail(inst, "unsupported floating-point constants");
                }
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_DefaultConstruct:
            {
                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));
                outValueId = builder.getNullLiteralId(typeId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_CastIntToFloat:
        case kIROp_CastFloatToInt:
            return emitScalarTypeCastInst(inst, outValueId);

        case kIROp_Var:
            return emitVarInst(cast<IRVar>(inst), outValueId);

        case kIROp_Load:
            return emitLoadInst(cast<IRLoad>(inst), outValueId);

        case kIROp_CastDescriptorHandleToResource:
            {
                if (inst->getOperandCount() != 1)
                    return fail(inst, "malformed descriptor-handle dereference");

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));
                HGBCId handleId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), handleId));
                outValueId = builder.allocId();
                builder.emitDescriptorHandleToResource(typeId, outValueId, handleId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_AllocObj:
            {
                if (inst->getOperandCount() != 0 || !as<IRClassType>(inst->getDataType()))
                    return fail(inst, "malformed class-object allocation");

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));
                outValueId = builder.allocId();
                builder.emitAllocateObject(typeId, outValueId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_FieldAddress:
            return emitFieldAddressInst(cast<IRFieldAddress>(inst), outValueId);

        case kIROp_GetElementPtr:
            return emitElementAddressInst(cast<IRGetElementPtr>(inst), outValueId);

        case kIROp_HGVMRuntimeArrayGetElementPtr:
            {
                auto elementAddress = cast<IRHGVMRuntimeArrayGetElementPtr>(inst);
                return emitElementAddressInst(
                    elementAddress,
                    elementAddress->getBase(),
                    elementAddress->getIndex(),
                    outValueId);
            }

        case kIROp_StructuredBufferGetDimensions:
            {
                if (inst->getOperandCount() != 1)
                    return fail(inst, "malformed structured buffer GetDimensions instructions");

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                HGBCId bufferTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getOperand(0)->getDataType(), bufferTypeId));

                HGBCId bufferId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), bufferId));

                outValueId = builder.allocId();
                builder
                    .emitStructuredBufferGetDimensions(typeId, outValueId, bufferTypeId, bufferId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_TextureGetDimensions:
            {
                if (inst->getOperandCount() != 1)
                    return fail(inst, "malformed texture GetDimensions instructions");

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                HGBCId textureId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), textureId));

                outValueId = builder.allocId();
                builder.emitTextureGetDimensions(typeId, outValueId, textureId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_Swizzle:
            {
                auto swizzle = cast<IRSwizzle>(inst);
                HGBCId baseId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(swizzle->getBase(), baseId));

                auto baseType = as<IRVectorType>(swizzle->getBase()->getDataType());
                if (!baseType)
                    return fail(inst, "swizzles of non-vector values");
                HGBCId elementTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(baseType->getElementType(), elementTypeId));

                List<HGBCId> elementIds;
                if (swizzle->getElementCount() == 0)
                    return fail(inst, "empty vector swizzles");
                for (UInt element = 0; element < swizzle->getElementCount(); ++element)
                {
                    auto elementIndex = as<IRIntLit>(swizzle->getElementIndex(element));
                    if (!elementIndex)
                        return fail(inst, "non-constant vector swizzles");
                    HGBCId indexId = builder.getIntLiteralId(
                        builder.getUIntTypeId(),
                        uint32_t(elementIndex->getValue()));
                    HGBCId elementId = builder.allocId();
                    builder.emitExtractElement(elementTypeId, elementId, baseId, indexId);
                    elementIds.add(elementId);
                }

                if (elementIds.getCount() == 1)
                {
                    outValueId = elementIds[0];
                }
                else
                {
                    HGBCId typeId = 0;
                    SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));
                    outValueId = builder.allocId();
                    builder.emitMakeVector(typeId, outValueId, elementIds);
                }
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_GetElement:
            {
                if (inst->getOperandCount() != 2)
                    return fail(inst, "malformed element extracts");

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                HGBCId baseId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), baseId));

                HGBCId indexId = 0;
                SLANG_RETURN_ON_FAIL(emitIndexValue(inst->getOperand(1), indexId));

                outValueId = builder.allocId();
                builder.emitExtractElement(typeId, outValueId, baseId, indexId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Div:
        case kIROp_Mul:
        case kIROp_IRem:
        case kIROp_Lsh:
            {
                if (inst->getOperandCount() != 2)
                    return fail(inst, "malformed binary arithmetic instructions");

                auto resultType = inst->getDataType();
                IRType* scalarType = tryGetNumericScalarType(resultType);
                if (!scalarType || (scalarType->getOp() != kIROp_IntType &&
                                    scalarType->getOp() != kIROp_UIntType &&
                                    scalarType->getOp() != kIROp_HalfType &&
                                    scalarType->getOp() != kIROp_FloatType &&
                                    scalarType->getOp() != kIROp_DoubleType))
                {
                    return fail(inst, "arithmetic on non-numeric graph values");
                }

                // An HGBC arithmetic record declares one type for both inputs. The HGBC target
                // legalization turns `float2Value * scalar` into
                // `float2Value * float2(scalar)` before emission. Reject any remaining mixed
                // representation here, while the IR still carries the source location, instead
                // of writing a record that the format validator can only identify by byte offset.
                for (UInt operandIndex = 0; operandIndex < inst->getOperandCount(); ++operandIndex)
                {
                    if (inst->getOperand(operandIndex)->getDataType() != resultType)
                    {
                        return fail(
                            inst,
                            "arithmetic with operand types that differ from the result type");
                    }
                }

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                HGBCId leftId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), leftId));

                HGBCId rightId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(1), rightId));

                outValueId = builder.allocId();
                HGBCOpcode opcode = HGBCOpcode::Mul;
                if (inst->getOp() == kIROp_Add)
                    opcode = HGBCOpcode::Add;
                else if (inst->getOp() == kIROp_Sub)
                    opcode = HGBCOpcode::Sub;
                else if (inst->getOp() == kIROp_Div)
                {
                    if (scalarType->getOp() == kIROp_HalfType ||
                        scalarType->getOp() == kIROp_FloatType ||
                        scalarType->getOp() == kIROp_DoubleType)
                        opcode = HGBCOpcode::FDiv;
                    else if (
                        scalarType->getOp() == kIROp_UIntType ||
                        scalarType->getOp() == kIROp_IntType)
                        opcode = HGBCOpcode::IDiv;
                    else
                        return fail(inst, "division on non-numeric graph values");
                }
                else if (inst->getOp() == kIROp_IRem)
                {
                    if (scalarType->getOp() == kIROp_UIntType ||
                        scalarType->getOp() == kIROp_IntType)
                    {
                        opcode = HGBCOpcode::IRem;
                    }
                    else
                    {
                        return fail(inst, "remainder on non-integer graph values");
                    }
                }
                else if (inst->getOp() == kIROp_Lsh)
                {
                    if (scalarType->getOp() == kIROp_UIntType ||
                        scalarType->getOp() == kIROp_IntType)
                    {
                        opcode = HGBCOpcode::Lsh;
                    }
                    else
                    {
                        return fail(inst, "left shift on non-integer graph values");
                    }
                }
                builder.emitTypedBinaryOp(opcode, typeId, outValueId, typeId, leftId, rightId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_Neg:
            {
                IRType* scalarType = tryGetNumericScalarType(inst->getDataType());
                if (inst->getOperandCount() != 1 || !scalarType ||
                    (scalarType->getOp() != kIROp_IntType &&
                     scalarType->getOp() != kIROp_HalfType &&
                     scalarType->getOp() != kIROp_FloatType &&
                     scalarType->getOp() != kIROp_DoubleType))
                {
                    return fail(inst, "negation of unsigned or non-numeric graph values");
                }

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                HGBCId valueId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), valueId));

                outValueId = builder.allocId();
                builder.emitUnary(HGBCOpcode::Neg, typeId, outValueId, valueId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Less:
        case kIROp_Greater:
        case kIROp_Leq:
        case kIROp_Geq:
            {
                if (inst->getOperandCount() != 2)
                    return fail(inst, "malformed comparison instructions");

                bool isOrderedCompare = inst->getOp() == kIROp_Less ||
                                        inst->getOp() == kIROp_Greater ||
                                        inst->getOp() == kIROp_Leq || inst->getOp() == kIROp_Geq;
                if (isOrderedCompare)
                {
                    for (UInt operandIndex = 0; operandIndex < inst->getOperandCount();
                         operandIndex++)
                    {
                        auto operandType = inst->getOperand(operandIndex)->getDataType();
                        IRType* scalarType = tryGetNumericScalarType(operandType);
                        if (!scalarType || (scalarType->getOp() != kIROp_IntType &&
                                            scalarType->getOp() != kIROp_UIntType &&
                                            scalarType->getOp() != kIROp_HalfType &&
                                            scalarType->getOp() != kIROp_FloatType &&
                                            scalarType->getOp() != kIROp_DoubleType))
                        {
                            return fail(inst, "ordered comparisons on non-numeric operands");
                        }
                    }
                }

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                auto operandType = inst->getOperand(0)->getDataType();
                if (!operandType || operandType != inst->getOperand(1)->getDataType())
                    return fail(inst, "comparisons with mismatched operand types");

                HGBCId operandTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(operandType, operandTypeId));

                HGBCId leftId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), leftId));

                HGBCId rightId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(1), rightId));

                outValueId = builder.allocId();
                HGBCOpcode opcode = inst->getOp() == kIROp_Eql       ? HGBCOpcode::Eql
                                    : inst->getOp() == kIROp_Neq     ? HGBCOpcode::Neq
                                    : inst->getOp() == kIROp_Less    ? HGBCOpcode::Less
                                    : inst->getOp() == kIROp_Greater ? HGBCOpcode::Greater
                                    : inst->getOp() == kIROp_Leq     ? HGBCOpcode::Leq
                                                                     : HGBCOpcode::Geq;
                builder
                    .emitTypedBinaryOp(opcode, typeId, outValueId, operandTypeId, leftId, rightId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_Not:
            {
                if (inst->getOperandCount() != 1)
                    return fail(inst, "malformed not instructions");

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                HGBCId valueId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), valueId));

                outValueId = builder.allocId();
                builder.emitNot(typeId, outValueId, valueId);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_MakeVector:
            {
                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                List<HGBCId> elementIds;
                SLANG_RETURN_ON_FAIL(emitScalarConstructorOperands(inst, elementIds));

                outValueId = builder.allocId();
                builder.emitMakeVector(typeId, outValueId, elementIds);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_MakeVectorFromScalar:
            {
                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                auto vectorType = inst->getDataType();
                if (vectorType->getOperandCount() < 2)
                    return fail(inst, "malformed vector splats");

                auto elementCount = as<IRIntLit>(vectorType->getOperand(1));
                if (!elementCount)
                    return fail(inst, "non-constant vector splats");

                HGBCId scalarId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), scalarId));

                List<HGBCId> elementIds;
                for (uint32_t i = 0; i < uint32_t(elementCount->getValue()); i++)
                    elementIds.add(scalarId);

                outValueId = builder.allocId();
                builder.emitMakeVector(typeId, outValueId, elementIds);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_MakeMatrix:
        case kIROp_MakeMatrixFromScalar:
            {
                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                List<HGBCId> elementIds;
                if (inst->getOp() == kIROp_MakeMatrixFromScalar)
                {
                    auto matrixType = as<IRMatrixType>(inst->getDataType());
                    auto rowCount = matrixType ? as<IRIntLit>(matrixType->getRowCount()) : nullptr;
                    auto columnCount =
                        matrixType ? as<IRIntLit>(matrixType->getColumnCount()) : nullptr;
                    if (!rowCount || !columnCount || inst->getOperandCount() != 1)
                        return fail(inst, "malformed matrix splats");

                    HGBCId scalarId = 0;
                    SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), scalarId));
                    IRIntegerValue elementCount = rowCount->getValue() * columnCount->getValue();
                    if (elementCount < 0 || elementCount > IRIntegerValue(UINT32_MAX))
                        return fail(inst, "matrix splats with invalid dimensions");
                    for (uint32_t i = 0; i < uint32_t(elementCount); ++i)
                        elementIds.add(scalarId);
                }
                else
                {
                    SLANG_RETURN_ON_FAIL(emitScalarConstructorOperands(inst, elementIds));
                }

                outValueId = builder.allocId();
                builder.emitMakeMatrix(typeId, outValueId, elementIds);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_MakeArray:
        case kIROp_MakeArrayFromElement:
            {
                auto arrayType = as<IRArrayType>(inst->getDataType());
                auto elementCount =
                    arrayType ? as<IRIntLit>(arrayType->getElementCount()) : nullptr;
                if (!elementCount || elementCount->getValue() < 0 ||
                    elementCount->getValue() > IRIntegerValue(UINT32_MAX))
                {
                    return fail(inst, "array construction with an invalid fixed dimension");
                }

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                List<HGBCId> elementIds;
                if (inst->getOp() == kIROp_MakeArrayFromElement)
                {
                    if (inst->getOperandCount() != 1)
                        return fail(inst, "malformed fixed-size array splats");
                    HGBCId elementId = 0;
                    SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), elementId));
                    for (uint32_t i = 0; i < uint32_t(elementCount->getValue()); ++i)
                        elementIds.add(elementId);
                }
                else
                {
                    if (inst->getOperandCount() != UInt(elementCount->getValue()))
                    {
                        return fail(
                            inst,
                            "fixed-size array construction with the wrong element count");
                    }
                    for (UInt i = 0; i < inst->getOperandCount(); ++i)
                    {
                        HGBCId elementId = 0;
                        SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(i), elementId));
                        elementIds.add(elementId);
                    }
                }

                outValueId = builder.allocId();
                builder.emitMakeArray(typeId, outValueId, elementIds);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_MakeStruct:
            {
                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                List<HGBCId> fieldValueIds;
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    HGBCId fieldValueId = 0;
                    SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(i), fieldValueId));
                    fieldValueIds.add(fieldValueId);
                }

                outValueId = builder.allocId();
                builder.emitMakeStruct(typeId, outValueId, fieldValueIds);
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        case kIROp_FieldExtract:
            {
                auto fieldExtract = cast<IRFieldExtract>(inst);
                auto fieldKey = as<IRStructKey>(fieldExtract->getField());
                if (!fieldKey)
                    return fail(inst, "non-struct field extracts");

                HGBCId typeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(inst->getDataType(), typeId));

                HGBCId baseId = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(fieldExtract->getBase(), baseId));

                outValueId = builder.allocId();
                builder.emitFieldExtract(typeId, outValueId, baseId, getFieldNameId(fieldKey));
                registerValueId(inst, outValueId);
                return SLANG_OK;
            }

        default:
            break;
        }

        StringBuilder message;
        message << "graph value produced by ";
        message << getIROpInfo(inst->getOp()).name;
        return fail(inst, message.getBuffer());
    }

    SlangResult emitPrintfLikeInst(IRInst* inst)
    {
        if (inst->getOperandCount() == 0)
            return fail(inst, "print calls without an argument");

        if (inst->getOperandCount() > 1 && !isEmptyAggregate(inst->getOperand(1)))
            return fail(inst, "formatted print arguments");

        HGBCId valueId = 0;
        SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), valueId));
        HGBCId valueTypeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(inst->getOperand(0)->getDataType(), valueTypeId));
        builder.emitPrint(valueTypeId, valueId);
        return SLANG_OK;
    }

    SlangResult emitReturnInst(IRInst* inst)
    {
        if (inst->getOperandCount() > 0 && inst->getOperand(0)->getDataType() &&
            inst->getOperand(0)->getDataType()->getOp() != kIROp_VoidType)
        {
            HGBCId valueId = 0;
            SLANG_RETURN_ON_FAIL(emitInstResultId(inst->getOperand(0), valueId));
            builder.emitReturn(valueId);
            return SLANG_OK;
        }

        builder.emitReturn();
        return SLANG_OK;
    }

    SlangResult emitBranchInst(IRUnconditionalBranch* branch)
    {
        // HGBC is emitted from non-SSA CFG IR. Consider `br merge(value)`: eliminatePhis must
        // replace the merge parameter with a variable and this branch with `store; br merge`
        // before emission, so any surviving argument is a compiler-pipeline invariant failure.
        SLANG_RELEASE_ASSERT(branch->getArgCount() == 0);
        builder.emitBranch(getBlockId(branch->getTargetBlock()));
        return SLANG_OK;
    }

    SlangResult emitConditionalBranchInst(IRConditionalBranch* branch)
    {
        HGBCId conditionId = 0;
        SLANG_RETURN_ON_FAIL(emitInstResultId(branch->getCondition(), conditionId));

        builder.emitBranchConditional(
            conditionId,
            getBlockId(branch->getTrueBlock()),
            getBlockId(branch->getFalseBlock()));
        return SLANG_OK;
    }

    SlangResult emitDispatchKernelInst(IRDispatchKernel* dispatchKernel)
    {
        if (dispatchKernel->getOperandCount() < 3)
            return fail(dispatchKernel, "malformed dispatch kernel instructions");

        HGBCId entryPointId = 0;
        SLANG_RETURN_ON_FAIL(getDispatchEntryPointId(dispatchKernel->getOperand(0), entryPointId));

        IRInst* dispatchSize = dispatchKernel->getOperand(2);

        HGBCId dispatchSizeId = 0;
        SLANG_RETURN_ON_FAIL(emitInstResultId(dispatchSize, dispatchSizeId));

        List<HGBCId> argTypeIds;
        List<HGBCId> argIds;

        HGBCId dispatchSizeTypeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(dispatchSize->getDataType(), dispatchSizeTypeId));
        argTypeIds.add(dispatchSizeTypeId);
        argIds.add(dispatchSizeId);

        for (UInt i = 0; i < dispatchKernel->getArgCount(); i++)
        {
            auto arg = dispatchKernel->getArg(i);

            HGBCId argTypeId = 0;
            SLANG_RETURN_ON_FAIL(lowerType(arg->getDataType(), argTypeId));
            argTypeIds.add(argTypeId);

            HGBCId argId = 0;
            SLANG_RETURN_ON_FAIL(emitInstResultId(arg, argId));
            argIds.add(argId);
        }

        HGBCId dispatcherTypeId = builder.getFuncTypeId(builder.getVoidTypeId(), argTypeIds);
        auto mapping = dispatchKernel->findDecoration<IRShaderParameterMappingDecoration>();
        SLANG_RELEASE_ASSERT(
            mapping && mapping->getOperandCount() == dispatchKernel->getArgCount());
        List<uint32_t> parameterIndices;
        for (UInt i = 0; i < mapping->getOperandCount(); ++i)
            parameterIndices.add(uint32_t(cast<IRIntLit>(mapping->getOperand(i))->getValue()));
        HGBCId dispatcherId = builder.getComputeDispatcherId(
            dispatcherTypeId,
            entryPointId,
            argTypeIds,
            parameterIndices);

        HGBCId callId = builder.allocId();
        builder.emitCall(builder.getVoidTypeId(), callId, dispatcherId, argIds);
        registerValueId(dispatchKernel, callId);
        return SLANG_OK;
    }

    /// Serializes the library's normalized state description after ordinary specialization and
    /// constant folding. Only literal aggregates are valid: accepting a host value here would
    /// require a pipeline variant during a draw, violating the global preparation contract.
    SlangResult appendGraphicsStateWords(IRInst* value, List<uint32_t>& words)
    {
        if (auto literal = as<IRFloatLit>(value))
        {
            float scalar = float(literal->getValue());
            uint32_t bits;
            memcpy(&bits, &scalar, sizeof(bits));
            words.add(bits);
            return SLANG_OK;
        }
        if (auto literal = as<IRIntLit>(value))
        {
            words.add(uint32_t(literal->getValue()));
            return SLANG_OK;
        }
        switch (value->getOp())
        {
        case kIROp_MakeStruct:
        case kIROp_MakeTuple:
        case kIROp_MakeArray:
            for (UInt i = 0; i < value->getOperandCount(); ++i)
                SLANG_RETURN_ON_FAIL(appendGraphicsStateWords(value->getOperand(i), words));
            return SLANG_OK;
        default:
            return fail(
                value,
                "graphics pipeline state that specializes to constant policy values");
        }
    }

    SlangResult emitMakeGraphicsPipelineInst(IRMakeGraphicsPipeline* pipeline)
    {
        auto functionType = cast<IRFuncType>(pipeline->getDataType());
        uint32_t drawCount =
            uint32_t(cast<IRIntLit>(pipeline->getDrawParameterCount())->getValue());
        uint32_t vertexCount =
            uint32_t(cast<IRIntLit>(pipeline->getVertexArgumentCount())->getValue());
        SLANG_RELEASE_ASSERT(drawCount == 3 || drawCount == 4);
        auto colorType = as<IRTextureTypeBase>(functionType->getParamType(0));
        auto format = colorType ? as<IRIntLit>(colorType->getFormatInst()) : nullptr;
        if (!format || format->getValue() == 0)
            return fail(pipeline, "color targets with a statically known texture format");

        HGBCResourceFormat depthFormat = HGBCResourceFormat::Undefined;
        if (drawCount == 4)
        {
            auto depthType = as<IRTextureTypeBase>(functionType->getParamType(1));
            auto depthFormatInst = depthType ? as<IRIntLit>(depthType->getFormatInst()) : nullptr;
            if (!depthFormatInst ||
                !isHGBCDepthFormat(HGBCResourceFormat(depthFormatInst->getValue())))
                return fail(pipeline, "depth targets with a statically known depth/stencil format");
            depthFormat = HGBCResourceFormat(depthFormatInst->getValue());
        }

        List<uint32_t> stateWords;
        SLANG_RETURN_ON_FAIL(appendGraphicsStateWords(pipeline->getState(), stateWords));
        const auto stateInfo = getHGBCOpcodeInfo(HGBCOpcode::GraphicsPipelineState);
        const auto colorInfo = getHGBCOpcodeInfo(HGBCOpcode::GraphicsColorTargetState);
        const Index stateCount = stateInfo->fixedOperandCount - 1;
        const Index colorCount = colorInfo->fixedOperandCount - 2;
        if (stateWords.getCount() != stateCount + colorCount)
            return fail(pipeline, "one color-target policy for the single-color draw overload");

        HGBCId vertexId = 0, fragmentId = 0, typeId = 0, colorTypeId = 0;
        SLANG_RETURN_ON_FAIL(getDispatchEntryPointId(pipeline->getVertexFn(), vertexId));
        SLANG_RETURN_ON_FAIL(getDispatchEntryPointId(pipeline->getFragmentFn(), fragmentId));
        SLANG_RETURN_ON_FAIL(getFuncTypeId(functionType, typeId));
        SLANG_RETURN_ON_FAIL(lowerType(colorType, colorTypeId));
        StringBuilder key;
        key << typeId << ":" << vertexId << ":" << fragmentId << ":" << vertexCount;
        for (auto word : stateWords)
            key << ":" << word;
        for (UInt i = 5; i < pipeline->getOperandCount(); ++i)
            key << ":" << cast<IRIntLit>(pipeline->getOperand(i))->getValue();
        String keyString = key.produceString();
        if (auto existing = builder.graphicsPipelineIds.tryGetValue(keyString))
        {
            registerValueId(pipeline, *existing);
            return SLANG_OK;
        }

        HGBCId resultId = builder.allocId();
        builder.graphicsPipelineIds.add(keyString, resultId);
        HGBCId stateId = builder.allocId();
        List<uint32_t> stateOperands;
        stateOperands.add(stateId);
        for (Index i = 0; i < stateCount; ++i)
            stateOperands.add(stateWords[i]);
        builder.emitInst(
            HGBCOpcode::GraphicsPipelineState,
            stateOperands.getBuffer(),
            stateOperands.getCount());
        HGBCId colorId = builder.allocId();
        List<uint32_t> colorOperands;
        colorOperands.add(colorId);
        colorOperands.add(colorTypeId);
        for (Index i = stateCount; i < stateWords.getCount(); ++i)
            colorOperands.add(stateWords[i]);
        builder.emitInst(
            HGBCOpcode::GraphicsColorTargetState,
            colorOperands.getBuffer(),
            colorOperands.getCount());
        HGBCId attachmentsId = builder.allocId();
        uint32_t attachmentOperands[] = {attachmentsId, uint32_t(depthFormat), colorId};
        builder.emitInst(
            HGBCOpcode::GraphicsAttachments,
            attachmentOperands,
            SLANG_COUNT_OF(attachmentOperands));
        List<uint32_t> operands;
        operands.add(typeId);
        operands.add(resultId);
        operands.add(vertexId);
        operands.add(fragmentId);
        operands.add(stateId);
        operands.add(attachmentsId);
        operands.add(drawCount);
        operands.add(vertexCount);
        for (UInt i = drawCount; i < functionType->getParamCount(); ++i)
        {
            HGBCId argumentType = 0;
            SLANG_RETURN_ON_FAIL(lowerType(functionType->getParamType(i), argumentType));
            UInt argumentIndex = i - drawCount;
            uint32_t parameterIndex =
                uint32_t(cast<IRIntLit>(pipeline->getOperand(5 + argumentIndex))->getValue());
            HGBCId host = builder.getTypeLayoutId(
                argumentType,
                HGBCLayoutDomain::Host,
                HGBCLayoutUsage::Value);
            HGBCId shader = builder.getTypeLayoutId(
                argumentType,
                HGBCLayoutDomain::ShaderObject,
                HGBCLayoutUsage::ShaderParameter,
                resultId,
                parameterIndex,
                argumentIndex < vertexCount ? 0 : 1);
            operands.add(builder.getLayoutConverterId(host, shader));
        }
        builder.emitInst(
            HGBCOpcode::MakeGraphicsPipeline,
            operands.getBuffer(),
            operands.getCount());
        registerValueId(pipeline, resultId);
        return SLANG_OK;
    }

    SlangResult emitMakeRayTracerInst(IRMakeRayTracer* rayTracer)
    {
        if (rayTracer->getOperandCount() != 5)
            return fail(rayTracer, "malformed ray-tracing dispatcher constructors");

        HGBCId payloadTypeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(rayTracer->getOperand(0), payloadTypeId));
        auto maxRecursionInst = as<IRIntLit>(rayTracer->getOperand(1));
        if (!maxRecursionInst || maxRecursionInst->getValue() < 1 ||
            maxRecursionInst->getValue() > std::numeric_limits<uint32_t>::max())
        {
            return fail(rayTracer, "ray tracers with a valid constant recursion depth");
        }

        HGBCId rayGenerationEntryPointId = 0;
        HGBCId closestHitEntryPointId = 0;
        HGBCId missEntryPointId = 0;
        SLANG_RETURN_ON_FAIL(
            getDispatchEntryPointId(rayTracer->getOperand(2), rayGenerationEntryPointId));
        SLANG_RETURN_ON_FAIL(
            getDispatchEntryPointId(rayTracer->getOperand(3), closestHitEntryPointId));
        SLANG_RETURN_ON_FAIL(getDispatchEntryPointId(rayTracer->getOperand(4), missEntryPointId));

        auto dispatcherType = as<IRFuncType>(rayTracer->getDataType());
        if (!dispatcherType)
            return fail(rayTracer, "ray tracers with function type");
        HGBCId dispatcherTypeId = 0;
        SLANG_RETURN_ON_FAIL(getFuncTypeId(dispatcherType, dispatcherTypeId));
        HGBCId dispatcherId = builder.getRayTracerId(
            dispatcherTypeId,
            payloadTypeId,
            uint32_t(maxRecursionInst->getValue()),
            rayGenerationEntryPointId,
            closestHitEntryPointId,
            missEntryPointId);
        registerValueId(rayTracer, dispatcherId);
        return SLANG_OK;
    }

    SlangResult emitRayPipelineCallInst(IRCall* call, IRRayPipelineCall* recipe)
    {
        if (recipe->getOperandCount() != 11 + 2 * call->getArgCount())
            return fail(recipe, "ray calls with preserved shader parameter identities");
        auto kind = cast<IRIntLit>(recipe->getOperand(0))->getValue();
        HGBCId entryPoint = 0;
        if (!as<IRIntLit>(recipe->getOperand(1)))
            SLANG_RETURN_ON_FAIL(getDispatchEntryPointId(recipe->getOperand(1), entryPoint));
        List<uint32_t> operands;
        HGBCOpcode opcode;
        if (kind == 2)
        {
            opcode = HGBCOpcode::DispatchRays;
            HGBCId extent = 0;
            SLANG_RETURN_ON_FAIL(emitInstResultId(recipe->getOperand(5), extent));
            auto depth = as<IRIntLit>(recipe->getOperand(6));
            if (!depth || depth->getValue() < 1 || depth->getValue() > UINT32_MAX)
                return fail(recipe, "ray dispatches with a positive constant uint recursion depth");
            operands.add(entryPoint);
            operands.add(extent);
            operands.add(uint32_t(depth->getValue()));
        }
        else
        {
            HGBCId pipeline = 0, raySlot = 0, payloadType = 0;
            SLANG_RETURN_ON_FAIL(emitInstResultId(recipe->getOperand(2), pipeline));
            SLANG_RETURN_ON_FAIL(emitInstResultId(recipe->getOperand(4), raySlot));
            SLANG_RETURN_ON_FAIL(lowerType(recipe->getOperand(7), payloadType));
            operands.add(pipeline);
            if (kind == 0)
            {
                opcode = HGBCOpcode::SetRayMiss;
                operands.add(raySlot);
                operands.add(entryPoint);
                operands.add(payloadType);
            }
            else
            {
                SLANG_RELEASE_ASSERT(kind == 1 || kind == 3);
                opcode = HGBCOpcode::SetRayHitGroup;
                HGBCId geometrySlot = 0, attributeType = 0;
                SLANG_RETURN_ON_FAIL(emitInstResultId(recipe->getOperand(3), geometrySlot));
                SLANG_RETURN_ON_FAIL(lowerType(recipe->getOperand(8), attributeType));
                HGBCId entries[3] = {entryPoint, 0, 0};
                for (UInt role = 1; role < 3; ++role)
                    if (!as<IRIntLit>(recipe->getOperand(8 + role)))
                        SLANG_RETURN_ON_FAIL(
                            getDispatchEntryPointId(recipe->getOperand(8 + role), entries[role]));
                auto geometryKind =
                    entries[2] ? HGBCRayGeometryKind::Procedural : HGBCRayGeometryKind::Triangles;
                StringBuilder key;
                key << uint32_t(geometryKind) << ":" << payloadType << ":" << attributeType;
                for (auto entry : entries)
                    key << ":" << entry;
                String groupKey = key.produceString();
                HGBCId groupId = 0;
                if (auto existing = builder.rayHitGroupIds.tryGetValue(groupKey))
                    groupId = *existing;
                else
                {
                    groupId = builder.allocId();
                    builder.rayHitGroupIds.add(groupKey, groupId);
                    List<uint32_t> groupOperands;
                    groupOperands.add(groupId);
                    groupOperands.add(uint32_t(geometryKind));
                    groupOperands.add(payloadType);
                    groupOperands.add(attributeType);
                    for (UInt role = 0; role < 3; ++role)
                        if (entries[role])
                        {
                            groupOperands.add(uint32_t(role));
                            groupOperands.add(entries[role]);
                        }
                    builder.emitInst(
                        HGBCOpcode::RayHitGroup,
                        groupOperands.getBuffer(),
                        groupOperands.getCount());
                }
                operands.add(geometrySlot);
                operands.add(raySlot);
                operands.add(groupId);
            }
        }
        for (UInt i = 0; i < call->getArgCount(); ++i)
        {
            HGBCId argumentType = 0, argument = 0;
            SLANG_RETURN_ON_FAIL(lowerType(call->getArg(i)->getDataType(), argumentType));
            SLANG_RETURN_ON_FAIL(emitInstResultId(call->getArg(i), argument));
            if (kind == 1 || kind == 3)
                operands.add(uint32_t(cast<IRIntLit>(recipe->getOperand(11 + 2 * i))->getValue()));
            operands.add(uint32_t(cast<IRIntLit>(recipe->getOperand(12 + 2 * i))->getValue()));
            operands.add(argumentType);
            operands.add(argument);
        }
        builder.emitInst(opcode, operands.getBuffer(), operands.getCount());
        return SLANG_OK;
    }

    SlangResult emitCallInst(IRCall* call)
    {
        auto calleeInst = call->getOperand(0);
        if (auto recipe = as<IRRayPipelineCall>(calleeInst))
            return emitRayPipelineCallInst(call, recipe);
        auto callee = as<IRFunc>(calleeInst);
        HGBCId calleeId = 0;
        if (callee)
        {
            SLANG_RETURN_ON_FAIL(checkFunctionType(callee));
            calleeId = getFuncId(callee);
            queueFuncForEmission(callee);
        }
        else if (as<IRMakeGraphicsPipeline>(calleeInst))
        {
            SLANG_RETURN_ON_FAIL(emitInstResultId(calleeInst, calleeId));
        }
        else if (as<IRMakeRayTracer>(calleeInst))
        {
            SLANG_RETURN_ON_FAIL(emitInstResultId(calleeInst, calleeId));
        }
        else
        {
            return fail(call, "non-direct graph function callees");
        }

        List<HGBCId> argIds;
        for (UInt i = 0; i < call->getArgCount(); i++)
        {
            HGBCId argId = 0;
            SLANG_RETURN_ON_FAIL(emitInstResultId(call->getArg(i), argId));
            argIds.add(argId);
        }

        HGBCId callId = builder.allocId();
        HGBCId resultTypeId = 0;
        SLANG_RETURN_ON_FAIL(lowerType(call->getDataType(), resultTypeId));

        builder.emitCall(resultTypeId, callId, calleeId, argIds);
        registerValueId(call, callId);
        return SLANG_OK;
    }

    SlangResult emitInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_HGBCAsm:
            {
                HGBCId ignoredValueId = 0;
                return emitHGBCAsm(cast<IRHGBCAsm>(inst), ignoredValueId);
            }

        case kIROp_Printf:
            return emitPrintfLikeInst(inst);

        case kIROp_DispatchKernel:
            return emitDispatchKernelInst(cast<IRDispatchKernel>(inst));

        case kIROp_MakeGraphicsPipeline:
            return emitMakeGraphicsPipelineInst(cast<IRMakeGraphicsPipeline>(inst));

        case kIROp_MakeRayTracer:
            return emitMakeRayTracerInst(cast<IRMakeRayTracer>(inst));

        case kIROp_Call:
            return emitCallInst(cast<IRCall>(inst));

        case kIROp_Store:
            return emitStoreInst(cast<IRStore>(inst));

        case kIROp_Return:
            return emitReturnInst(inst);

        case kIROp_UnconditionalBranch:
            return emitBranchInst(cast<IRUnconditionalBranch>(inst));

        case kIROp_ConditionalBranch:
            return emitConditionalBranchInst(cast<IRConditionalBranch>(inst));

        case kIROp_IntLit:
        case kIROp_BoolLit:
        case kIROp_FloatLit:
        case kIROp_DefaultConstruct:
        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_CastIntToFloat:
        case kIROp_CastFloatToInt:
        case kIROp_Var:
        case kIROp_Load:
        case kIROp_CastDescriptorHandleToResource:
        case kIROp_AllocObj:
        case kIROp_FieldAddress:
        case kIROp_GetElementPtr:
        case kIROp_HGVMRuntimeArrayGetElementPtr:
        case kIROp_StructuredBufferGetDimensions:
        case kIROp_HGVMBox:
        case kIROp_HGVMUnbox:
        case kIROp_HGVMMakeTaggedUnion:
        case kIROp_HGVMGetTaggedUnionCase:
        case kIROp_HGVMGetTaggedUnionPayload:
        case kIROp_HGVMExtractTaggedUnion:
        case kIROp_HGVMConvertTaggedUnion:
        case kIROp_TextureGetDimensions:
        case kIROp_Swizzle:
        case kIROp_GetElement:
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Div:
        case kIROp_Mul:
        case kIROp_IRem:
        case kIROp_Lsh:
        case kIROp_Neg:
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Less:
        case kIROp_Greater:
        case kIROp_Leq:
        case kIROp_Geq:
        case kIROp_Not:
        case kIROp_MakeVector:
        case kIROp_MakeVectorFromScalar:
        case kIROp_MakeMatrix:
        case kIROp_MakeMatrixFromScalar:
        case kIROp_MakeArray:
        case kIROp_MakeArrayFromElement:
        case kIROp_MakeStruct:
        case kIROp_FieldExtract:
            {
                HGBCId ignoredValueId = 0;
                return emitInstResultId(inst, ignoredValueId);
            }

        case kIROp_SlangEntryPoint:
        case kIROp_SpecializedSlangEntryPoint:
        case kIROp_RayPipelineCall:
        case kIROp_MakeTuple:
            return SLANG_OK;

        default:
            break;
        }

        StringBuilder message;
        message << getIROpInfo(inst->getOp()).name;
        return fail(inst, message.getBuffer());
    }

    /// Returns the semantic owner of a constructor or method, including intrinsic owners.
    IRType* getMemberOwnerType(IRFunc* func)
    {
        if (func->findDecoration<IRConstructorDecoration>())
            return func->getResultType();
        if (auto method = func->findDecoration<IRStaticMethodDecoration>())
            return method->getOwnerType();
        if (func->findDecoration<IRMethodDecoration>())
        {
            auto receiver = func->getFirstParam();
            SLANG_RELEASE_ASSERT(receiver);
            IRType* type = receiver->getDataType();
            if (auto address = as<IRPtrTypeBase>(type))
                type = address->getValueType();
            return type;
        }
        return nullptr;
    }

    /// HGBC exports aggregate APIs only for types represented as graph structs or classes.
    /// For example, a public BLAS alloc() method has a nominal struct owner in Slang, but
    /// that owner lowers to an opaque acceleration handle. Exporting it as a struct method
    /// would reference a nonexistent aggregate. Apply the same rule to types and members;
    /// opaque methods remain ordinary graph functions callable by the compiled program.
    bool isExportableAggregateType(IRType* type)
    {
        if (as<IRClassType>(type))
            return true;
        if (!as<IRStructType>(type))
            return false;
        HGBCOpcode opaqueTypeOpcode;
        return !tryGetOpaqueTypeOpcode(type, opaqueTypeOpcode);
    }

    SlangResult emitFunction(IRFunc* func, bool shouldExport)
    {
        if (emittedFuncs.containsKey(func))
            return SLANG_OK;

        emittedFuncs.add(func, true);
        SLANG_RETURN_ON_FAIL(checkFunctionType(func));
        valueIds.clear();
        blockIds.clear();

        if (!func->getFirstBlock())
            return fail(func, "declaration-only graph functions");

        for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
            getBlockId(block);

        HGBCId funcId = getFuncId(func);

        HGBCId funcTypeId = 0;
        SLANG_RETURN_ON_FAIL(getFuncTypeId(func, funcTypeId));

        if (shouldExport)
        {
            // Actual entry points carry their externally selected name. Other callable API
            // declarations retain their checked source name independently of debug/linkage.
            IRStringLit* exportName = nullptr;
            if (auto entryPoint = func->findDecoration<IREntryPointDecoration>())
                exportName = entryPoint->getName();
            else
                exportName = getReflectionNameLiteral(func);
            if (!exportName || exportName->getStringSlice().getLength() == 0)
                return fail(func, "exported functions without semantic names");
            HGBCId entryPointNameId = builder.getStringLiteralId(exportName->getStringSlice());

            auto emitExport = [&](HGBCExportKind kind, HGBCId ownerTypeId = 0)
            {
                StringBuilder signature;
                HGBCId overloadNameId = kind == HGBCExportKind::Constructor ? 0 : entryPointNameId;
                signature << uint32_t(kind) << ":" << overloadNameId << ":" << ownerTypeId << ":"
                          << funcTypeId;
                String key = signature.produceString();
                if (exportedFunctionSignatures.containsKey(key))
                    return;
                exportedFunctionSignatures.add(key, true);
                builder.emitExport(kind, entryPointNameId, funcId, ownerTypeId);
            };

            if (auto ownerType = getMemberOwnerType(func))
            {
                SLANG_RELEASE_ASSERT(isExportableAggregateType(ownerType));
                HGBCId ownerTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(ownerType, ownerTypeId));
                auto kind = func->findDecoration<IRConstructorDecoration>()
                                ? HGBCExportKind::Constructor
                            : func->findDecoration<IRStaticMethodDecoration>()
                                ? HGBCExportKind::StaticMethod
                                : HGBCExportKind::Method;
                emitExport(kind, ownerTypeId);
            }
            else
            {
                emitExport(HGBCExportKind::Function);
            }
        }
        if (auto debugName = func->findDecoration<IRNameHintDecoration>())
            builder.emitNameHint(funcId, builder.getStringLiteralId(debugName->getName()));
        builder.emitFunc(funcTypeId, funcId);
        if (auto debugFunction = getDebugFunction(func))
        {
            builder.emitDecorate(
                HGBCDecorationKind::DebugFunction,
                funcId,
                getDebugFunctionId(debugFunction));
        }

        for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
        {
            builder.emitBlock(getBlockId(block));

            // The first block's parameters are function parameters, not phis. All parameters on
            // later blocks must have been converted to variables by eliminatePhis. For example,
            // an SSA `merge(%selected)` reaches HGBC as an OpVar written by each predecessor.
            if (block != func->getFirstBlock())
                SLANG_RELEASE_ASSERT(block->getFirstParam() == nullptr);

            for (auto param : block->getParams())
            {
                HGBCId paramTypeId = 0;
                SLANG_RETURN_ON_FAIL(lowerType(param->getDataType(), paramTypeId));

                HGBCId paramId = builder.allocId();
                registerValueId(param, paramId);
                builder.emitParam(paramTypeId, paramId);
            }

            for (auto inst : block->getOrdinaryInsts())
            {
                bool debugInstProcessed = false;
                SLANG_RETURN_ON_FAIL(processDebugInst(inst, debugInstProcessed));
                if (debugInstProcessed)
                    continue;
                SLANG_RETURN_ON_FAIL(emitInst(inst));
            }

            builder.emitBlockEnd();
        }

        builder.emitFunctionEnd();
        return SLANG_OK;
    }

    SlangResult emitPendingFunctions()
    {
        for (Index i = 0; i < pendingFuncs.getCount(); i++)
        {
            IRFunc* func = pendingFuncs[i];
            auto ownerType = getMemberOwnerType(func);
            bool shouldExport =
                isHGBCExportedAPI(func) &&
                (ownerType ? isExportableAggregateType(ownerType) : isDispatchEntryPoint(func));
            SLANG_RETURN_ON_FAIL(emitFunction(func, shouldExport));
        }
        return SLANG_OK;
    }

    SlangResult emitExportedStructTypes(LinkedIR const& linkedIR)
    {
        for (auto globalInst : linkedIR.module->getGlobalInsts())
        {
            auto structType = as<IRStructType>(globalInst);
            if (!structType)
                continue;
            if (!isHGBCExportedAPI(structType))
                continue;
            if (exportedStructTypes.containsKey(structType))
                continue;

            if (!isExportableAggregateType(structType))
                continue;

            HGBCId structTypeId = 0;
            SLANG_RETURN_ON_FAIL(lowerStructType(structType, structTypeId));

            auto reflectionName = getReflectionNameLiteral(structType);
            if (!reflectionName)
                return fail(structType, "exported struct types without reflection names");
            HGBCId reflectionNameId = builder.getStringLiteralId(reflectionName->getStringSlice());
            builder.emitExport(HGBCExportKind::Type, reflectionNameId, structTypeId);
            exportedStructTypes.add(structType, true);
        }

        return SLANG_OK;
    }

    SlangResult emitExportedClassTypes(LinkedIR const& linkedIR)
    {
        for (auto globalInst : linkedIR.module->getGlobalInsts())
        {
            auto classType = as<IRClassType>(globalInst);
            if (!classType || !isHGBCExportedAPI(classType) ||
                exportedClassTypes.containsKey(classType))
            {
                continue;
            }

            HGBCId classTypeId = 0;
            SLANG_RETURN_ON_FAIL(lowerClassType(classType, classTypeId));

            auto reflectionName = getReflectionNameLiteral(classType);
            if (!reflectionName)
                return fail(classType, "exported class types without reflection names");
            HGBCId reflectionNameId = builder.getStringLiteralId(reflectionName->getStringSlice());
            builder.emitExport(HGBCExportKind::Type, reflectionNameId, classTypeId);
            exportedClassTypes.add(classType, true);
        }
        return SLANG_OK;
    }

    SlangResult emitGlobalDebugInsts(LinkedIR const& linkedIR)
    {
        for (auto inst : linkedIR.module->getGlobalInsts())
        {
            if (as<IRDebugSource>(inst))
            {
                getDebugSourceId(inst);
            }
            else if (inst->getOp() == kIROp_DebugLambda)
            {
                emitDebugLambda(inst);
            }
            else if (as<IRDebugFunction>(inst))
            {
                getDebugFunctionId(inst);
            }
            else if (as<IRDebugLexicalBlock>(inst))
            {
                getDebugScopeId(inst);
            }
            else if (as<IRDebugInlinedAt>(inst))
            {
                getDebugInlineSiteId(inst);
            }
            else if (auto buildIdentifier = as<IRDebugBuildIdentifier>(inst))
            {
                auto identifier = as<IRStringLit>(buildIdentifier->getBuildIdentifier());
                SLANG_RELEASE_ASSERT(identifier);
                HGBCId id = builder.allocId();
                builder.emitDebugBuildIdentifier(
                    id,
                    builder.getStringLiteralId(identifier->getStringSlice()),
                    getDebugInteger(buildIdentifier->getFlags()));
            }
            else if (auto compilationUnit = as<IRDebugCompilationUnit>(inst))
            {
                HGBCId id = builder.allocId();
                builder.emitDebugCompilationUnit(
                    id,
                    getDebugSourceId(compilationUnit->getSource()));
            }
        }
        return SLANG_OK;
    }

    SlangResult emit(LinkedIR const& linkedIR, ComPtr<IArtifact>& outArtifact)
    {
        SLANG_RETURN_ON_FAIL(emitGlobalDebugInsts(linkedIR));

        Count emittedEntryPointCount = 0;
        for (auto entryPoint : linkedIR.entryPoints)
        {
            if (!isDispatchEntryPoint(entryPoint))
                continue;

            SLANG_RETURN_ON_FAIL(emitFunction(entryPoint, true));
            SLANG_RETURN_ON_FAIL(emitPendingFunctions());
            emittedEntryPointCount++;
        }

        SLANG_RETURN_ON_FAIL(emitExportedStructTypes(linkedIR));
        SLANG_RETURN_ON_FAIL(emitExportedClassTypes(linkedIR));
        SLANG_RETURN_ON_FAIL(emitSerializedModules());

        if (emittedEntryPointCount == 0)
        {
            diagnoseRaw(sink, "HGBC target requires at least one graph/dispatch entry point\n");
            return SLANG_FAIL;
        }

        ComPtr<ISlangBlob> blob;
        SLANG_RETURN_ON_FAIL(builder.writeContainer(blob));

        outArtifact = ArtifactUtil::createArtifactForCompileTarget(SLANG_HGBC);
        outArtifact->addRepresentationUnknown(blob);
        ArtifactUtil::addAssociated(outArtifact, linkedIR.metadata);
        return SLANG_OK;
    }
};

} // namespace

SlangResult emitHGBCForEntryPoints(CodeGenContext* codeGenContext, ComPtr<IArtifact>& outArtifact)
{
    LinkedIR linkedIR;
    SLANG_RETURN_ON_FAIL(linkAndOptimizeIRForHGBC(codeGenContext, linkedIR));
    if (codeGenContext->getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    auto sink = codeGenContext->getSink();

    HGBCEmitter emitter;
    emitter.codeGenContext = codeGenContext;
    emitter.sink = sink;
    return emitter.emit(linkedIR, outArtifact);
}

} // namespace Slang
