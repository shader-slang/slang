//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#downstream-tint
//META: doc_section_digest=d3546f7669ed5e1ba58590ad4e253593e18e5d04c72f8e2aa620eb12247e3d7a
//META: purpose=The wgsl-spirv target hands the emitted WGSL text to Tint which produces a SPIR-V module whose OpEntryPoint names main
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Downstream Tint" section states that for WGSLSPIRV /
// WGSLSPIRVAssembly the Slang downstream-compile path invokes Tint
// with sourceLanguage=WGSL, targetType=SPIRV. We exercise that path
// via -target wgsl-spirv-asm and pin observable SPIR-V tokens that
// only appear when Tint actually produces a module.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl-spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 42;
}

// SPIR-V module markers produced by Tint.
// CHECK: OpEntryPoint
// CHECK-SAME: main
// CHECK: OpExecutionMode
