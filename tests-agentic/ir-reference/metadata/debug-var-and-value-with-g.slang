//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=ac7b3f14612cf30d111a742d03f1dabae49c13bdd0024c69f90d82e36c1ecb8d
//META: purpose=DebugVar declares a user-visible local for the debugger; DebugValue reports the current value of a DebugVar.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Debug-info-family table: `DebugVar` declares a
// user-visible variable for the debugger (operands: name, type,
// scope, location). `DebugValue` reports the current value
// (variadic, min=2). A non-trivial local under `-g` emits both.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int local = int(tid.x);
    buf[0] = local;
}

// CHECK-DAG: = DebugVar(
// CHECK-DAG: DebugValue(
