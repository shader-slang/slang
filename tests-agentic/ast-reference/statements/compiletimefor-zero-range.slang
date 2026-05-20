//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#compiletimeforstmt
//META: doc_section_digest=b47d8d92e9d8444a86d4940595b47a98936eeee918ce8511059d4125824088d0
//META: purpose=A CompileTimeForStmt with a zero-length range (`$for(i in Range(0))`) unrolls to zero copies of the body; the surrounding accumulator is unchanged by the unrolled-no-times loop.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: lower-edge range size for CompileTimeForStmt. The doc
// says the body is "duplicated once per iteration"; with zero
// iterations no copies should be emitted. The observable is that
// no side effects from the body are applied.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 7;
    $for(i in Range(0))
    {
        sum = sum + 100;
    }
    // CHECK: sum=7
    printf("sum=%d\n", sum);
}
