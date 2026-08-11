//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:37:14+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-this.md#mutability
//META: doc_section_digest=2822c7f6b30708a773dbe170117103457227a510de3a625a9ca4bed55f2b62fe
//META: purpose=Passing an l-value variable as an `out` parameter argument succeeds and the written value is observable.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a named local variable (l-value) can be passed to an `out` parameter
// and that the callee's write is visible to the caller. `fill(v)` sets v to 42;
// CHECK asserts the printf prints 42, which is only reachable if the `out` write
// propagated back through the l-value argument.

//TEST:INTERPRET(filecheck=CHECK):

void fill(out int x) { x = 42; }

void main()
{
    int v = 0;
    fill(v);
    // CHECK: 42
    printf("%d\n", v);
}
