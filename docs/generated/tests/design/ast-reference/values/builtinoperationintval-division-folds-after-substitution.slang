//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=7294e983132681667c88eb7937aa63061a1f9c975bb9567ce8dd36184dcda775
//META: purpose=A symbolic builtin operator whose operand is a generic value parameter re-evaluates on substitution and folds to a constant, so N / 2 becomes the concrete element count of the parameter array.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a division over a generic value parameter is carried
// symbolically and collapses to a constant at instantiation. sumHalf's
// parameter is typed int[N / 2]; the call sumHalf<8>(aa) with aa of
// type int[4] type-checks only if 8 / 2 folded to the constant 4, and
// CHECK asserts the four summed elements flow through at runtime.

//TEST:INTERPRET(filecheck=CHECK):

int sumHalf<let N : int>(int a[N / 2])
{
    int total = 0;
    for (int i = 0; i < N / 2; ++i) { total += a[i]; }
    return total;
}

void main()
{
    // 8 / 2 == 4, so int[4] is exactly the substituted parameter type.
    int aa[4] = { 1, 2, 3, 4 };
    // CHECK: half=10
    printf("half=%d\n", sumHalf<8>(aa));
}
