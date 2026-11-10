//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#nodes
//META: doc_section_digest=29d86296deb73a57a4f207507e16ba645a6bf2e722162db8157e7554d2cf8004
//META: purpose=A ray payload passed `inout` to a closesthit entry point lowers to a GLSL `rayPayloadInEXT` variable.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ray-payload attribute family's user-visible consequence on the
// GLSL back end: the payload parameter becomes a module-scope `rayPayloadInEXT`
// variable rather than an ordinary function argument, which is how Vulkan ray
// tracing passes payloads. CHECK asserts that storage qualifier appears in the
// emitted GLSL.

// The direct SPIR-V path performs the same rewrite under the Khronos name for
// that storage class, so the SPIRV prefix asserts the payload became a module
// -scope `IncomingRayPayloadKHR` variable there rather than staying a function
// parameter.

//TEST:SIMPLE(filecheck=CHECK): -target glsl -profile glsl_460+GL_EXT_ray_tracing -stage closesthit -entry chitMain
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage closesthit -entry chitMain

struct ColorPayload
{
    float4 color;
};

[shader("closesthit")]
void chitMain(inout ColorPayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = float4(0.25f, 0.5f, 1.0f, 1.0f);
}

//CHECK: rayPayloadInEXT

//SPIRV: %payload = OpVariable %{{[A-Za-z0-9_]+}} IncomingRayPayloadKHR
