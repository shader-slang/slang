//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry chsMain -stage closesthit
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -entry chsMain -stage closesthit -Xnvrtc -I"./external/optix-dev/include/"
//TEST:SIMPLE(filecheck=CBUF): -target cuda -entry rgMain -stage raygeneration -DUSE_CBUFFER
//TEST:REFLECTION(filecheck=REFL): -target cuda -entry chsMain -stage closesthit -no-codegen

// Regression test for shader-slang/slang#12628: on CUDA/OptiX a shader-record global must be read
// from the shader binding table (SBT) via `optixGetSbtDataPointer()`, the way SPIR-V reads it from
// `ShaderRecordBufferKHR`. It must not become a pointer field of the `GlobalParams` launch struct.
// Ordinary globals keep their place in `GlobalParams`.

// SBT reads are not `__ldg`'d (the host may rewrite the SBT between launches, #10188), so
// `CHECK-NOT: __ldg` is placed on both sides of the anchor.

//CHECK-NOT: __ldg
//CHECK: struct GlobalParams_0
//CHECK-NOT: gSbtRecordData
//CHECK: gScale
//CHECK-NOT: gSbtRecordData
//CHECK: };
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer
//CHECK-NOT: __ldg

//CHECK-PTX: _optix_get_sbt_data_ptr_64

// The global is reflected as the shader record, and its contents keep CUDA's ordinary struct
// layout (`float4` at 0, `uint` at 16, size padded to the 16-byte alignment of `float4`).

//REFL: "name": "gSbtRecordData"
//REFL-NEXT: "binding": {"kind": "shaderRecord", "index": 0}
//REFL: "name": "color"
//REFL: "binding": {"kind": "uniform", "offset": 0, "size": 16
//REFL: "name": "id"
//REFL: "binding": {"kind": "uniform", "offset": 16, "size": 4
//REFL: {"kind": "uniform", "value": 32, "alignment": 16}

// The `cbuffer` spelling of a shader record goes through the same path.

//CBUF: struct GlobalParams_0
//CBUF-NOT: ShaderRecord
//CBUF: };
//CBUF: optixGetSbtDataPointer

struct SbtData
{
    float4 color;
    uint id;
};

#ifdef USE_CBUFFER

[[vk::shader_record]]
cbuffer ShaderRecord
{
    uint shaderRecordID;
    float scale;
}

uniform float gScale;
RWStructuredBuffer<float> gOutScalar;

[shader("raygeneration")]
void rgMain()
{
    gOutScalar[DispatchRaysIndex().x] = shaderRecordID * scale * gScale;
}

#else

layout(shaderRecordEXT) ConstantBuffer<SbtData> gSbtRecordData;

uniform float gScale;
RWStructuredBuffer<float4> gOut;

// Uses from a helper function and from a `static` global's initializer, not only from the entry
// point itself.
float4 readColor()
{
    return gSbtRecordData.color;
}

static uint gCachedId = gSbtRecordData.id;

[shader("closesthit")]
void chsMain(inout float4 payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload = readColor() * gScale + float(gSbtRecordData.id + gCachedId);
    gOut[0] = payload;
}

#endif
