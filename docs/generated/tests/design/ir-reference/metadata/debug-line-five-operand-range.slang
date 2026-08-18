//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugline
//META: doc_section_digest=eb5cc6b339fed6275ffa177c8c0c47a41583954b25feb65903b85fa94849df4a
//META: purpose=DebugLine pins an instruction to a source range with five operands: the source file, the start and end lines, and the start and end columns.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the range record's arity and its file operand. The CHECK captures
// the module's `DebugSource` id and then requires a `DebugLine` whose first
// operand is that id followed by exactly four unsigned literals, so a record
// with a different operand count would fail.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: %[[SRC:[0-9]+]]{{.*}}= DebugSource(
// CHECK: DebugLine(%[[SRC]], {{[0-9]+}} : UInt, {{[0-9]+}} : UInt, {{[0-9]+}} : UInt, {{[0-9]+}} : UInt)
