//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugvar
//META: doc_section_digest=c618286f222917160f4fd6b587c35c8e881c2d35f85713788e1e7cc4f09c2f07
//META: purpose=A DebugVar for an ordinary local omits the argument-index operand, and the variable's own type is the pointee of the instruction's pointer result type rather than an operand.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the absent case of the optional operand together with the result
// type rule. The local is an `int`, so its record must print with a `Ptr(Int)`
// result type and exactly three operands; pinning the closing parenthesis
// after the column rules out an argument index, and pinning the result type
// shows the variable's type is carried by the result rather than by an
// operand.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int local = int(tid.x) + 7;
    buf[0] = local;
}

// CHECK: Ptr(Int){{.*}}= DebugVar(%{{[0-9]+}}, {{[0-9]+}} : UInt, {{[0-9]+}} : UInt)
