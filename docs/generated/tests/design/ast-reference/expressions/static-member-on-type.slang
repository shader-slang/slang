//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr
//META: doc_section_digest=58d3dc1b167dae22620293ef4a1c128bfcf79423ded9a56ab1bbc40bc734de7e
//META: purpose=Member access with `.` on a type-valued base (`Limits.kMax`) is reinterpreted during checking to reach the static member without an instance.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `.` member access whose base names a type (not a
// value) still reaches the type's static member: the parser starts it
// as a plain member access, and checking reinterprets it once the base
// resolves to a type. CHECK asserts the static constant's value read
// through `Limits.kMax` without an instance.

//TEST:INTERPRET(filecheck=CHECK):

struct Limits
{
    static const int kMax = 1234;
}

void main()
{
    int v = Limits.kMax;
    // CHECK: v=1234
    printf("v=%d\n", v);
}
