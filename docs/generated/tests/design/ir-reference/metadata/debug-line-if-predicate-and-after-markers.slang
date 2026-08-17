//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugline
//META: doc_section_digest=eb5cc6b339fed6275ffa177c8c0c47a41583954b25feb65903b85fa94849df4a
//META: purpose=One if statement produces DebugLine markers at both its predicate and the location following the statement, in the predicate block and the merge block respectively.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that lowering marks a selected control-flow subexpression as well as
// the statement itself, so that stepping over an `if` stops on the test and
// then on what follows it. The evidence is positional: the predicate marker
// carries the column of `tid.x > 2` and sits immediately before the `ifElse`
// terminator, while the after marker carries the line of the statement that
// follows the `if` and sits in the merge block, past the two arm blocks. The
// CHECKs anchor at `func %main` first so nothing in the core module can match.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1

uniform RWStructuredBuffer<float> gOut;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float acc = 1.0;
    if (tid.x > 2)
    {
        acc = 5.0;
    }
    gOut[0] = acc;
}

// CHECK: {{^}}func %main {{.*}}Func(Void,
// The statement itself is marked at the `if` keyword's column.
// CHECK: DebugLine(%{{[0-9]+}}, 28 : UInt, 28 : UInt, 5 : UInt, 6 : UInt)
// The predicate gets a second marker of its own, at the comparison operator's
// column, emitted after the comparison and immediately before the branch.
// CHECK: DebugLine(%{{[0-9]+}}, 28 : UInt, 28 : UInt, 15 : UInt, 16 : UInt)
// Operand 3 of the branch names the merge block, which is where the after
// location is marked -- following the id rather than taking whichever marker
// comes next is what distinguishes the merge block from the else arm.
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %[[MERGE:[0-9]+]])
// CHECK: {{^}}block %[[MERGE]](
// CHECK: DebugLine(%{{[0-9]+}}, 32 : UInt, 32 : UInt, 5 : UInt, 6 : UInt)
