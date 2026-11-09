-- This file is the canonical description of the HGBC binary instruction set.
--
-- Keep wire-level facts here rather than duplicating them in the emitter,
-- container reader, or disassembler. The public C++ declarations and lookup
-- tables are generated from this description by slang-fiddle.
--
-- HGBC has no cross-version compatibility guarantee. Opcode and enum values may
-- change or be reused; regenerate bytecode with the matching compiler/runtime.

local M = {}

M.sections = {
    { name = "OFST", four_cc = "OFST", contains_records = false },
    { name = "LITR", four_cc = "LITR", contains_records = true },
    { name = "EXPT", four_cc = "EXPT", contains_records = true },
    { name = "META", four_cc = "META", contains_records = true },
    { name = "TYPE", four_cc = "TYPE", contains_records = true },
    { name = "FUNC", four_cc = "FUNC", contains_records = true },
    { name = "EMBD", four_cc = "EMBD", contains_records = true },
}

M.enums = {
    {
        name = "AccelerationStructureKind", cpp_name = "HGBCAccelerationStructureKind",
        values = { { name = "BottomLevel", value = 0 }, { name = "TopLevel", value = 1 } },
    },
    {
        name = "AccelerationStructureInputKind", cpp_name = "HGBCAccelerationStructureInputKind",
        values = { { name = "Triangles", value = 0 }, { name = "Instances", value = 1 },
                   { name = "AABBs", value = 2 } },
    },
    {
        name = "RayGeometryKind", cpp_name = "HGBCRayGeometryKind",
        values = { { name = "Triangles", value = 0 }, { name = "Procedural", value = 1 } },
    },
    {
        name = "RayStageRole", cpp_name = "HGBCRayStageRole",
        values = {
            { name = "ClosestHit", value = 0 },
            { name = "AnyHit", value = 1 },
            { name = "Intersection", value = 2 },
        },
    },
    {
        name = "ExportKind",
        cpp_name = "HGBCExportKind",
        values = {
            { name = "Function", value = 0 },
            { name = "Type", value = 1 },
            { name = "Constructor", value = 2 },
            { name = "Method", value = 3 },
            { name = "StaticMethod", value = 4 },
        },
    },
    {
        name = "DecorationKind",
        cpp_name = "HGBCDecorationKind",
        values = {
            { name = "NameHint", value = 0 },
            { name = "ModuleName", value = 1 },
            { name = "ModuleDigest", value = 2 },
            { name = "OriginalPath", value = 3 },
            { name = "SlangTypeName", value = 4 },
            { name = "DebugFunction", value = 5 },
            { name = "TypeMangledName", value = 6 },
        },
    },
    {
        name = "DispatchStage",
        cpp_name = "HGBCDispatchStage",
        values = {
            { name = "Compute", value = 0 },
        },
    },
    {
        name = "LayoutDomain",
        cpp_name = "HGBCLayoutDomain",
        values = {
            { name = "Host", value = 0 },
            { name = "DeviceMemory", value = 1 },
            { name = "ShaderObject", value = 2 },
            { name = "Serialized", value = 3 },
        },
    },
    {
        name = "LayoutUsage",
        cpp_name = "HGBCLayoutUsage",
        values = {
            { name = "Value", value = 0 },
            { name = "StructuredBufferElement", value = 1 },
            { name = "ShaderParameter", value = 2 },
        },
    },
    {
        name = "ByteStreamAccess",
        cpp_name = "HGBCByteStreamAccess",
        values = {
            { name = "Read", value = 0 },
            { name = "Write", value = 1 },
        },
    },
    {
        name = "ByteStreamEndpointKind",
        cpp_name = "HGBCByteStreamEndpointKind",
        values = {
            { name = "HostAddress", value = 0 },
            { name = "DeviceBuffer", value = 1 },
            { name = "RuntimeArray", value = 2 },
            { name = "DeviceTexture2D", value = 3 },
        },
    },
    {
        name = "ImageCodec",
        cpp_name = "HGBCImageCodec",
        values = {
            { name = "EXR", value = 0 },
            { name = "PNG", value = 1 },
            { name = "JPEG", value = 2 },
            { name = "BMP", value = 3 },
        },
    },
    {
        name = "EncodedImageEndpointKind",
        cpp_name = "HGBCEncodedImageEndpointKind",
        values = {
            { name = "ByteStream", value = 0 },
            { name = "HostByteSpan", value = 1 },
            { name = "DeviceBufferByteSpan", value = 2 },
        },
    },
    {
        name = "PixelImageEndpointKind",
        cpp_name = "HGBCPixelImageEndpointKind",
        values = {
            { name = "HostImageView", value = 0 },
            { name = "DeviceTexture2D", value = 1 },
        },
    },
    {
        name = "PrimitiveTopology",
        cpp_name = "HGBCPrimitiveTopology",
        values = {
            { name = "TriangleList", value = 0 },
            { name = "PointList", value = 1 },
            { name = "LineList", value = 2 },
            { name = "LineStrip", value = 3 },
            { name = "TriangleStrip", value = 4 },
            { name = "PatchList", value = 5 },
        },
    },
    {
        name = "TextureAspect",
        cpp_name = "HGBCTextureAspect",
        values = {
            { name = "All", value = 0 },
            { name = "DepthOnly", value = 1 },
            { name = "StencilOnly", value = 2 },
        },
    },
    {
        name = "ComparisonFunc",
        cpp_name = "HGBCComparisonFunc",
        values = {
            { name = "Never", value = 0 },
            { name = "Less", value = 1 },
            { name = "Equal", value = 2 },
            { name = "LessEqual", value = 3 },
            { name = "Greater", value = 4 },
            { name = "NotEqual", value = 5 },
            { name = "GreaterEqual", value = 6 },
            { name = "Always", value = 7 },
        },
    },
    {
        name = "SpecializationArgKind",
        cpp_name = "HGBCSpecializationArgKind",
        values = {
            { name = "Type", value = 0 },
            { name = "Expr", value = 1 },
        },
    },
    {
        name = "RaytracingGeometryKind",
        cpp_name = "HGBCRaytracingGeometryKind",
        values = {
            { name = "IndexedTriangles", value = 0 },
        },
    },
    {
        name = "RaytracingVertexFormat",
        cpp_name = "HGBCRaytracingVertexFormat",
        values = {
            { name = "Float3", value = 0 },
        },
    },
    {
        name = "RaytracingIndexFormat",
        cpp_name = "HGBCRaytracingIndexFormat",
        values = {
            { name = "UInt32", value = 0 },
        },
    },
    {
        name = "ResourceKind",
        cpp_name = "HGBCResourceKind",
        values = {
            { name = "Buffer", value = 0 },
            { name = "Texture", value = 1 },
        },
    },
    {
        name = "ResourceDimension",
        cpp_name = "HGBCResourceDimension",
        values = {
            { name = "Buffer", value = 0 },
            { name = "Texture1D", value = 1 },
            { name = "Texture2D", value = 2 },
            { name = "Texture3D", value = 3 },
            { name = "TextureCube", value = 4 },
        },
    },
    {
        name = "TextureAccess",
        cpp_name = "HGBCTextureAccess",
        values = {
            { name = "ReadOnly", value = 0 },
            { name = "ReadWrite", value = 1 },
            { name = "WriteOnly", value = 2 },
        },
    },
    {
        name = "ResourceFormat",
        cpp_name = "HGBCResourceFormat",
        -- These values intentionally match Slang ImageFormat in include/slang-image-format-defs.h.
        -- They are a shared semantic numbering, not a separately versioned HGBC format list.
        values = {
            { name = "Undefined", value = 0 },
            { name = "RGBA32Float", value = 1 },
            { name = "RGBA16Float", value = 2 },
            { name = "RG32Float", value = 3 },
            { name = "RG16Float", value = 4 },
            { name = "R11G11B10Float", value = 5 },
            { name = "R32Float", value = 6 },
            { name = "R16Float", value = 7 },
            { name = "RGBA16Unorm", value = 8 },
            { name = "RGB10A2Unorm", value = 9 },
            { name = "RGBA8Unorm", value = 10 },
            { name = "RG16Unorm", value = 11 },
            { name = "RG8Unorm", value = 12 },
            { name = "R16Unorm", value = 13 },
            { name = "R8Unorm", value = 14 },
            { name = "RGBA16Snorm", value = 15 },
            { name = "RGBA8Snorm", value = 16 },
            { name = "RG16Snorm", value = 17 },
            { name = "RG8Snorm", value = 18 },
            { name = "R16Snorm", value = 19 },
            { name = "R8Snorm", value = 20 },
            { name = "RGBA32Sint", value = 21 },
            { name = "RGBA16Sint", value = 22 },
            { name = "RGBA8Sint", value = 23 },
            { name = "RG32Sint", value = 24 },
            { name = "RG16Sint", value = 25 },
            { name = "RG8Sint", value = 26 },
            { name = "R32Sint", value = 27 },
            { name = "R16Sint", value = 28 },
            { name = "R8Sint", value = 29 },
            { name = "RGBA32Uint", value = 30 },
            { name = "RGBA16Uint", value = 31 },
            { name = "RGB10A2Uint", value = 32 },
            { name = "RGBA8Uint", value = 33 },
            { name = "RG32Uint", value = 34 },
            { name = "RG16Uint", value = 35 },
            { name = "RG8Uint", value = 36 },
            { name = "R32Uint", value = 37 },
            { name = "R16Uint", value = 38 },
            { name = "R8Uint", value = 39 },
            { name = "R64Uint", value = 40 },
            { name = "R64Sint", value = 41 },
            { name = "BGRA8Unorm", value = 42 },
            { name = "RGBA8UnormSrgb", value = 43 },
            { name = "D32Float", value = 44 },
            { name = "D16Unorm", value = 45 },
            { name = "D32FloatS8Uint", value = 46 },
        },
    },
    {
        name = "ResourceMemoryType",
        cpp_name = "HGBCResourceMemoryType",
        values = {
            { name = "DeviceLocal", value = 0 },
            { name = "Upload", value = 1 },
            { name = "ReadBack", value = 2 },
        },
    },
    {
        name = "ResourceState",
        cpp_name = "HGBCResourceState",
        values = {
            { name = "Undefined", value = 0 },
            { name = "ShaderResource", value = 1 },
            { name = "UnorderedAccess", value = 2 },
            { name = "CopySource", value = 3 },
            { name = "CopyDestination", value = 4 },
            { name = "RenderTarget", value = 5 },
            { name = "DepthWrite", value = 6 },
        },
    },
    {
        name = "ResourceUsage",
        cpp_name = "HGBCResourceUsage",
        values = {
            { name = "ShaderResource", value = 0 },
            { name = "UnorderedAccess", value = 1 },
            { name = "CopySource", value = 2 },
            { name = "CopyDestination", value = 3 },
            { name = "VertexBuffer", value = 4 },
            { name = "IndexBuffer", value = 5 },
            { name = "AccelerationStructureBuildInput", value = 6 },
            { name = "RenderTarget", value = 7 },
            { name = "DepthStencil", value = 8 },
        },
    },
}

local function operand(name, kind, enum_name, is_ref, is_implicit)
    return {
        name = name,
        kind = kind,
        enum_name = enum_name,
        is_ref = is_ref or false,
        is_implicit = is_implicit or false,
    }
end

local function operands(required, optional, repeated)
    return {
        required = required or {},
        optional = optional or {},
        repeated = repeated or {},
    }
end

local function opcode(name, value, section, args)
    args = args or {}
    args.name = name
    args.asm_name = "Op" .. name
    args.value = value
    args.section = section
    args.operands = args.operands or operands()
    return args
end

local id = function(name)
    return operand(name, "Id")
end
local ref_id = function(name)
    return operand(name, "Id", nil, true)
end
-- An implicit operand exists in serialized HGBC but not in source hgbc_asm. For example,
-- OpAllocateResource receives a compiler-created device TypeLayout after its source operands have
-- been lowered, so library code never needs to manufacture metadata handles at runtime.
local implicit_id = function(name)
    return operand(name, "Id", nil, false, true)
end
local type_id = function(name)
    return operand(name, "TypeId")
end
local result_id = function(name)
    return operand(name, "ResultId")
end
local uint = function(name)
    return operand(name, "UInt")
end
local boolean = function(name)
    return operand(name, "Bool")
end
local float_bits = function(name)
    return operand(name, "FloatBits")
end
local byte_count = function(name)
    return operand(name, "ByteCount")
end
local enum = function(name, enum_name)
    return operand(name, "Enum", enum_name)
end

M.opcodes = {
    opcode("BoxType", 0xc6, "TYPE", {
        compiler_owned = true, opaque_type = true, operands = operands({ result_id("result") }),
    }),
    opcode("TaggedUnionType", 0xc7, "TYPE", {
        compiler_owned = true, opaque_type = true,
        operands = operands({ result_id("result") }, nil,
            { uint("caseTag"), type_id("payloadType") }),
    }),
    opcode("Box", 0xc8, "FUNC", {
        compiler_owned = true,
        operands = operands({ type_id("type"), result_id("result"),
            type_id("payloadType"), ref_id("payload") }),
    }),
    opcode("Unbox", 0xc9, "FUNC", {
        compiler_owned = true,
        operands = operands({ type_id("type"), result_id("result"), ref_id("box") }),
    }),
    opcode("MakeTaggedUnion", 0xca, "FUNC", {
        compiler_owned = true,
        operands = operands({ type_id("type"), result_id("result"),
            ref_id("caseTag"), ref_id("box") }),
    }),
    opcode("GetTaggedUnionCase", 0xcb, "FUNC", {
        compiler_owned = true,
        operands = operands({ type_id("type"), result_id("result"), ref_id("value") }),
    }),
    opcode("GetTaggedUnionPayload", 0xcc, "FUNC", {
        compiler_owned = true,
        operands = operands({ type_id("type"), result_id("result"), ref_id("value") }),
    }),
    opcode("ExtractTaggedUnion", 0xcd, "FUNC", {
        compiler_owned = true,
        operands = operands({ type_id("type"), result_id("result"),
            ref_id("value"), uint("caseTag") }),
    }),
    opcode("ConvertTaggedUnion", 0xce, "FUNC", {
        compiler_owned = true,
        operands = operands({ type_id("type"), result_id("result"), ref_id("value") }, nil,
            { uint("sourceTag"), uint("destinationTag") }),
    }),
    opcode("RaytracingBottomLevelAccelerationStructureType", 0xc3, "TYPE", {
        compiler_owned = true, opaque_type = true, operands = operands({ result_id("result") }),
    }),
    opcode("AllocAccelerationStructure", 0xc4, "FUNC", {
        inline_asm = true,
        operands = operands({ type_id("type"), result_id("result"),
            enum("kind", "AccelerationStructureKind") }),
    }),
    opcode("BuildAccelerationStructure", 0xc5, "FUNC", {
        inline_asm = true,
        operands = operands({ enum("kind", "AccelerationStructureKind"),
            enum("inputKind", "AccelerationStructureInputKind"), type_id("inputType"),
            ref_id("structure"), ref_id("inputs"), ref_id("buildFlags"), ref_id("buildMode") }),
    }),
    opcode("RaytracingPipelineType", 0xbd, "TYPE", {
        compiler_owned = true, opaque_type = true, operands = operands({ result_id("result") }),
    }),
    opcode("MakeRaytracingPipeline", 0xbe, "FUNC", {
        inline_asm = true,
        operands = operands({ type_id("type"), result_id("result"), ref_id("rayTypeCount") }),
    }),
    opcode("SetRayMiss", 0xbf, "FUNC", {
        compiler_owned = true,
        operands = operands({ ref_id("pipeline"), ref_id("raySlot"), id("entryPoint"),
            type_id("payloadType") }, nil,
            { uint("parameterIndex"), type_id("argumentType"), ref_id("argument") }),
    }),
    opcode("RayHitGroup", 0xc0, "EMBD", {
        compiler_owned = true,
        operands = operands({ result_id("result"), enum("geometryKind", "RayGeometryKind"),
            type_id("payloadType"), type_id("attributeType") }, nil,
            { enum("stageRole", "RayStageRole"), id("entryPoint") }),
    }),
    opcode("SetRayHitGroup", 0xc1, "FUNC", {
        compiler_owned = true,
        operands = operands({ ref_id("pipeline"), ref_id("geometrySlot"), ref_id("raySlot"),
            id("hitGroup") }, nil,
            { enum("stageRole", "RayStageRole"), uint("parameterIndex"),
                type_id("argumentType"), ref_id("argument") }),
    }),
    opcode("DispatchRays", 0xc2, "FUNC", {
        compiler_owned = true,
        operands = operands({ id("entryPoint"), ref_id("extent"), uint("maxRecursion") }, nil,
            { uint("parameterIndex"), type_id("argumentType"), ref_id("argument") }),
    }),
    opcode("GraphicsPipelineState", 0xb3, "EMBD", {
        compiler_owned = true, named_operands = true,
        operands = operands({
            result_id("result"), enum("topology", "PrimitiveTopology"),
            uint("fillMode"), uint("cullMode"), uint("frontFace"), uint("depthBias"),
            float_bits("depthBiasClamp"), float_bits("slopeScaledDepthBias"),
            boolean("depthClipEnable"), boolean("scissorEnable"),
            boolean("multisampleEnable"), boolean("antialiasedLineEnable"),
            boolean("enableConservativeRasterization"), uint("forcedSampleCount"),
            boolean("depthTestEnable"), boolean("depthWriteEnable"),
            enum("depthFunc", "ComparisonFunc"), boolean("stencilEnable"),
            uint("stencilReadMask"), uint("stencilWriteMask"),
            uint("frontStencilFailOp"), uint("frontStencilDepthFailOp"),
            uint("frontStencilPassOp"), enum("frontStencilFunc", "ComparisonFunc"),
            uint("backStencilFailOp"), uint("backStencilDepthFailOp"),
            uint("backStencilPassOp"), enum("backStencilFunc", "ComparisonFunc"),
            uint("sampleMask"), boolean("alphaToCoverageEnable"), boolean("alphaToOneEnable"),
        }),
    }),
    opcode("GraphicsColorTargetState", 0xb4, "EMBD", {
        compiler_owned = true, named_operands = true,
        operands = operands({
            result_id("result"), type_id("textureType"), boolean("enableBlend"),
            uint("logicOp"), uint("writeMask"), uint("colorSrcFactor"), uint("colorDstFactor"),
            uint("colorOp"), uint("alphaSrcFactor"), uint("alphaDstFactor"), uint("alphaOp"),
        }),
    }),
    opcode("GraphicsAttachments", 0xb5, "EMBD", {
        compiler_owned = true,
        operands = operands({ result_id("result"), enum("depthFormat", "ResourceFormat") },
            nil, { id("colorTargetState") }),
    }),
    opcode("MakeGraphicsPipeline", 0xb6, "EMBD", {
        compiler_owned = true,
        operands = operands({
            type_id("type"), result_id("result"), id("vertexEntryPoint"),
            id("fragmentEntryPoint"), id("state"), id("attachments"),
            uint("drawParameterCount"), uint("vertexArgumentCount"),
        }, nil, { id("argumentConverter") }),
    }),
    opcode("GraphicsDrawType", 0xb7, "TYPE", {
        compiler_owned = true, opaque_type = true, operands = operands({ result_id("result") }),
    }),
    opcode("MakeGraphicsDraw", 0xb8, "FUNC", {
        inline_asm = true,
        operands = operands({ type_id("type"), result_id("result"),
            ref_id("count"), ref_id("first"), ref_id("instanceCount"),
            ref_id("firstInstance"), ref_id("baseVertex"),
        }, { ref_id("indices") }),
    }),
    opcode("GraphicsDrawParametersType", 0xb9, "TYPE", {
        compiler_owned = true, opaque_type = true, operands = operands({ result_id("result") }),
    }),
    opcode("MakeGraphicsDrawParameters", 0xba, "FUNC", {
        inline_asm = true,
        operands = operands({ type_id("type"), result_id("result"),
            ref_id("useDefaultViewport"), ref_id("useDefaultScissor"),
            ref_id("originX"), ref_id("originY"), ref_id("extentX"), ref_id("extentY"),
            ref_id("minZ"), ref_id("maxZ"),
            ref_id("minX"), ref_id("minY"), ref_id("maxX"), ref_id("maxY"),
            ref_id("stencilRef"),
        }),
    }),
    opcode("ClearTexture", 0xbb, "FUNC", {
        inline_asm = true, named_operands = true,
        operands = operands({
            type_id("textureType"), ref_id("texture"), enum("aspect", "TextureAspect"),
            ref_id("mip"), ref_id("mipCount"), ref_id("layer"), ref_id("layerCount"),
            ref_id("clearValue"),
        }, { ref_id("stencilValue") }),
    }),
    opcode(
        "Export",
        0x02,
        "EXPT",
        {
            compiler_owned = true,
            operands = operands(
                {
                    result_id("result"),
                    enum("kind", "ExportKind"),
                    id("name"),
                    id("target"),
                },
                { type_id("ownerType") }
            ),
        }
    ),
    opcode(
        "Decorate",
        0x03,
        "META",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                enum("kind", "DecorationKind"),
                id("target"),
                id("value"),
            }),
        }
    ),
    opcode(
        "TypeSize",
        0x04,
        "META",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                type_id("type"),
                uint("byteSize"),
                uint("byteAlignment"),
                uint("byteStride"),
            }),
        }
    ),

    opcode(
        "UIntType",
        0x10,
        "TYPE",
        { compiler_owned = true, operands = operands({ result_id("result") }) }
    ),
    opcode(
        "Float32Type",
        0x11,
        "TYPE",
        { compiler_owned = true, operands = operands({ result_id("result") }) }
    ),
    opcode(
        "Float16Type",
        0x2c,
        "TYPE",
        { compiler_owned = true, operands = operands({ result_id("result") }) }
    ),
    opcode(
        "DescriptorHandleType",
        0x2d,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({ result_id("result"), type_id("resourceType") }),
        }
    ),
    opcode(
        "Texture2DType",
        0x2e,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                type_id("elementType"),
                enum("access", "TextureAccess"),
                enum("format", "ResourceFormat"),
            }),
        }
    ),
    opcode(
        "VoidType",
        0x12,
        "TYPE",
        { compiler_owned = true, operands = operands({ result_id("result") }) }
    ),
    opcode(
        "RWStructuredBufferType",
        0x13,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({ result_id("result"), type_id("elementType") }),
        }
    ),
    opcode(
        "VectorType",
        0x14,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                uint("elementCount"),
                type_id("elementType"),
            }),
        }
    ),
    opcode(
        "FuncType",
        0x15,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands(
                { result_id("result"), type_id("returnType") },
                nil,
                { type_id("parameterType") }
            ),
        }
    ),
    opcode(
        "BoolType",
        0x16,
        "TYPE",
        { compiler_owned = true, operands = operands({ result_id("result") }) }
    ),
    opcode(
        "StructType",
        0x17,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands(
                { result_id("result") },
                nil,
                { id("fieldName"), type_id("fieldType") }
            ),
        }
    ),
    opcode(
        "RenderTarget2DType",
        0x18,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                type_id("elementType"),
                enum("format", "ResourceFormat"),
            }),
        }
    ),
    opcode(
        "StructuredBufferType",
        0x19,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({ result_id("result"), type_id("elementType") }),
        }
    ),
    opcode(
        "OutType",
        0x1a,
        "TYPE",
        {
            compiler_owned = true,
            address_type = true,
            operands = operands({ result_id("result"), type_id("valueType") }),
        }
    ),
    opcode(
        "InOutType",
        0x1b,
        "TYPE",
        {
            compiler_owned = true,
            address_type = true,
            operands = operands({ result_id("result"), type_id("valueType") }),
        }
    ),
    opcode(
        "MatrixType",
        0x1c,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                uint("rowCount"),
                uint("columnCount"),
                uint("layout"),
                type_id("elementType"),
            }),
        }
    ),
    opcode(
        "DeferType",
        0x1d,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({ result_id("result"), type_id("valueType") }),
        }
    ),
    opcode(
        "RaytracingAccelerationStructureType",
        0x9a,
        "TYPE",
        {
            compiler_owned = true,
            opaque_type = true,
            operands = operands({ result_id("result") }),
        }
    ),
    opcode(
        "WindowType",
        0x1e,
        "TYPE",
        {
            compiler_owned = true,
            opaque_type = true,
            operands = operands({ result_id("result") }),
        }
    ),
    opcode(
        "SurfaceType",
        0x1f,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({ result_id("result"), enum("format", "ResourceFormat") }),
        }
    ),

    opcode(
        "IntLit",
        0x20,
        "LITR",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                uint("value"),
            }),
        }
    ),
    opcode(
        "StringLit",
        0x21,
        "LITR",
        {
            compiler_owned = true,
            payload_size_operand = "byteCount",
            operands = operands({
                type_id("type"),
                result_id("result"),
                byte_count("byteCount"),
            }),
        }
    ),
    opcode(
        "BinaryLit",
        0x22,
        "LITR",
        {
            compiler_owned = true,
            payload_size_operand = "byteCount",
            operands = operands({ result_id("result"), byte_count("byteCount") }),
        }
    ),
    opcode(
        "BoolLit",
        0x23,
        "LITR",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                boolean("value"),
            }),
        }
    ),
    opcode(
        "FloatLit",
        0x24,
        "LITR",
        {
            inline_asm = true,
            -- Binary16/32 use one word; binary64 appends the high word of its IEEE bits.
            operands = operands({
                type_id("type"),
                result_id("result"),
                float_bits("value"),
            }, { float_bits("valueHigh") }),
        }
    ),
    opcode(
        "StringType",
        0x25,
        "TYPE",
        { compiler_owned = true, operands = operands({ result_id("result") }) }
    ),
    opcode(
        "TimerType",
        0x26,
        "TYPE",
        {
            compiler_owned = true,
            opaque_type = true,
            operands = operands({ result_id("result") }),
        }
    ),
    opcode(
        "ArrayType",
        0x27,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                uint("elementCount"),
                type_id("elementType"),
            }),
        }
    ),
    opcode(
        "IntType",
        0x28,
        "TYPE",
        { compiler_owned = true, operands = operands({ result_id("result") }) }
    ),
    opcode(
        "ClassType",
        0x29,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands(
                { result_id("result") },
                nil,
                { id("fieldName"), type_id("fieldType") }
            ),
        }
    ),
    opcode(
        "RuntimeArrayType",
        0x2a,
        "TYPE",
        {
            compiler_owned = true,
            operands = operands({ result_id("result"), type_id("elementType") }),
        }
    ),
    opcode(
        "NullLit",
        0x2b,
        "LITR",
        {
            inline_asm = true,
            operands = operands({ type_id("type"), result_id("result") }),
        }
    ),

    opcode(
        "Func",
        0x30,
        "FUNC",
        {
            compiler_owned = true,
            operands = operands({ type_id("type"), result_id("result") }),
        }
    ),
    opcode(
        "Block",
        0x31,
        "FUNC",
        { compiler_owned = true, operands = operands({ result_id("result") }) }
    ),
    opcode(
        "Param",
        0x32,
        "FUNC",
        {
            compiler_owned = true,
            operands = operands({ type_id("type"), result_id("result") }),
        }
    ),
    opcode(
        "Return",
        0x33,
        "FUNC",
        { compiler_owned = true, operands = operands(nil, { ref_id("value") }) }
    ),
    opcode(
        "Print",
        0x34,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({ type_id("valueType"), ref_id("value") }),
        }
    ),
    opcode("FunctionEnd", 0x35, "FUNC", { compiler_owned = true }),
    opcode("BlockEnd", 0x36, "FUNC", { compiler_owned = true }),
    opcode(
        "Branch",
        0x37,
        "FUNC",
        {
            compiler_owned = true,
            operands = operands({ id("targetBlock") }),
        }
    ),
    opcode(
        "BranchConditional",
        0x38,
        "FUNC",
        {
            compiler_owned = true,
            operands = operands({
                ref_id("condition"),
                id("trueBlock"),
                id("falseBlock"),
            }),
        }
    ),
    opcode(
        "Var",
        0x39,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({ type_id("type"), result_id("result") }),
        }
    ),
    opcode(
        "Load",
        0x3a,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("pointer"),
            }),
        }
    ),
    opcode(
        "Store",
        0x3b,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({ ref_id("pointer"), ref_id("value") }),
        }
    ),
    opcode(
        "FieldAddress",
        0x3c,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("base"),
                id("fieldName"),
            }),
        }
    ),
    opcode(
        "ElementAddress",
        0x3d,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("base"),
                ref_id("index"),
            }),
        }
    ),
    opcode(
        "AllocateObject",
        0x3e,
        "FUNC",
        {
            compiler_owned = true,
            operands = operands({ type_id("type"), result_id("result") }),
        }
    ),
    opcode(
        "AllocateRuntimeArray",
        0x3f,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({ type_id("type"), result_id("result"), ref_id("elementCount") }),
        }
    ),
    opcode(
        "RuntimeArrayGetCount",
        0x40,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("arrayType"),
                ref_id("array"),
            }),
        }
    ),
    opcode(
        "CopyArrayRange",
        0x41,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("destinationType"),
                type_id("sourceType"),
                implicit_id("converter"),
                ref_id("destination"),
                ref_id("destinationOffset"),
                ref_id("source"),
                ref_id("sourceOffset"),
                ref_id("elementCount"),
            }),
        }
    ),

    opcode(
        "ExtractElement",
        0x43,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("base"),
                ref_id("index"),
            }),
        }
    ),
    opcode(
        "StructuredBufferGetDimensions",
        0x44,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("bufferType"),
                ref_id("buffer"),
            }),
        }
    ),
    opcode(
        "AllocateResource",
        0x45,
        "FUNC",
        {
            inline_asm = true,
            operands = operands(
                {
                    type_id("type"),
                    result_id("result"),
                    enum("kind", "ResourceKind"),
                    enum("dimension", "ResourceDimension"),
                    enum("format", "ResourceFormat"),
                    enum("memoryType", "ResourceMemoryType"),
                    enum("initialState", "ResourceState"),
                    type_id("elementType"),
                    ref_id("widthOrElementCount"),
                    ref_id("height"),
                    ref_id("depth"),
                    uint("mipCount"),
                    uint("sampleCount"),
                    uint("sampleQuality"),
                    implicit_id("layout"),
                },
                nil,
                { enum("usage", "ResourceUsage") }
            ),
        }
    ),
    opcode(
        "TextureGetDimensions",
        0x46,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("texture"),
            }),
        }
    ),
    opcode(
        "ReadBackBufferElement",
        0x47,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("bufferType"),
                implicit_id("converter"),
                ref_id("buffer"),
                ref_id("index"),
            }),
        }
    ),
    opcode(
        "AsyncReadBackBufferElement",
        0x48,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("bufferType"),
                implicit_id("converter"),
                ref_id("buffer"),
                ref_id("index"),
            }),
        }
    ),
    opcode(
        "WaitDefer",
        0x49,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("deferred"),
            }),
        }
    ),
    opcode(
        "ResourceToDescriptorHandle",
        0x4a,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("resource"),
            }),
        }
    ),
    opcode(
        "BlitTexture2D",
        0x4b,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("destinationType"),
                type_id("sourceType"),
                ref_id("destination"),
                ref_id("source"),
            }),
        }
    ),
    opcode(
        "ImageDecode",
        0x4c,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                enum("codec", "ImageCodec"),
                enum("sourceKind", "EncodedImageEndpointKind"),
                enum("destinationKind", "PixelImageEndpointKind"),
                ref_id("source"),
                ref_id("destination"),
            }),
        }
    ),
    opcode(
        "ImageEncode",
        0x4d,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                enum("codec", "ImageCodec"),
                enum("sourceKind", "PixelImageEndpointKind"),
                enum("destinationKind", "EncodedImageEndpointKind"),
                ref_id("source"),
                ref_id("destination"),
            }),
        }
    ),

    opcode(
        "Add",
        0x50,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "IDiv",
        0x51,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "MakeVector",
        0x52,
        "FUNC",
        {
            inline_asm = true,
            operands = operands(
                { type_id("type"), result_id("result") },
                nil,
                { ref_id("element") }
            ),
        }
    ),
    opcode(
        "Mul",
        0x53,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "Eql",
        0x54,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "Not",
        0x55,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Neq",
        0x56,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "Less",
        0x57,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "Greater",
        0x58,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "Leq",
        0x59,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "Geq",
        0x5a,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "FDiv",
        0x5b,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "ScalarTypeCast",
        0x5c,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                result_id("result"),
                type_id("sourceType"),
                type_id("type"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Sin",
        0x5d,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Cos",
        0x5e,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Tan",
        0x5f,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),

    opcode(
        "MakeDispatcher",
        0x60,
        "EMBD",
        {
            compiler_owned = true,
            operands = operands(
                {
                    type_id("type"),
                    result_id("result"),
                    enum("stage", "DispatchStage"),
                    id("entryPoint"),
                },
                nil,
                { id("argumentConverter") }
            ),
        }
    ),
    opcode(
        "Call",
        0x61,
        "FUNC",
        {
            compiler_owned = true,
            operands = operands(
                { type_id("type"), result_id("result"), ref_id("callee") },
                nil,
                { ref_id("argument") }
            ),
        }
    ),
    opcode(
        "MakeStruct",
        0x62,
        "FUNC",
        {
            inline_asm = true,
            operands = operands(
                { type_id("type"), result_id("result") },
                nil,
                { ref_id("fieldValue") }
            ),
        }
    ),
    opcode(
        "FieldExtract",
        0x63,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("base"),
                id("fieldName"),
            }),
        }
    ),
    opcode(
        "MakeRayTracer",
        0x9b,
        "EMBD",
        {
            compiler_owned = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("payloadType"),
                uint("maxRecursion"),
                id("rayGenerationEntryPoint"),
                id("closestHitEntryPoint"),
                id("missEntryPoint"),
            }),
        }
    ),
    opcode(
        "CreateRaytracingScene",
        0x9c,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                enum("geometryKind", "RaytracingGeometryKind"),
                enum("vertexFormat", "RaytracingVertexFormat"),
                enum("indexFormat", "RaytracingIndexFormat"),
                ref_id("vertexCount"),
                ref_id("vertices"),
                ref_id("indexCount"),
                ref_id("indices"),
            }),
        }
    ),
    opcode(
        "MakeArray",
        0x9d,
        "FUNC",
        {
            inline_asm = true,
            operands = operands(
                { type_id("type"), result_id("result") },
                nil,
                { ref_id("element") }
            ),
        }
    ),
    opcode(
        "Sqrt",
        0x65,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "MakeMatrix",
        0x66,
        "FUNC",
        {
            inline_asm = true,
            operands = operands(
                { type_id("type"), result_id("result") },
                nil,
                { ref_id("element") }
            ),
        }
    ),
    opcode(
        "CreateWindow",
        0x67,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("title"),
                ref_id("width"),
                ref_id("height"),
            }),
        }
    ),
    opcode(
        "ProcessWindowEvents",
        0x68,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("window"),
            }),
        }
    ),
    opcode(
        "CreateSurface",
        0x69,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("window"),
                ref_id("imageCount"),
            }),
        }
    ),
    opcode(
        "AcquireSurfaceTexture",
        0x6a,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("surface"),
            }),
        }
    ),
    opcode(
        "PresentSurface",
        0x6b,
        "FUNC",
        { inline_asm = true, operands = operands({ ref_id("surface") }) }
    ),
    opcode(
        "ByteStreamRead",
        0x6c,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                enum("destinationKind", "ByteStreamEndpointKind"),
                ref_id("stream"),
                ref_id("destination"),
                ref_id("destinationByteOffset"),
                ref_id("byteCount"),
            }),
        }
    ),
    opcode(
        "ByteStreamWrite",
        0x6d,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                enum("sourceKind", "ByteStreamEndpointKind"),
                ref_id("stream"),
                ref_id("source"),
                ref_id("sourceByteOffset"),
                ref_id("byteCount"),
            }),
        }
    ),
    opcode(
        "PollInputEvent",
        0x6e,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("window"),
            }),
        }
    ),
    opcode(
        "CloseWindow",
        0x6f,
        "FUNC",
        { inline_asm = true, operands = operands({ ref_id("window") }) }
    ),

    opcode(
        "SlangModule",
        0x70,
        "EMBD",
        {
            compiler_owned = true,
            operands = operands(
                { result_id("result"), id("name"), id("binary") },
                nil,
                { id("dependency") }
            ),
        }
    ),
    opcode(
        "SlangEntryPoint",
        0x71,
        "EMBD",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                id("module"),
                id("entryPointName"),
            }),
        }
    ),
    opcode(
        "SpecializeSlangEntryPoint",
        0x72,
        "EMBD",
        {
            compiler_owned = true,
            operands = operands(
                { result_id("result"), id("baseEntryPoint") },
                nil,
                {
                    enum("argumentKind", "SpecializationArgKind"),
                    id("argument"),
                }
            ),
        }
    ),
    opcode(
        "Sub",
        0x73,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "Pow",
        0x74,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("base"),
                ref_id("exponent"),
            }),
        }
    ),
    opcode(
        "FormatString",
        0x75,
        "FUNC",
        {
            inline_asm = true,
            operands = operands(
                { type_id("type"), result_id("result"), ref_id("pattern") },
                nil,
                { type_id("argumentType"), ref_id("argument") }
            ),
        }
    ),
    opcode(
        "SetWindowTitle",
        0x76,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({ ref_id("window"), ref_id("title") }),
        }
    ),
    opcode(
        "CreateTimer",
        0x77,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({ type_id("type"), result_id("result") }),
        }
    ),
    opcode(
        "TimerElapsed",
        0x78,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("timer"),
            }),
        }
    ),
    opcode(
        "ResetTimer",
        0x79,
        "FUNC",
        { inline_asm = true, operands = operands({ ref_id("timer") }) }
    ),
    opcode(
        "Neg",
        0x7a,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "ByteStreamType",
        0x7b,
        "TYPE",
        {
            compiler_owned = true,
            opaque_type = true,
            operands = operands({ result_id("result") }),
        }
    ),
    opcode(
        "UploadBufferStorageType",
        0x7c,
        "TYPE",
        {
            compiler_owned = true,
            opaque_type = true,
            operands = operands({ result_id("result") }),
        }
    ),
    opcode(
        "ByteStreamFromFile",
        0x7d,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                enum("access", "ByteStreamAccess"),
                ref_id("path"),
            }),
        }
    ),
    opcode(
        "CloseByteStream",
        0x7e,
        "FUNC",
        { inline_asm = true, operands = operands({ ref_id("stream") }) }
    ),
    opcode(
        "ByteStreamGetSize",
        0x7f,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("stream"),
            }),
        }
    ),
    opcode(
        "UploadBufferStore",
        0x85,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("elementType"),
                ref_id("buffer"),
                ref_id("index"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "UploadBufferUploadTo",
        0x86,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                ref_id("buffer"),
                ref_id("destination"),
            }),
        }
    ),
    opcode(
        "Fail",
        0x87,
        "FUNC",
        { inline_asm = true, operands = operands({ ref_id("message") }) }
    ),
    opcode(
        "StringGetByteCount",
        0x88,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "StringReadByte",
        0x89,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
                ref_id("index"),
            }),
        }
    ),
    opcode(
        "DebugSource",
        0x8c,
        "META",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({
                result_id("result"),
                id("path"),
                id("content"),
            }),
        }
    ),
    opcode(
        "DebugLambda",
        0xbc,
        "META",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({
                result_id("result"),
                id("name"),
                id("module"),
                id("source"),
                uint("line"),
                uint("column"),
            }),
        }
    ),
    opcode(
        "DebugFunction",
        0x8d,
        "META",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({
                result_id("result"),
                id("name"),
                id("source"),
                uint("line"),
                uint("column"),
            }),
        }
    ),
    opcode(
        "DebugInlinedAt",
        0x8e,
        "META",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({
                result_id("result"),
                id("scope"),
                id("source"),
                uint("line"),
                uint("column"),
                id("parent"),
            }),
        }
    ),
    opcode(
        "DebugLine",
        0x8f,
        "FUNC",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({
                id("source"),
                uint("lineStart"),
                uint("lineEnd"),
                uint("columnStart"),
                uint("columnEnd"),
            }),
        }
    ),
    opcode(
        "DebugVariable",
        0x90,
        "FUNC",
        {
            compiler_owned = true,
            debug = true,
            operands = operands(
                {
                    result_id("result"),
                    id("name"),
                    id("type"),
                    id("source"),
                    uint("line"),
                    uint("column"),
                    id("scope"),
                },
                { uint("argumentIndexPlusOne") }
            ),
        }
    ),
    opcode(
        "DebugValue",
        0x91,
        "FUNC",
        {
            compiler_owned = true,
            debug = true,
            -- Selectors name struct fields or element index values, in root-to-leaf order.
            operands = operands(
                { id("variable") },
                { id("value") },
                { type_id("selectedType"), id("selector") }
            ),
        }
    ),
    opcode(
        "DebugLexicalBlock",
        0x92,
        "META",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({
                result_id("result"),
                id("parent"),
                id("source"),
                uint("lineStart"),
                uint("columnStart"),
                uint("lineEnd"),
                uint("columnEnd"),
            }),
        }
    ),
    opcode(
        "DebugScope",
        0x93,
        "FUNC",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({ id("scope") }, { id("inlinedAt") }),
        }
    ),
    opcode(
        "DebugNoScope",
        0x94,
        "FUNC",
        {
            compiler_owned = true,
            debug = true,
        }
    ),
    opcode(
        "DebugBuildIdentifier",
        0x95,
        "META",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({
                result_id("result"),
                id("identifier"),
                uint("flags"),
            }),
        }
    ),
    opcode(
        "DebugCompilationUnit",
        0x96,
        "META",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({ result_id("result"), id("source") }),
        }
    ),
    opcode(
        "DebugInlinedVariable",
        0x97,
        "META",
        {
            compiler_owned = true,
            debug = true,
            operands = operands({
                result_id("result"),
                id("variable"),
                id("inlinedAt"),
            }),
        }
    ),
    opcode(
        "PtrType",
        0x9e,
        "TYPE",
        {
            compiler_owned = true,
            address_type = true,
            operands = operands({ result_id("result"), type_id("valueType") }),
        }
    ),
    opcode(
        "RefType",
        0x9f,
        "TYPE",
        {
            compiler_owned = true,
            address_type = true,
            operands = operands({ result_id("result"), type_id("valueType") }),
        }
    ),
    opcode(
        "BorrowInType",
        0xa0,
        "TYPE",
        {
            compiler_owned = true,
            address_type = true,
            operands = operands({ result_id("result"), type_id("valueType") }),
        }
    ),
    opcode(
        "TypeLayout",
        0xa1,
        "EMBD",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                type_id("logicalType"),
                enum("domain", "LayoutDomain"),
                enum("usage", "LayoutUsage"),
                id("context"),
                -- ShaderObject uses the original reflected entry-point parameter index, not
                -- an ordinal among resource-consuming parameters. Empty parameters have indices.
                uint("contextIndex"),
            }, { uint("entryPointIndex") }),
        }
    ),
    opcode(
        "LayoutConverter",
        0xa2,
        "EMBD",
        {
            compiler_owned = true,
            operands = operands({
                result_id("result"),
                id("sourceLayout"),
                id("destinationLayout"),
            }),
        }
    ),
    opcode(
        "IRem",
        0xa3,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "DescriptorHandleToResource",
        0xa4,
        "FUNC",
        {
            compiler_owned = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("handle"),
            }),
        }
    ),
    opcode(
        "Ceil",
        0xa5,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Floor",
        0xa6,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Exp",
        0xa7,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Exp2",
        0xa8,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Log",
        0xa9,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Log2",
        0xaa,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Asin",
        0xab,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Atan2",
        0xac,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("y"),
                ref_id("x"),
            }),
        }
    ),
    opcode(
        "Lsh",
        0xad,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("value"),
                ref_id("amount"),
            }),
        }
    ),
    opcode(
        "Any",
        0xae,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("value"),
            }),
        }
    ),
    opcode(
        "Min",
        0xaf,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "Max",
        0xb0,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                type_id("operandType"),
                ref_id("left"),
                ref_id("right"),
            }),
        }
    ),
    opcode(
        "Float64Type",
        0xb1,
        "TYPE",
        { compiler_owned = true, operands = operands({ result_id("result") }) }
    ),
    opcode(
        "ResizeSurface",
        0xcf,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("surface"),
                ref_id("width"),
                ref_id("height"),
            }),
        }
    ),
    opcode(
        "TryAcquireSurfaceTexture",
        0xd0,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("surface"),
                ref_id("destination"),
            }),
        }
    ),
    opcode(
        "TryPresentSurface",
        0xd1,
        "FUNC",
        {
            inline_asm = true,
            operands = operands({
                type_id("type"),
                result_id("result"),
                ref_id("surface"),
            }),
        }
    ),
}

local operand_kinds = {
    Id = true,
    TypeId = true,
    ResultId = true,
    UInt = true,
    Bool = true,
    FloatBits = true,
    ByteCount = true,
    Enum = true,
}

local function fail(message)
    error("invalid HGBC opcode schema: " .. message, 0)
end

local function validate()
    local sections = {}
    local section_four_ccs = {}
    for _, section in ipairs(M.sections) do
        if sections[section.name] then
            fail("duplicate section " .. section.name)
        end
        if #section.four_cc ~= 4 then
            fail("section " .. section.name .. " does not have a four-character code")
        end
        if section_four_ccs[section.four_cc] then
            fail("duplicate section four-character code " .. section.four_cc)
        end
        sections[section.name] = section
        section_four_ccs[section.four_cc] = true
    end

    local enums = {}
    for _, enum_desc in ipairs(M.enums) do
        if enums[enum_desc.name] then
            fail("duplicate enum " .. enum_desc.name)
        end
        enums[enum_desc.name] = enum_desc
        local names = {}
        local values = {}
        for _, value in ipairs(enum_desc.values) do
            if names[value.name] then
                fail("duplicate " .. enum_desc.name .. " name " .. value.name)
            end
            if values[value.value] then
                fail("duplicate " .. enum_desc.name .. " value " .. tostring(value.value))
            end
            names[value.name] = true
            values[value.value] = true
        end
    end

    local opcode_names = {}
    local asm_names = {}
    local opcode_values = {}
    for _, op in ipairs(M.opcodes) do
        if opcode_names[op.name] then
            fail("duplicate opcode name " .. op.name)
        end
        if asm_names[op.asm_name] then
            fail("duplicate assembly opcode name " .. op.asm_name)
        end
        if opcode_values[op.value] then
            fail(
                string.format(
                    "duplicate opcode value 0x%02x for %s and %s",
                    op.value,
                    op.name,
                    opcode_values[op.value]
                )
            )
        end
        if op.value < 0 or op.value > 0xffff then
            fail("opcode " .. op.name .. " does not fit in the 16-bit opcode field")
        end
        if not sections[op.section] or not sections[op.section].contains_records then
            fail("opcode " .. op.name .. " references invalid record section " .. op.section)
        end
        opcode_names[op.name] = true
        asm_names[op.asm_name] = true
        opcode_values[op.value] = op.name

        local operand_names = {}
        local result_index = -1
        local type_index = -1
        local payload_size_index = -1
        local flat_operands = {}
        local groups = {
            op.operands.required,
            op.operands.optional,
            op.operands.repeated,
        }
        for _, group in ipairs(groups) do
            for _, operand_desc in ipairs(group) do
                if operand_names[operand_desc.name] then
                    fail("opcode " .. op.name .. " has duplicate operand " .. operand_desc.name)
                end
                if not operand_kinds[operand_desc.kind] then
                    fail(
                        "opcode "
                            .. op.name
                            .. " has unknown operand kind "
                            .. tostring(operand_desc.kind)
                    )
                end
                if operand_desc.kind == "Enum" then
                    if not enums[operand_desc.enum_name] then
                        fail(
                            "opcode "
                                .. op.name
                                .. " references unknown enum "
                                .. tostring(operand_desc.enum_name)
                        )
                    end
                elseif operand_desc.enum_name then
                    fail(
                        "opcode "
                            .. op.name
                            .. " gives non-enum operand "
                            .. operand_desc.name
                            .. " an enum"
                    )
                end
                if operand_desc.is_ref and operand_desc.kind ~= "Id" then
                    fail(
                        "opcode "
                            .. op.name
                            .. " marks non-id operand "
                            .. operand_desc.name
                            .. " as a reference"
                    )
                end

                local index = #flat_operands
                if operand_desc.kind == "ResultId" then
                    if result_index >= 0 then
                        fail("opcode " .. op.name .. " has more than one result operand")
                    end
                    result_index = index
                elseif operand_desc.kind == "TypeId" and operand_desc.name == "type" then
                    if type_index >= 0 then
                        fail("opcode " .. op.name .. " has more than one result type operand")
                    end
                    type_index = index
                end
                if op.payload_size_operand == operand_desc.name then
                    if operand_desc.kind ~= "ByteCount" then
                        fail(
                            "opcode "
                                .. op.name
                                .. " payload size operand must use ByteCount"
                        )
                    end
                    payload_size_index = index
                end
                operand_names[operand_desc.name] = true
                table.insert(flat_operands, operand_desc)
            end
        end

        if #op.operands.required + #op.operands.optional > 255 then
            fail("opcode " .. op.name .. " has too many fixed operands")
        end
        if op.payload_size_operand and payload_size_index < 0 then
            fail(
                "opcode "
                    .. op.name
                    .. " references missing payload size operand "
                    .. op.payload_size_operand
            )
        end
        if payload_size_index >= #op.operands.required then
            fail("opcode " .. op.name .. " payload size operand must be required and fixed")
        end
        if result_index >= #op.operands.required + #op.operands.optional then
            fail("opcode " .. op.name .. " result operand cannot be repeated")
        end
        if type_index >= 0 and result_index < 0 then
            fail("opcode " .. op.name .. " has a result type but no result")
        end
        if op.inline_asm and op.compiler_owned then
            fail("opcode " .. op.name .. " cannot be both inline and compiler-owned")
        end
        if not op.inline_asm and not op.compiler_owned then
            fail("opcode " .. op.name .. " must be inline or compiler-owned")
        end
        if op.opaque_type and not op.compiler_owned then
            fail("opaque type opcode " .. op.name .. " must be compiler-owned")
        end
        if op.address_type and not op.compiler_owned then
            fail("address type opcode " .. op.name .. " must be compiler-owned")
        end
        if op.inline_asm and payload_size_index >= 0 then
            fail("payload opcode " .. op.name .. " cannot be used in hgbc_asm")
        end
        for _, operand_desc in ipairs(op.operands.optional) do
            if operand_desc.is_implicit then
                fail("opcode " .. op.name .. " has an optional implicit operand")
            end
        end
        for _, operand_desc in ipairs(op.operands.repeated) do
            if operand_desc.is_implicit then
                fail("opcode " .. op.name .. " has a repeated implicit operand")
            end
        end
        for _, operand_desc in ipairs(op.operands.required) do
            if operand_desc.is_implicit and not op.inline_asm then
                fail("compiler-owned opcode " .. op.name .. " has an implicit operand")
            end
        end

        op.flat_operands = flat_operands
        op.required_operand_count = #op.operands.required
        op.fixed_operand_count = #op.operands.required + #op.operands.optional
        op.repeated_operand_count = #op.operands.repeated
        op.result_operand_index = result_index
        op.type_operand_index = type_index
        op.payload_size_operand_index = payload_size_index
    end
end

validate()

local function cpp_string(value)
    return '"' .. value:gsub("\\", "\\\\"):gsub('"', '\\"') .. '"'
end

local function hex(value, width)
    return string.format("0x%0" .. tostring(width) .. "x", value)
end

local function emit(line)
    RAW(line or "")
    RAW("\n")
end

function M.emit_cpp()
    emit("enum class HGBCSectionTag : uint32_t")
    emit("{")
    for _, section in ipairs(M.sections) do
        local c = section.four_cc
        emit(
            "    "
                .. section.name
                .. " = makeHGBCFourCC('"
                .. c:sub(1, 1)
                .. "', '"
                .. c:sub(2, 2)
                .. "', '"
                .. c:sub(3, 3)
                .. "', '"
                .. c:sub(4, 4)
                .. "'),"
        )
    end
    emit("};")
    emit("")

    emit("static constexpr HGBCSectionTag kHGBCRecordSections[] =")
    emit("{")
    for _, section in ipairs(M.sections) do
        if section.contains_records then
            emit("    HGBCSectionTag::" .. section.name .. ",")
        end
    end
    emit("};")
    emit("")

    for _, enum_desc in ipairs(M.enums) do
        emit("enum class " .. enum_desc.cpp_name .. " : uint32_t")
        emit("{")
        for _, value in ipairs(enum_desc.values) do
            emit("    " .. value.name .. " = " .. tostring(value.value) .. ",")
        end
        emit("};")
        emit("")
    end

    -- Named fixed operands keep large state records in sync with their consumers.
    for _, op in ipairs(M.opcodes) do
        if op.named_operands then
            emit("struct HGBC" .. op.name .. "Operands")
            emit("{")
            for index, field in ipairs(op.operands.required) do
                emit("    static constexpr size_t " .. field.name .. " = " .. tostring(index - 1) .. ";")
            end
            emit("};")
            emit("")
        end
    end

    emit("enum class HGBCEnumKind : uint8_t")
    emit("{")
    emit("    None,")
    for _, enum_desc in ipairs(M.enums) do
        emit("    " .. enum_desc.name .. ",")
    end
    emit("};")
    emit("")

    emit("enum class HGBCOperandKind : uint8_t")
    emit("{")
    emit("    Id,")
    emit("    TypeId,")
    emit("    ResultId,")
    emit("    UInt,")
    emit("    Bool,")
    emit("    FloatBits,")
    emit("    ByteCount,")
    emit("    Enum,")
    emit("};")
    emit("")

    emit("enum class HGBCOpcodeFlags : uint8_t")
    emit("{")
    emit("    None = 0,")
    emit("    InlineAsm = 1 << 0,")
    emit("    CompilerOwned = 1 << 1,")
    emit("    OpaqueType = 1 << 2,")
    emit("    Debug = 1 << 3,")
    emit("    AddressType = 1 << 4,")
    emit("};")
    emit("")

    emit("enum class HGBCOpcode : uint16_t")
    emit("{")
    for _, op in ipairs(M.opcodes) do
        emit("    " .. op.name .. " = " .. hex(op.value, 2) .. ",")
    end
    emit("};")
    emit("")

    emit("struct HGBCOperandInfo")
    emit("{")
    emit("    const char* name;")
    emit("    HGBCOperandKind kind;")
    emit("    HGBCEnumKind enumKind;")
    emit("    bool isRef;")
    emit("    bool isImplicit;")
    emit("};")
    emit("")
    emit("struct HGBCOpcodeInfo")
    emit("{")
    emit("    HGBCOpcode opcode;")
    emit("    const char* name;")
    emit("    HGBCSectionTag section;")
    emit("    const HGBCOperandInfo* operands;")
    emit("    uint8_t requiredOperandCount;")
    emit("    uint8_t fixedOperandCount;")
    emit("    uint8_t repeatedOperandCount;")
    emit("    int16_t resultOperandIndex;")
    emit("    int16_t typeOperandIndex;")
    emit("    int16_t payloadSizeOperandIndex;")
    emit("    uint8_t flags;")
    emit("};")
    emit("")

    for _, op in ipairs(M.opcodes) do
        if #op.flat_operands > 0 then
            emit("static constexpr HGBCOperandInfo kHGBC" .. op.name .. "Operands[] =")
            emit("{")
            for _, operand_desc in ipairs(op.flat_operands) do
                local enum_kind = operand_desc.enum_name or "None"
                emit(
                    "    {"
                        .. cpp_string(operand_desc.name)
                        .. ", HGBCOperandKind::"
                        .. operand_desc.kind
                        .. ", HGBCEnumKind::"
                        .. enum_kind
                        .. ", "
                        .. tostring(operand_desc.is_ref)
                        .. ", "
                        .. tostring(operand_desc.is_implicit)
                        .. "},"
                )
            end
            emit("};")
            emit("")
        end
    end

    emit("static constexpr HGBCOpcodeInfo kHGBCOpcodeInfos[] =")
    emit("{")
    for _, op in ipairs(M.opcodes) do
        local operands_name = #op.flat_operands > 0 and ("kHGBC" .. op.name .. "Operands") or "nullptr"
        local flags = {}
        if op.inline_asm then
            table.insert(flags, "uint8_t(HGBCOpcodeFlags::InlineAsm)")
        end
        if op.compiler_owned then
            table.insert(flags, "uint8_t(HGBCOpcodeFlags::CompilerOwned)")
        end
        if op.opaque_type then
            table.insert(flags, "uint8_t(HGBCOpcodeFlags::OpaqueType)")
        end
        if op.debug then
            table.insert(flags, "uint8_t(HGBCOpcodeFlags::Debug)")
        end
        if op.address_type then
            table.insert(flags, "uint8_t(HGBCOpcodeFlags::AddressType)")
        end
        local flag_expr = #flags > 0 and table.concat(flags, " | ") or "0"
        emit(
            "    {HGBCOpcode::"
                .. op.name
                .. ", "
                .. cpp_string(op.asm_name)
                .. ", HGBCSectionTag::"
                .. op.section
                .. ", "
                .. operands_name
                .. ", "
                .. op.required_operand_count
                .. ", "
                .. op.fixed_operand_count
                .. ", "
                .. op.repeated_operand_count
                .. ", "
                .. op.result_operand_index
                .. ", "
                .. op.type_operand_index
                .. ", "
                .. op.payload_size_operand_index
                .. ", "
                .. flag_expr
                .. "},"
        )
    end
    emit("};")
    emit("")
    emit("static constexpr size_t kHGBCOpcodeCount = sizeof(kHGBCOpcodeInfos) / sizeof(kHGBCOpcodeInfos[0]);")
    emit("")

    emit("inline const char* getHGBCSectionName(HGBCSectionTag tag)")
    emit("{")
    emit("    switch (tag)")
    emit("    {")
    for _, section in ipairs(M.sections) do
        emit("    case HGBCSectionTag::" .. section.name .. ': return "' .. section.name .. '";')
    end
    emit("    }")
    emit('    return "????";')
    emit("}")
    emit("")

    emit("inline const HGBCOpcodeInfo* getHGBCOpcodeInfo(HGBCOpcode opcode)")
    emit("{")
    emit("    switch (opcode)")
    emit("    {")
    for index, op in ipairs(M.opcodes) do
        emit(
            "    case HGBCOpcode::"
                .. op.name
                .. ": return &kHGBCOpcodeInfos["
                .. (index - 1)
                .. "];"
        )
    end
    emit("    }")
    emit("    return nullptr;")
    emit("}")
    emit("")

    emit("inline bool findHGBCOpcodeByName(const char* name, size_t length, HGBCOpcode& outOpcode)")
    emit("{")
    emit("    for (size_t i = 0; i < kHGBCOpcodeCount; ++i)")
    emit("    {")
    emit("        const HGBCOpcodeInfo& info = kHGBCOpcodeInfos[i];")
    emit("        const size_t infoLength = strlen(info.name);")
    emit("        if (length == infoLength && memcmp(name, info.name, length) == 0)")
    emit("        {")
    emit("            outOpcode = info.opcode;")
    emit("            return true;")
    emit("        }")
    emit("    }")
    emit("    return false;")
    emit("}")
    emit("")

    emit("inline const char* getHGBCOpcodeName(HGBCOpcode opcode)")
    emit("{")
    emit("    const HGBCOpcodeInfo* info = getHGBCOpcodeInfo(opcode);")
    emit('    return info ? info->name : "OpUnknown";')
    emit("}")
    emit("")

    emit("inline bool hasHGBCOpcodeFlag(HGBCOpcode opcode, HGBCOpcodeFlags flag)")
    emit("{")
    emit("    const HGBCOpcodeInfo* info = getHGBCOpcodeInfo(opcode);")
    emit("    return info && (info->flags & uint8_t(flag)) != 0;")
    emit("}")
    emit("")

    emit("inline const HGBCOperandInfo* getHGBCOperandInfo(const HGBCOpcodeInfo& info, size_t index)")
    emit("{")
    emit("    if (index < info.fixedOperandCount)")
    emit("        return &info.operands[index];")
    emit("    if (!info.repeatedOperandCount)")
    emit("        return nullptr;")
    emit(
        "    return &info.operands[info.fixedOperandCount + "
            .. "((index - info.fixedOperandCount) % info.repeatedOperandCount)];"
    )
    emit("}")
    emit("")

    emit("inline const HGBCOperandInfo* getHGBCOperandInfo(HGBCOpcode opcode, size_t index)")
    emit("{")
    emit("    const HGBCOpcodeInfo* info = getHGBCOpcodeInfo(opcode);")
    emit("    return info ? getHGBCOperandInfo(*info, index) : nullptr;")
    emit("}")
    emit("")

    emit("inline bool isValidHGBCOperandCount(const HGBCOpcodeInfo& info, size_t count)")
    emit("{")
    emit("    if (count > 255 || count < info.requiredOperandCount)")
    emit("        return false;")
    emit("    if (count <= info.fixedOperandCount)")
    emit("        return true;")
    emit("    return info.repeatedOperandCount &&")
    emit("           ((count - info.fixedOperandCount) % info.repeatedOperandCount) == 0;")
    emit("}")
    emit("")

    emit("inline bool isValidHGBCOperandCount(HGBCOpcode opcode, size_t count)")
    emit("{")
    emit("    const HGBCOpcodeInfo* info = getHGBCOpcodeInfo(opcode);")
    emit("    return info && isValidHGBCOperandCount(*info, count);")
    emit("}")
    emit("")

    emit("// Maps source hgbc_asm operands onto the serialized operand schema. Consider")
    emit("// `OpAllocateResource ...`: the source omits its compiler-created TypeLayout, while the")
    emit("// serialized record includes that layout immediately before the repeated usage operands.")
    emit("inline size_t getHGBCInlineAsmRequiredOperandCount(const HGBCOpcodeInfo& info)")
    emit("{")
    emit("    size_t count = 0;")
    emit("    for (size_t i = 0; i < info.requiredOperandCount; ++i)")
    emit("        count += info.operands[i].isImplicit ? 0 : 1;")
    emit("    return count;")
    emit("}")
    emit("")
    emit("inline size_t getHGBCInlineAsmFixedOperandCount(const HGBCOpcodeInfo& info)")
    emit("{")
    emit("    size_t count = 0;")
    emit("    for (size_t i = 0; i < info.fixedOperandCount; ++i)")
    emit("        count += info.operands[i].isImplicit ? 0 : 1;")
    emit("    return count;")
    emit("}")
    emit("")
    emit("inline int getHGBCInlineAsmSourceOperandIndex(")
    emit("    const HGBCOpcodeInfo& info,")
    emit("    size_t wireIndex)")
    emit("{")
    emit("    if (wireIndex >= info.fixedOperandCount || info.operands[wireIndex].isImplicit)")
    emit("        return -1;")
    emit("    int sourceIndex = 0;")
    emit("    for (size_t i = 0; i < wireIndex; ++i)")
    emit("        sourceIndex += info.operands[i].isImplicit ? 0 : 1;")
    emit("    return sourceIndex;")
    emit("}")
    emit("")
    emit("inline const HGBCOperandInfo* getHGBCInlineAsmOperandInfo(")
    emit("    const HGBCOpcodeInfo& info,")
    emit("    size_t sourceIndex)")
    emit("{")
    emit("    size_t sourceFixedIndex = 0;")
    emit("    for (size_t wireIndex = 0; wireIndex < info.fixedOperandCount; ++wireIndex)")
    emit("    {")
    emit("        const HGBCOperandInfo& operand = info.operands[wireIndex];")
    emit("        if (operand.isImplicit)")
    emit("            continue;")
    emit("        if (sourceFixedIndex++ == sourceIndex)")
    emit("            return &operand;")
    emit("    }")
    emit("    if (!info.repeatedOperandCount || sourceIndex < sourceFixedIndex)")
    emit("        return nullptr;")
    emit("    return &info.operands[info.fixedOperandCount +")
    emit("        ((sourceIndex - sourceFixedIndex) % info.repeatedOperandCount)];")
    emit("}")
    emit("")
    emit("inline bool isValidHGBCInlineAsmOperandCount(")
    emit("    const HGBCOpcodeInfo& info,")
    emit("    size_t count)")
    emit("{")
    emit("    const size_t requiredCount = getHGBCInlineAsmRequiredOperandCount(info);")
    emit("    const size_t fixedCount = getHGBCInlineAsmFixedOperandCount(info);")
    emit("    if (count > 255 || count < requiredCount)")
    emit("        return false;")
    emit("    if (count <= fixedCount)")
    emit("        return true;")
    emit("    return info.repeatedOperandCount &&")
    emit("           ((count - fixedCount) % info.repeatedOperandCount) == 0;")
    emit("}")
    emit("")

    emit("inline int getHGBCResultOperandIndex(HGBCOpcode opcode)")
    emit("{")
    emit("    const HGBCOpcodeInfo* info = getHGBCOpcodeInfo(opcode);")
    emit("    return info ? info->resultOperandIndex : -1;")
    emit("}")
    emit("")
    emit("inline int getHGBCTypeOperandIndex(HGBCOpcode opcode)")
    emit("{")
    emit("    const HGBCOpcodeInfo* info = getHGBCOpcodeInfo(opcode);")
    emit("    return info ? info->typeOperandIndex : -1;")
    emit("}")
    emit("")
    emit("inline int getHGBCPayloadSizeOperandIndex(HGBCOpcode opcode)")
    emit("{")
    emit("    const HGBCOpcodeInfo* info = getHGBCOpcodeInfo(opcode);")
    emit("    return info ? info->payloadSizeOperandIndex : -1;")
    emit("}")
    emit("")

    emit("inline const char* getHGBCEnumValueName(HGBCEnumKind kind, uint32_t value)")
    emit("{")
    emit("    switch (kind)")
    emit("    {")
    for _, enum_desc in ipairs(M.enums) do
        emit("    case HGBCEnumKind::" .. enum_desc.name .. ":")
        emit("        switch (" .. enum_desc.cpp_name .. "(value))")
        emit("        {")
        for _, value in ipairs(enum_desc.values) do
            emit(
                "        case "
                    .. enum_desc.cpp_name
                    .. "::"
                    .. value.name
                    .. ': return "'
                    .. value.name
                    .. '";'
            )
        end
        emit("        }")
        emit("        return nullptr;")
    end
    emit("    case HGBCEnumKind::None:")
    emit("        return nullptr;")
    emit("    }")
    emit("    return nullptr;")
    emit("}")
    emit("")

    emit(
        "inline bool findHGBCEnumValueByName("
            .. "HGBCEnumKind kind, const char* name, size_t length, uint32_t& outValue)"
    )
    emit("{")
    emit("    switch (kind)")
    emit("    {")
    for _, enum_desc in ipairs(M.enums) do
        emit("    case HGBCEnumKind::" .. enum_desc.name .. ":")
        for _, value in ipairs(enum_desc.values) do
            emit(
                "        if (length == "
                    .. #value.name
                    .. " && memcmp(name, "
                    .. cpp_string(value.name)
                    .. ", length) == 0)"
            )
            emit("        {")
            emit("            outValue = " .. tostring(value.value) .. ";")
            emit("            return true;")
            emit("        }")
        end
        emit("        return false;")
    end
    emit("    case HGBCEnumKind::None:")
    emit("        return false;")
    emit("    }")
    emit("    return false;")
    emit("}")
end

return M
