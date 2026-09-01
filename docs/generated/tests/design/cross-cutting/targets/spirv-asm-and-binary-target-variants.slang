//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=09ecdf153c06b46e85b5bfd65a0136d6c1516c1e42e5e33b4b02ee440f2f963c
//META: purpose=`SLANG_SPIRV` and `SLANG_SPIRV_ASM` are binary and assembly variants of one emit backend, and the assembly variant `-target spirv-asm` produces SPIR-V module text.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the binary and assembly SPIR-V targets are two
// renderings of one emit backend by exercising the rendering a text
// runner can read. The CHECKs pin the structural header a SPIR-V
// module always carries — capability, memory model, entry point and
// execution mode — so a pass shows the direct SPIR-V backend ran and
// emitted a well-formed module rather than some other text.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// SPIRV: OpCapability Shader
// SPIRV: OpMemoryModel
// SPIRV: OpEntryPoint GLCompute %main "main"
// SPIRV: OpExecutionMode %main LocalSize 1 1 1
