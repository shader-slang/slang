//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#compiletimeforstmt
//META: doc_section_digest=b47d8d92e9d8444a86d4940595b47a98936eeee918ce8511059d4125824088d0
//META: purpose=A CompileTimeForStmt iterates a compile-time-constant range and is unrolled at compile time; the body executes once per iteration with the loop variable bound to a compile-time constant.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc spells CompileTimeForStmt as "Range-based loop unrolled at
// compile time" and explains that bounds must be compile-time
// constants. The surface form is `$for(i in Range(N))`. We observe by
// summing the iterator values: 0+1+2+3 = 6 for Range(4).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    $for(i in Range(4))
    {
        sum = sum + i;
    }
    // CHECK: sum=6
    printf("sum=%d\n", sum);
}
