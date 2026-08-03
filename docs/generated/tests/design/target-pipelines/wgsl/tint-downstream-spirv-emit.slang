//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:19Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#downstream-tint
//META: doc_section_digest=860f913f0d9a957b746931c036f15ca420d7ad048e6e12e9ce2ce6ffd62bf18a
//META: purpose=The wgsl-spirv-asm target compiles the WGSL text through Tint to a SPIR-V module and then disassembles it via glslang into assembly whose OpEntryPoint names main
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two-step downstream chain behind the assembly target:
// WGSLSPIRVAssembly first reduces to the WGSLSPIRV intermediate (Tint
// consumes the emitted WGSL and produces a SPIR-V module), then
// glslang disassembles that module to SPIR-V assembly text. The
// CHECKed OpEntryPoint / OpExecutionMode tokens exist only in that
// disassembly, so matching them proves both downstream transitions
// ran.

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
