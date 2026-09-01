// Regression test for:
// https://github.com/shader-slang/slang/issues/11230
//
// A hit-shader-only translation unit (no `TraceRay`-style call) with a
// user-authored `[raypayload]` struct that carries one-sided (partial) payload
// access qualifiers. The call-site PAQ fill only covers payload structs reached
// through a `TraceRay` / `HitObject::TraceRay` / `HitObject::Invoke` argument, so
// before the structural fill this struct kept its one-sided PAQ at SM 6.7+ and
// DXC rejected it. The structural pass must fill the missing per-side qualifier
// on every `[raypayload]` struct, while leaving the explicit side untouched.
//
// The fix is keyed on `IRRayPayloadDecoration`, not on entry-point stage, so the
// fill must apply for any hit-shader stage (anyhit / closesthit / miss) and for a
// `lib_6_7` library compile that holds several such entry points over a shared
// payload with no `TraceRay` — the realistic per-stage-compiled, runtime-linked
// shader-library shape from the issue.

// Each hit-shader stage compiled on its own (no TraceRay) must still fill PartialPayload.
//TEST:SIMPLE(filecheck=HLSL66): -target hlsl -profile sm_6_6 -stage anyhit -entry ahit_main
//TEST:SIMPLE(filecheck=HLSL67): -target hlsl -profile sm_6_7 -stage anyhit -entry ahit_main
//TEST:SIMPLE(filecheck=HLSL67): -target hlsl -profile sm_6_7 -stage closesthit -entry chit_main
//TEST:SIMPLE(filecheck=HLSL67): -target hlsl -profile sm_6_7 -stage miss -entry miss_main
//TEST:SIMPLE(filecheck=DXIL67): -target dxil -profile sm_6_7 -stage anyhit -entry ahit_main

// A field that already declares *both* sides must be left exactly as written; this struct
// only reaches a hit shader, so it goes through the new structural pass, not the TraceRay
// call-site path.
//TEST:SIMPLE(filecheck=EXPLICIT67): -target hlsl -profile sm_6_7 -stage closesthit -entry chit_explicit

// A lib_6_7 compile holds every entry point over the shared payloads with no TraceRay --
// the per-stage-compiled, runtime-linked shader-library shape from the issue.
//TEST:SIMPLE(filecheck=HLSL67): -target hlsl -profile lib_6_7
//TEST:SIMPLE(filecheck=EXPLICIT67): -target hlsl -profile lib_6_7
//TEST:SIMPLE(filecheck=DXIL67): -target dxil -profile lib_6_7

// At SM 6.6 PAQs are not emitted at all.
// HLSL66-NOT: [raypayload]
// HLSL66-NOT: : read(
// HLSL66-NOT: : write(

// At SM 6.7+ every field must carry both sides; the one-sided fields get their missing
// side filled with the full-stage default.
// HLSL67-DAG: struct [raypayload] PartialPayload
// HLSL67-DAG: float3 readOnly_0 : read(caller) : write(caller, anyhit, closesthit, miss);
// HLSL67-DAG: float3 writeOnly_0 : read(caller, anyhit, closesthit, miss) : write(caller);

// A field that already declares both sides is left exactly as written.
// EXPLICIT67: struct [raypayload] ExplicitPayload
// EXPLICIT67: float3 normal_0 : read(caller) : write(miss);

// And DXC accepts it (payload annotations are emitted in the DXIL).
// DXIL67: define void @
// DXIL67: !dx.dxrPayloadAnnotations

RWTexture2D<float4> outputImage;
RaytracingAccelerationStructure topLevelAS;

[raypayload]
struct PartialPayload
{
    float3 readOnly  : read(caller);
    float3 writeOnly : write(caller);
};

[raypayload]
struct ExplicitPayload
{
    float3 normal : read(caller) : write(miss);
};

[shader("anyhit")]
void ahit_main(inout PartialPayload p, in BuiltInTriangleIntersectionAttributes attr)
{
    p.readOnly  = float3(1, 0, 0);
    p.writeOnly = float3(0, 1, 0);
}

[shader("closesthit")]
void chit_main(inout PartialPayload p, in BuiltInTriangleIntersectionAttributes attr)
{
    p.writeOnly = p.readOnly;
}

[shader("miss")]
void miss_main(inout PartialPayload p)
{
    p.writeOnly = float3(0, 0, 0);
}

[shader("closesthit")]
void chit_explicit(inout ExplicitPayload p, in BuiltInTriangleIntersectionAttributes attr)
{
    p.normal = float3(0, 0, 1);
}
