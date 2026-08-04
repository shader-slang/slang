//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=Verifies that the FloatingPointLiteralExpr distinguishes +0.0f and -0.0f at the bit level — `1.0/+0` yields +inf while `1.0/-0` yields -inf.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// +0 and -0 compare equal but have distinct sign bits. Dividing 1.0
// by each yields oppositely-signed infinities, which makes the bit-
// level distinction observable without target-specific intrinsics.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float p = 0.0f;
    float n = -0.0f;
    // CHECK: eq=1
    printf("eq=%d\n", int(p == n));
    float r1 = 1.0f / p;
    float r2 = 1.0f / n;
    // CHECK: same-inf=0
    printf("same-inf=%d\n", int(r1 == r2));
}
