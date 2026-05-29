//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#getoptixraypayloadptr
//META: doc_section_digest=c3a35e4b5037da8ad936ccd365aac116a4da968dbaab5a0d940274845cb0ca71
//META: purpose=`CallShader(idx, payload)` from a raygeneration entry point on `-target spirv-asm` surfaces the documented `GetVulkanRayTracingPayloadLocation` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's raytracing-payload table lists `GetVulkanRayTracingPayloadLocation`
// as the synthesized opcode that returns the binding location of a
// Vulkan ray-tracing payload. It is invoked by the GLSL/SPIR-V
// lowering of `CallShader(...)` (via the internal
// `__callablePayloadLocation` helper).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage raygeneration -entry main

struct CallablePayload
{
    float3 color;
};

[shader("raygeneration")]
void main()
{
    CallablePayload payload;
    payload.color = float3(0.0, 0.0, 0.0);
    CallShader(0, payload);
}

// IR-DAG: GetVulkanRayTracingPayloadLocation
