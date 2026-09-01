//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=96f5a93c2d3b7b0c3cfafeba9cc0e6087ddd11d34203934cd833e35b847de3f5
//META: purpose=A ForStmt is a ScopeStmt; the loop variable introduced by `initialStatement` is scoped to the body and does not leak into the surrounding scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the declaration in a `for` header is scoped to the loop, so
// a same-named variable in the surrounding scope is shadowed but not
// written. The CHECKs assert the outer `i` still reads 100 after the
// loop while the loop itself iterated 0..3.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = 100;
    int sum = 0;
    for (int i = 0; i < 4; i = i + 1)
    {
        sum = sum + i;
    }
    // CHECK: i=100
    printf("i=%d\n", i);
    // 0+1+2+3 = 6
    // CHECK: sum=6
    printf("sum=%d\n", sum);
}
