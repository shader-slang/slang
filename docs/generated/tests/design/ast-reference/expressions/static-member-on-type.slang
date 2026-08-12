//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr--staticmemberexpr--derefmemberexpr
//META: doc_section_digest=2e4c0cf09ff9c6fb2f3bd8e1c643d300367d45168a8b35d087ed154695482428
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
