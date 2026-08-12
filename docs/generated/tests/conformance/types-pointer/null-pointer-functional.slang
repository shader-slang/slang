//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=7d0d8d99e81aaca2b6f41962be8f0bffe859a83dddeba6909e61b74e024ba2ff
//META: purpose=A null pointer (`nullptr`) is a valid pointer value that compares equal to `nullptr` and not equal to a non-null pointer.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `nullptr` is a valid pointer value and that null/non-null comparison
// produces the right boolean. Compares a null pointer and a real pointer each
// against nullptr; CHECK asserts the results are 1 and 0 respectively.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 5;
    int* nul = nullptr;
    int* non = &x;
    int is_null   = (nul == nullptr) ? 1 : 0;  // 1
    int is_notnull = (non == nullptr) ? 1 : 0;  // 0
    //CHECK: 1 0
    printf("%d %d\n", is_null, is_notnull);
}
