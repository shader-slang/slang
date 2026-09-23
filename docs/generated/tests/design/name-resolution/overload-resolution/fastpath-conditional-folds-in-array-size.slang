//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=16e17f0dd7f2f633f3f55fe47544e0f4c04b1b5b2ce0bf083439c29eeb4a3acc
//META: purpose=The Conditional operation kind lets a resolved ?: expression fold to a compile-time constant usable as an array size.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `?:` on compile-time-constant operands folds to a constant
// that an array bound accepts. The array is declared with `c ? 4 : 7`; CHECK
// asserts the observed element count is 4, which is only reachable if the
// conditional folded during checking rather than being deferred to runtime.

//TEST:INTERPRET(filecheck=CHECK):

static const bool c = true;

void main()
{
    int arr[c ? 4 : 7];
    int n = int(sizeof(arr) / sizeof(int));
    // CHECK: n=4
    printf("n=%d\n", n);
}
