//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
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
