//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=16e17f0dd7f2f633f3f55fe47544e0f4c04b1b5b2ce0bf083439c29eeb4a3acc
//META: purpose=A user-declared operator+(int, int) never fires on builtin int operands because convertToBuiltinArithmeticOp rewrites the expression before any candidate is collected.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the builtin-operator fast path runs before candidate
// collection, so a user-declared `operator+` on builtin `int` operands is
// never even considered. The user overload returns a sentinel 999; CHECK
// asserts the sum is the ordinary builtin 3, which is only possible if
// overload resolution never ran for this expression.

//TEST:INTERPRET(filecheck=CHECK):

int operator+(int a, int b) { return 999; }

void main()
{
    int lhs = 1;
    int rhs = 2;
    int v = lhs + rhs;
    // CHECK: v=3
    printf("v=%d\n", v);
}
