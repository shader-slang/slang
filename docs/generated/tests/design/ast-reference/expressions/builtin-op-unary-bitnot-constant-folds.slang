//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=c1890a75acdc226411b7d61f4ee2807ae95bd2bb75bf036bb2da6f27662ff946
//META: purpose=A unary builtin operator carries a single operand: `~(-4)` constant-folds to 3 and is accepted as an array dimension.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one-operand edge of the builtin-operator argument shape:
// bitwise-not is a unary builtin operator, so `~(-4)` must resolve and
// constant-fold (two's complement: ~(-4) == 3). The folded value is
// demanded as an array dimension, so compilation itself proves the fold;
// CHECK asserts the runtime read-back of the same constant.

//TEST:INTERPRET(filecheck=CHECK):

static const int kDim = ~(-4);

void main()
{
    int arr[kDim];
    arr[kDim - 1] = kDim;
    // CHECK: dim=3
    printf("dim=%d\n", arr[kDim - 1]);
}
