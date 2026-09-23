//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes how a ray-tracing entry point with more than one inout varying parameter has those parameters merged into a single IncomingRayPayloadKHR struct variable at Location 0.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `consolidateParameters` (slang-ir-glsl-legalize.cpp:3040), reached
// from `consolidateRayTracingParameters` only when a ray-tracing entry point
// has **two or more** `inout` varying parameters. Vulkan allows a hit shader
// exactly one incoming payload variable, so Slang cannot map each `inout`
// parameter to its own payload; instead the legalizer builds an anonymous
// struct with one field per parameter, creates a single
// `IncomingRayPayloadKHR` global for it at location 0, and rewrites each
// parameter use into a field address of that global. CHECK asserts the merged
// shape: exactly one `IncomingRayPayloadKHR` variable in the entry-point
// interface, decorated `Location 0`, whose pointee is a struct (not `Payload`
// or `Extra` directly) — which is what distinguishes this arm from the
// single-`inout` arm that would name the user's own payload type.
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry chit -stage closesthit

struct Payload { float4 color; }
struct Extra { uint hits; }

[shader("closesthit")]
void chit(inout Payload p, inout Extra e, in BuiltInTriangleIntersectionAttributes attr)
{
    p.color = float4(attr.barycentrics, 0, 1);
    e.hits = e.hits + 1;
}

//SPIRV: OpEntryPoint ClosestHitKHR %chit
// The hit attributes stay a separate variable, named after the parameter.
//SPIRV: OpName %attr "attr"
// The two inout parameters are merged into one payload variable at location 0.
//SPIRV: OpDecorate %{{[0-9]+}} Location 0
//SPIRV: %_ptr_IncomingRayPayloadKHR__struct_{{[0-9]+}} = OpTypePointer IncomingRayPayloadKHR %_struct_{{[0-9]+}}
//SPIRV: %{{[0-9]+}} = OpVariable %_ptr_IncomingRayPayloadKHR__struct_{{[0-9]+}} IncomingRayPayloadKHR
// ...and the attributes remain in the HitAttributeKHR storage class.
//SPIRV: %_ptr_HitAttributeKHR_v2float
