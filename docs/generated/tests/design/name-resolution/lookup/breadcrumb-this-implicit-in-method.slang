//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#breadcrumbs
//META: doc_section_digest=33d07243bfb42afebbc13d5fbbb87264e2fb46bd44f69743fdec1ff0e2f6c8b3
//META: purpose=An unqualified name inside a method that names a field of the enclosing type records a ThisExpr navigation step, so the expression behaves as if it were written this.field.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the simplest navigation chain the doc describes: a lone
// ThisExpr step. The method reads `g` with no receiver written, so the
// value can only come from the caller's object if lookup recorded the
// implicit receiver. CHECK asserts the caller's 33 is what the method
// returns, which distinguishes a real receiver read from a
// default-initialized copy.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    int g;
    int read() { return g; }
}

void main()
{
    S s;
    s.g = 33;
    // CHECK: r=33
    printf("r=%d\n", s.read());
}
