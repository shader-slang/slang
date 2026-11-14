//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#compiletimeforstmt
//META: doc_section_digest=b830cb9d7a4474794ded574f3685c7c52f984f5f5edfcb84dd550dbf0d9ef1ae
//META: purpose=A CompileTimeForStmt iterates a compile-time-constant range and is unrolled at compile time; the body executes once per iteration with the loop variable bound to a compile-time constant.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `$for(i in Range(N))` runs its body once per value of a
// compile-time-constant range, with `i` bound to that value inside the
// body. The CHECK asserts the summed iterator values are 0+1+2+3 = 6,
// which distinguishes a correct unroll from a dropped or duplicated
// one.

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
