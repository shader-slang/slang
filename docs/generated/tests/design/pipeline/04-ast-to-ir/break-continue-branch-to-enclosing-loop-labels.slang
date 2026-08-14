//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A `BreakStmt` and a `ContinueStmt` each lower to an `unconditionalBranch` to the enclosing statement's break / continue label block — the same two blocks the `loop(...)` terminator names as its second and third operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `BreakStmt`/`ContinueStmt` row by tying each branch back
// to the label block the enclosing loop declared, rather than merely asserting
// that some `unconditionalBranch` exists. The `loop` terminator carries its
// body, break, and continue blocks as operands, so the test captures the second
// and third with FileCheck variables and then requires the `break` arm to
// branch to the captured break label and the `continue` arm to branch to the
// captured continue label. The loop bound is a `uniform` so neither the loop
// nor its early exits can be folded away, and the accumulator is written to the
// output buffer so the whole body survives.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;
uniform int n;

[numthreads(1, 1, 1)]
void main()
{
    int acc = 0;
    for (int i = 0; i < n; ++i)
    {
        if (i == 3) continue;
        if (i == 7) break;
        acc += i;
    }
    outputBuffer[0] = acc;
}

// The loop terminator names its break label second and its continue label third.
// CHECK: loop(%{{[A-Za-z0-9_]+}}, %[[BRK:[A-Za-z0-9_]+]], %[[CNT:[A-Za-z0-9_]+]],
// `continue` branches to the continue label, carrying the loop-carried value.
// CHECK-DAG: unconditionalBranch(%[[CNT]],
// `break` branches to the break label.
// CHECK-DAG: unconditionalBranch(%[[BRK]])
