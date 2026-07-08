//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:09:48Z
//META: source_commit=f39188078fe010048714735213c24a7cb15532b9
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugscope
//META: doc_section_digest=39f7ccab66bac47197cd0a95044a8180619767ca0b6e93966a0e57e3dc200e0e
//META: purpose=A function-level DebugScope's first operand references the DebugFunction that owns the scope.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies scope chaining terminates at the owning DebugFunction. The
// DebugScope insts are materialized during SPIR-V emission and never
// appear in -dump-ir output, so the claim is observed on the emitted
// NonSemantic Shader DebugInfo: with a single function compiled under
// -g, the id of the DebugFunction ext-inst is captured and a DebugScope
// ext-inst must carry that id as its first (enclosing-scope) operand.
// Only a function-level scope can reference the DebugFunction directly,
// so the match proves the chain reaches its documented root.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int local = int(tid.x);
    buf[0] = local;
}

// CHECK: %[[DF:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[DF]]
