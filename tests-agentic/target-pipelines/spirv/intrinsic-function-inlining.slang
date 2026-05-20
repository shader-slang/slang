//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=performIntrinsicFunctionInlining (SPIR-V-only, gated on emitSpirvDirectly) and performForceInlining fold a small helper into the caller - no separate OpFunction survives.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V emit runs performForceInlining then
// performIntrinsicFunctionInlining (the second inlining pass
// gated on emitSpirvDirectly). Together they fold small user
// helpers into the caller. Observation: only %main appears as
// an OpFunction in the emit; the helper does not survive.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

struct S { int a; int b; }
int sumPair(S s) { return s.a + s.b; }

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    S s = { int(tid.x), int(tid.y) };
    outBuf[tid.x] = sumPair(s);
}

// CHECK: %main = OpFunction %void None
// CHECK-NOT: OpFunction
// CHECK: OpFunctionEnd
