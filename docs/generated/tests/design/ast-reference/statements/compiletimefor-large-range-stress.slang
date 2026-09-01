//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#compiletimeforstmt
//META: doc_section_digest=e6b1411878ad1410d1a25b47fb11f2d5bfdbd0f414a7e77f7f2b914774e3bc68
//META: purpose=A CompileTimeForStmt over a 32-element range unrolls to 32 copies of the body; the summed loop-variable values match the closed-form 0+1+...+31 = 496.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: a 32-iteration compile-time unroll. Verifies the body is
// instantiated once per value of the range with the loop variable bound
// to that value. The CHECK asserts the accumulated sum equals the
// closed form 0+1+...+31 = 496, which only matches if every iteration
// was emitted exactly once.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    $for(i in Range(32))
    {
        sum = sum + i;
    }
    // CHECK: sum=496
    printf("sum=%d\n", sum);
}
