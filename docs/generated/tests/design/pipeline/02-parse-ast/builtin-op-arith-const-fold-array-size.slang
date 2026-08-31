//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=90f68d89058e95e927dbc5ffd876802120cff1e5fe6639465168476f05517709
//META: purpose=Constant folding consumes the checker's builtin-operator rewrite: an arithmetic expression over scalar operands folds to a compile-time value usable as an array size.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a builtin arithmetic operator invocation over builtin
// scalar operands is resolved by the checker into a form the constant
// folder can consume. `2 + 3 * 4` is used as an array size — a position
// that requires a compile-time value — so the program only compiles and
// prints 14 slots if the folder understood the rewritten operator.

//TEST:INTERPRET(filecheck=CHECK):

static const int N = 2 + 3 * 4;

void main()
{
    int arr[2 + 3 * 4];
    for (int i = 0; i < N; i++)
        arr[i] = 1;
    int sum = 0;
    for (int i = 0; i < N; i++)
        sum += arr[i];
    // CHECK: slots=14
    printf("slots=%d\n", sum);
}
