//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=1c3af19385b1b45e53fc2911de4a5495ab14067ddf0d2925bb4569db9c7887aa
//META: purpose=A mixed-type shift is declined by the fast path and resolved by the general path, which keeps the type of the left operand rather than a common type.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fast path's mixed-type-shift carve-out: `a << b` does not go
// through the common-type rule, so a 64-bit left operand shifted by a 32-bit
// amount keeps its 64-bit type. An overload probe on `int` versus `int64_t`
// observes the type of the shift; CHECK asserts the int64_t overload fired.

//TEST:INTERPRET(filecheck=CHECK):

int probe(int64_t x) { return 1; }
int probe(int x)     { return 2; }

void main()
{
    int64_t a = 3;
    int amount = 1;
    int tag = probe(a << amount);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
