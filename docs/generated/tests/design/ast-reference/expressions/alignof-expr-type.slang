//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
