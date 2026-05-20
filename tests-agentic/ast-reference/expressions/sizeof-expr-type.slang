//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=A SizeOfExpr `sizeof(T)` yields a compile-time-known size in bytes; for `int` the layout is 4.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SizeOfExpr is documented to compute `sizeof(T)` / `sizeof(e)`. We
// evaluate it at file scope into a `static const` so the value is
// definitely the compile-time constant; printing it confirms.

//TEST:INTERPRET(filecheck=CHECK):

static const int sz = sizeof(int);

void main()
{
    // CHECK: sz=4
    printf("sz=%d\n", sz);
}
