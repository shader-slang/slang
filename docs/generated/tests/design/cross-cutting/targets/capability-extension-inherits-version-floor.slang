//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#definition-forms
//META: doc_section_digest=d6d360ed810fe20f1543f761a89b7a194383364a9db523edf2e823b33ddfb19e
//META: purpose=An extension atom records its version floor through inheritance, so asking for `SPV_KHR_cooperative_matrix` transitively asks for its `_spirv_1_6` floor and its inherited memory-model requirement.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that inheritance in a capability declaration is what
// carries an extension's version floor and its companion
// requirements, rather than each requirement being asserted at the
// use site. `SPV_KHR_cooperative_matrix` inherits a SPIR-V 1.6 floor
// and the Vulkan memory model; the kernel itself uses neither. The
// CHECKs assert the emitted module header moved up to 1.6 and that
// the memory-model capability was declared, both of which can only
// come from folding in the inherited atoms.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -capability SPV_KHR_cooperative_matrix -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 5;
}

// SPIRV-DAG: Version: 1.6
// SPIRV-DAG: OpCapability VulkanMemoryModel
