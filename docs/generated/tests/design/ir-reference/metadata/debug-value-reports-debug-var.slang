//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=50809951cc8677fde31d92b4c0fe9b213a4fc77720df38216d197ee7561bc6d1
//META: purpose=DebugValue reports the current value of a DebugVar, taking the variable declaration in operand 0 and the value in operand 1.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pairing between a variable declaration and a value report. The
// CHECK captures the id of the `DebugVar` created for the local and then
// requires a following `DebugValue` whose first operand is that same id, which
// is what makes the report attributable to a specific variable rather than to
// a position in the block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int local = int(tid.x) + 7;
    buf[0] = local;
}

// CHECK: %[[V:[a-zA-Z0-9_]+]]{{.*}}= DebugVar(%{{[0-9]+}}, {{[0-9]+}} : UInt, {{[0-9]+}} : UInt)
// CHECK: DebugValue(%[[V]], %{{[a-zA-Z0-9_]+}})
