//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=An AlignOfExpr `alignof(T)` yields a compile-time-known alignment in bytes; for `int` the value is 4.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// AlignOfExpr derives from SizeOfLikeExpr and represents `alignof(T)`.
// We pin the value at file scope as `static const int` and print it.

//TEST:INTERPRET(filecheck=CHECK):

static const int al = alignof(int);

void main()
{
    // CHECK: al=4
    printf("al=%d\n", al);
}
