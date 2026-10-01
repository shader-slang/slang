//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry chsMain -stage closesthit
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -entry chsMain -stage closesthit -Xnvrtc -I"./external/optix-dev/include/"
//TEST:SIMPLE(filecheck=CBUF): -target cuda -entry rgMain -stage raygeneration -DUSE_CBUFFER
//TEST:REFLECTION(filecheck=REFL): -target cuda -entry chsMain -stage closesthit -no-codegen
//TEST:SIMPLE(filecheck=TWO): -target cuda -entry chsMain -stage closesthit -entry missMain -stage miss
//TEST:REFLECTION(filecheck=CBUFREFL): -target cuda -entry rgMain -stage raygeneration -DUSE_CBUFFER -no-codegen
//TEST:SIMPLE(filecheck=BOTH): -target cuda -entry rgBoth -stage raygeneration

// Regression test for shader-slang/slang#12628: on CUDA/OptiX a shader-record global must be read
// from the shader binding table (SBT) via `optixGetSbtDataPointer()`, the way SPIR-V reads it from
// `ShaderRecordBufferKHR`. It must not become a pointer field of the `GlobalParams` launch struct.
// Ordinary globals keep their place in `GlobalParams`.

// Each body that uses the record reads it through the SBT: `readColor` gives the `color_0` anchor.
// CUDA moves the `static` initializer into its own function, called at the start of the kernel and
// emitted before it, which gives the first `id_0` anchor. The kernel's own read gives the second.
// SBT reads are not `__ldg`'d (the host may rewrite the SBT between launches, #10188), so
// `CHECK-NOT: __ldg` is placed between every anchor.

//CHECK-NOT: __ldg
//CHECK: struct GlobalParams_0
//CHECK-NOT: gSbtRecordData
//CHECK: gScale
//CHECK-NOT: gSbtRecordData
//CHECK: };
//CHECK-NOT: __ldg
//CHECK: readColor_0()
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->color_0
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->id_0
//CHECK-NOT: __ldg
//CHECK: __closesthit__chsMain
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->id_0
//CHECK-NOT: __ldg

//CHECK-PTX: _optix_get_sbt_data_ptr_64

// A helper shared by two ray tracing programs fetches the SBT pointer itself, so each program reads
// its own record.

//TWO-NOT: __ldg
//TWO: readColor_0()
//TWO-NOT: __ldg
//TWO: optixGetSbtDataPointer())->color_0
//TWO-NOT: __ldg
//TWO: __closesthit__chsMain
//TWO-NOT: __ldg
//TWO: readColor_0()
//TWO-NOT: __ldg
//TWO: __miss__missMain
//TWO-NOT: __ldg
//TWO: readColor_0()
//TWO-NOT: __ldg

// The global is reflected as the shader record, and its contents keep CUDA's ordinary struct
// layout (`float4` at 0, `uint` at 16, size padded to the 16-byte alignment of `float4`).

//REFL: "name": "gSbtRecordData"
//REFL-NEXT: "binding": {"kind": "shaderRecord", "index": 0}
//REFL: "name": "color"
//REFL: "binding": {"kind": "uniform", "offset": 0, "size": 16
//REFL: "name": "id"
//REFL: "binding": {"kind": "uniform", "offset": 16, "size": 4
//REFL: {"kind": "uniform", "value": 32, "alignment": 16}

// `gScale` moves to offset 0 of `GlobalParams`; host code that fills `GlobalParams` from
// reflection depends on this.

//REFL: "name": "gScale"
//REFL-NEXT: "binding": {"kind": "uniform", "offset": 0, "size": 4

//CBUFREFL: "name": "ShaderRecord"
//CBUFREFL-NEXT: "binding": {"kind": "shaderRecord", "index": 0}
//CBUFREFL: "name": "shaderRecordID"
//CBUFREFL: "binding": {"kind": "uniform", "offset": 0, "size": 4
//CBUFREFL: "name": "scale"
//CBUFREFL: "binding": {"kind": "uniform", "offset": 4, "size": 4
//CBUFREFL: "name": "gScale"
//CBUFREFL-NEXT: "binding": {"kind": "uniform", "offset": 0, "size": 4

// The `cbuffer` spelling of a shader record goes through the same path.

//CBUF-NOT: __ldg
//CBUF: struct GlobalParams_0
//CBUF-NOT: ShaderRecord
//CBUF: gScale
//CBUF-NOT: ShaderRecord
//CBUF: };
//CBUF-NOT: __ldg
//CBUF: optixGetSbtDataPointer())->shaderRecordID_0
//CBUF-NOT: __ldg
//CBUF: optixGetSbtDataPointer())->scale_0
//CBUF-NOT: __ldg

// A ray generation program that reads both an entry-point `uniform` and the global reads two
// views of the same SBT data, neither through `__ldg`.

//BOTH-NOT: __ldg
//BOTH: __raygen__rgBoth
//BOTH-NOT: __ldg
//BOTH: ((ShaderRecordParams_0*)optixGetSbtDataPointer())->value_0
//BOTH-NOT: __ldg
//BOTH: ((SbtData_0*)optixGetSbtDataPointer())->id_0
//BOTH-NOT: __ldg

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

[shader("miss")]
void missMain(inout float4 payload)
{
    payload = readColor();
}

// Entry-point `uniform` parameters are read from the start of the same SBT data as the global.
[shader("raygeneration")]
void rgBoth(uniform uint value)
{
    gOut[0] = float4(float(value + gSbtRecordData.id));
}

#endif
