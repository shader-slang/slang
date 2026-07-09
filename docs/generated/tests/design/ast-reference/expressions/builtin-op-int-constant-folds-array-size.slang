//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=c1890a75acdc226411b7d61f4ee2807ae95bd2bb75bf036bb2da6f27662ff946
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
