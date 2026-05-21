//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#ray-tracing-attributes
//META: doc_section_digest=4a578d24f65dc29bcbb85085a150f642d821eab1012ac8a09ab76f12dff281fa
//META: purpose=`VulkanRayPayloadAttribute` (`[vk::ray_payload]`) on a closesthit payload propagates to emitted GLSL as a `rayPayloadInEXT`-class declaration.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `VulkanRayPayloadAttribute` is the AST node carrying the `location`
// arg of `[vk::ray_payload(N)]`. When attached to a payload struct on a
// closesthit entry point, lowering to GLSL must emit a `rayPayloadInEXT`
// variable for that struct.

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
