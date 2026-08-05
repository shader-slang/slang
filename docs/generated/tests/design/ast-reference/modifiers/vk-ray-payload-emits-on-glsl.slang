//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#ray-tracing-attributes
//META: doc_section_digest=199dda0284ee15697819954cfd47a494118e3dbf39cb7407bedf93c8394a88d8
//META: purpose=A ray payload passed `inout` to a closesthit entry point lowers to a GLSL `rayPayloadInEXT` variable.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ray-payload attribute family's user-visible consequence on the
// GLSL back end: the payload parameter becomes a module-scope `rayPayloadInEXT`
// variable rather than an ordinary function argument, which is how Vulkan ray
// tracing passes payloads. CHECK asserts that storage qualifier appears in the
// emitted GLSL.

//TEST:SIMPLE(filecheck=CHECK): -target glsl -profile glsl_460+GL_EXT_ray_tracing -stage closesthit -entry chitMain

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
