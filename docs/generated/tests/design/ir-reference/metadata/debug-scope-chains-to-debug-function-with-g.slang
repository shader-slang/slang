//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:09:48Z
//META: source_commit=f39188078fe010048714735213c24a7cb15532b9
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugscope
//META: doc_section_digest=39f7ccab66bac47197cd0a95044a8180619767ca0b6e93966a0e57e3dc200e0e
//META: purpose=A function-level DebugScope's first operand references the DebugFunction that owns the scope.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies scope chaining terminates at the owning DebugFunction: with a
// single function compiled under -g, the id of the module's DebugFunction
// is captured and some DebugScope in the dump must carry that id as its
// first (enclosing-scope) operand. Only a function-level scope can
// reference the DebugFunction directly, so the match proves the chain
// reaches its documented root.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int local = int(tid.x);
    buf[0] = local;
}

// CHECK: %[[DF:[0-9]+]]{{[^0-9].*}}= DebugFunction(
// CHECK: DebugScope(%[[DF]]{{[^0-9]}}
