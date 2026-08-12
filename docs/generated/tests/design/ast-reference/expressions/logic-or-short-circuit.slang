//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=A LogicOperatorShortCircuitExpr `||` does not evaluate the right operand when the left is true.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Dual of the `&&` short-circuit: when the left operand of `||` is
// true, the right operand must not be evaluated. We use a
// counter-incrementing right operand to make non-evaluation observable.

//TEST:INTERPRET(filecheck=CHECK):

bool bump(inout int counter) { counter++; return false; }

void main()
{
    int counter = 0;
    bool r1 = (true) || bump(counter);
    // CHECK: counter-after-true-or=0
    printf("counter-after-true-or=%d\n", counter);

    bool r2 = (false) || bump(counter);
    // CHECK: counter-after-false-or=1
    printf("counter-after-false-or=%d\n", counter);
}
