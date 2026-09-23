//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=81b1b9ef9946750fa4993fc9e38d2ffd230f962a9583bad2803017f58f335d29
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
