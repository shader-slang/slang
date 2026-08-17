//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#instrumentation
//META: doc_section_digest=138b9939ba5bbb4e52a45b224e1ac3fdd87ee9defa3e1c0614c72903418b616e
//META: purpose=Coverage instrumentation synthesizes a __slang_coverage buffer that is an RWStructuredBuffer<uint64_t> by default and rewrites marker ops into atomic adds.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the default shape of the synthesized coverage buffer and the
// rewrite that populates it. The shader declares no such buffer, so
// everything the CHECKs look for has to have been introduced by the pass.
// The SPIR-V directive is where the documented `__slang_coverage` spelling
// survives verbatim into the output; the HLSL directive pins the default
// 64-bit element type and the atomic add that replaced the marker op.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main -trace-coverage
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main -trace-coverage

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = int(tid.x);
}

// SPIRV-DAG: OpName %__slang_coverage "__slang_coverage"

// The default counter element type is 64-bit ...
// HLSL: RWStructuredBuffer<uint64_t {{.*}}slang_coverage
// ... and each marker became an atomic add of one.
// HLSL: InterlockedAdd({{.*}}slang_coverage{{.*}}, 1ULL,
