//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=acb151a083ad11459c8dc1bd5b90cc232de73c42a6dc1e1771f0fabb823d2ec8
//META: purpose=An `inout` parameter both reads the caller's value and writes back to it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `inout` is both directions at once: the callee sees the
// caller's value and its assignment propagates back. `doubleIt` computes
// `x * 2`, so the printed 42 proves both halves -- the callee read 21 (in) and
// the caller saw the product (out). Either half missing gives a different number.

//TEST:INTERPRET(filecheck=CHECK):

void doubleIt(inout int x)
{
    x = x * 2;
}

void main()
{
    int v = 21;
    doubleIt(v);
    //CHECK: v=42
    printf("v=%d\n", v);
}
