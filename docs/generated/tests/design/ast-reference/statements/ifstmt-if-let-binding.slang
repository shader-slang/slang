//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#ifstmt
//META: doc_section_digest=a6c1740eeba3eb150d93ff559a355c4b73decc79a2b471fc7f1203bf5d838824
//META: purpose=There is no dedicated statement form for `if let`; it desugars at parse time into an ordinary IfStmt whose positive branch is prefixed with a binding of the unwrapped value, so the bound name is readable only on the positive path.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `if (let v = opt)` behaves exactly like an `if` whose
// predicate tests the optional and whose positive branch opens with a
// binding of the unwrapped value. Two optionals are tested: a
// value-carrying one must enter the positive branch with `v` bound to
// the payload, and a `none` one must fall to the `else`. The CHECKs
// assert the payload is printed for the first and only the else marker
// for the second.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> some = 5;
    if (let v = some) { printf("some=%d\n", v); }

    Optional<int> nothing = none;
    if (let w = nothing) { printf("bound=%d\n", w); }
    else { printf("no-value\n"); }

    // CHECK: some=5
    // CHECK-NEXT: no-value
}
