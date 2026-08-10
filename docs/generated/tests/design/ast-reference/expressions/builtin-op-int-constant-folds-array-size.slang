//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=81b1b9ef9946750fa4993fc9e38d2ffd230f962a9583bad2803017f58f335d29
//META: purpose=A binary builtin operator on integer scalar operands resolves to a single operation kind and constant-folds: `(13 & 6) | 1` is accepted as an array dimension and evaluates to 5.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that builtin bitwise operators on integer scalars are resolved
// to their operation kind once and reach the constant folder: the value of
// `(13 & 6) | 1` is demanded in an array-dimension position, which requires
// a compile-time-known integer. The program only compiles if folding
// succeeded, and CHECK asserts the folded value 5 at runtime.

//TEST:INTERPRET(filecheck=CHECK):

static const int kFolded = (13 & 6) | 1;

void main()
{
    // Array dimension forces compile-time evaluation of kFolded.
    int arr[kFolded];
    arr[0] = kFolded;
    // CHECK: folded=5
    printf("folded=%d\n", arr[0]);
}
