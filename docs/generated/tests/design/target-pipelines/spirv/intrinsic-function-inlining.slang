//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=performIntrinsicFunctionInlining (SPIR-V-only, gated on emitSpirvDirectly) and performForceInlining fold a small helper into the caller - no separate OpFunction survives.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V emit runs performForceInlining then
// performIntrinsicFunctionInlining (the second inlining pass
// gated on emitSpirvDirectly). Together they fold small user
// helpers into the caller. Observation: only %main appears as
// an OpFunction in the emit; the helper does not survive.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -O1

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
// CHECK-NOT: OpFunctionCall
// CHECK: OpFunctionEnd
