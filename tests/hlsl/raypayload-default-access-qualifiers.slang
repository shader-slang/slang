// Regression test for #10267: at SM 6.7+, the DXR spec requires every field of
// a `[raypayload]` struct to declare both a `read(...)` and a `write(...)`
// qualifier
// (https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#payload-access-qualifiers).
// Slang's frontend only diagnoses fields where *both* qualifiers are missing,
// so three cases used to reach codegen with incomplete or absent decorations
// and DXC then rejected the emitted HLSL:
//   1. Struct without an explicit `[raypayload]` attribute used as a TraceRay
//      payload (Slang implicitly tags the struct with `IRRayPayloadDecoration`).
//   2. Explicit `[raypayload]` struct where the user wrote only `[read(...)]`
//      or only `[write(...)]` on a field.
//   3. Empty `[raypayload]` struct, where Slang synthesizes a `_slang_dummy`
//      field that itself needs spec-conforming qualifiers.
// `legalizeRayPayloadAccessQualifiersForHLSL` fills in whichever side is missing
// with the full-stage default `read/write(caller, anyhit, closesthit, miss)`,
// preserving the implicit pre-SM-6.7 "any stage may access any field" semantics.

//TEST:SIMPLE(filecheck=CHECK_67): -target hlsl -profile lib_6_7 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=DXIL_67): -target dxil -profile lib_6_7 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=CHECK_66): -target hlsl -profile lib_6_6 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=NONSTRUCT_67): -target hlsl -profile lib_6_7 -stage raygeneration -entry rgen_nonstruct

// SM 6.7+: `[raypayload]` is emitted on every payload struct, and each field
// carries spec-conforming `read(...) : write(...)` qualifiers. The pass leaves
// user-declared qualifiers untouched and fills in whichever side is missing.
// CHECK_67-DAG: struct [raypayload] RayPayload
// CHECK_67-DAG: float3 color_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// CHECK_67-DAG: struct [raypayload] ExplicitRayPayload
// CHECK_67-DAG: float3 normal_0 : read(caller) : write(miss);
// CHECK_67-DAG: struct [raypayload] PartialRayPayload
// CHECK_67-DAG: float3 readOnly_0 : read(caller) : write(caller, anyhit, closesthit, miss);
// CHECK_67-DAG: float3 writeOnly_0 : read(caller, anyhit, closesthit, miss) : write(caller);
// CHECK_67-DAG: struct [raypayload] EmptyRayPayload
// CHECK_67-DAG: int _slang_dummy_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);

// And DXC accepts the resulting HLSL and emits payload annotations metadata.
// DXIL_67: define void @
// DXIL_67: !dx.dxrPayloadAnnotations

// SM 6.6: no `[raypayload]` attribute and no payload access qualifiers (the
// SM 6.7+ feature must not leak into older targets).
// CHECK_66-NOT: [raypayload]
// CHECK_66-DAG: struct RayPayload
// CHECK_66-DAG: float3 color_0;
// CHECK_66-DAG: struct ExplicitRayPayload
// CHECK_66-DAG: float3 normal_0;
// CHECK_66-DAG: struct PartialRayPayload
// CHECK_66-DAG: float3 readOnly_0;
// CHECK_66-DAG: float3 writeOnly_0;
// CHECK_66-DAG: struct EmptyRayPayload
// CHECK_66-DAG: int _slang_dummy_0;
// CHECK_66-NOT: : read(
// CHECK_66-NOT: : write(

// Non-struct payload at SM 6.7+: Slang wraps `float4 payload` in a synthetic
// struct via `legalizeNonStructParameterToStructForHLSL`. The wrapper picks up
// `IRRayPayloadDecoration` implicitly and the field needs the full-stage
// default applied.
// NONSTRUCT_67-DAG: struct [raypayload] RayPayload_t
// NONSTRUCT_67-DAG: float4 data_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);

RWTexture2D<float4> outputImage;
RaytracingAccelerationStructure topLevelAS;

struct RayPayload
{
    float3 color;
};

[raypayload]
struct ExplicitRayPayload
{
    float3 normal : read(caller) : write(miss);
};

[raypayload]
struct PartialRayPayload
{
    float3 readOnly : read(caller);
    float3 writeOnly : write(caller);
};

[raypayload]
struct EmptyRayPayload
{
};

[shader("raygeneration")]
void rgen_main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    RayPayload payload;
    payload.color = float3(0, 0, 0);
    ExplicitRayPayload explicitPayload;
    explicitPayload.normal = float3(0, 0, 1);
    PartialRayPayload partialPayload;
    partialPayload.readOnly = float3(0, 1, 0);
    partialPayload.writeOnly = float3(1, 0, 0);
    EmptyRayPayload emptyPayload;

    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, explicitPayload);
    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, partialPayload);
    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, emptyPayload);

    outputImage[DispatchRaysIndex().xy] = float4(payload.color, 1.0);
}

[shader("raygeneration")]
void rgen_nonstruct()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    float4 payload = float4(0, 0, 0, 0);

    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    outputImage[DispatchRaysIndex().xy] = payload;
}
