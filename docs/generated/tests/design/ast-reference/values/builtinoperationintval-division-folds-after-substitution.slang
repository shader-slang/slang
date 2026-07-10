//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:25Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=4229b55a01470c7ad99014966f3313e48dd3d9bca875bef59b0f7b4c86cbc182
//META: purpose=A BuiltinOperationIntVal for N / 2 stays symbolic in the generic body and folds to its concrete quotient once N is substituted; the array parameter carries the folded element count.
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
