//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=7d0d8d99e81aaca2b6f41962be8f0bffe859a83dddeba6909e61b74e024ba2ff
//META: purpose=A pointer can be passed as a function argument; mutations through the pointer parameter are visible at the call site.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's example declares `uint *start, uint *end` as function
// parameters. Verify that passing a pointer to a function parameter
// lets the callee mutate the caller's variable through dereference.
//TEST:INTERPRET(filecheck=CHECK):

void addDelta(int* p, int delta)
{
    *p = *p + delta;
}

void main()
{
    int x = 10;
    addDelta(&x, 5);
    //CHECK: 15
    printf("%d\n", x);
}
