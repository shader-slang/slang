//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:37:14+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-this.md#mutability
//META: doc_section_digest=2822c7f6b30708a773dbe170117103457227a510de3a625a9ca4bed55f2b62fe
//META: purpose=Calling a `[mutating]` member function on an l-value variable succeeds and the mutation is observable on the instance.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that calling a `[mutating]` method on a named variable (l-value) succeeds
// and that the mutation is visible on the original instance after the call returns.
// `c.increment()` writes through the implicit `this` l-value; CHECK asserts c.value
// is 11, which is only reachable if the mutation propagated back to the caller.

//TEST:INTERPRET(filecheck=CHECK):

struct Counter
{
    int value;
    [mutating] void increment() { value = value + 1; }
}

void main()
{
    Counter c;
    c.value = 10;
    c.increment();
    // CHECK: 11
    printf("%d\n", c.value);
}
