//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#compiletimeforstmt
//META: doc_section_digest=b47d8d92e9d8444a86d4940595b47a98936eeee918ce8511059d4125824088d0
//META: purpose=A CompileTimeForStmt over a 32-element range unrolls to 32 copies of the body; the summed loop-variable values match the closed-form 0+1+...+31 = 496.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: 32-iteration unroll of CompileTimeForStmt. The doc says
// the body is duplicated once per iteration and the loop variable
// is bound to a compile-time constant per copy. Sum of 0..31 is
// 496.

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
