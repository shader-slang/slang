//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=Using a transcendental builtin like sin emits OpExtInstImport "GLSL.std.450" and an OpExtInst dispatch — Slang routes math intrinsics through the GLSL std-450 extended instruction set.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: sin() is one of the GLSL.std.450 extended instructions.
// The SPIR-V emit declares OpExtInstImport "GLSL.std.450" and
// dispatches the call as an OpExtInst against that import set.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = sin(float(tid.x));
}

// CHECK-DAG: OpExtInstImport "GLSL.std.450"
// CHECK-DAG: OpExtInst %float %{{[0-9]+}} Sin
