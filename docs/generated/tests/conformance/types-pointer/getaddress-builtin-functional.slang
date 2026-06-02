//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=7d0d8d99e81aaca2b6f41962be8f0bffe859a83dddeba6909e61b74e024ba2ff
//META: purpose=`__getAddress(obj)` is an alternative to the `&` operator; both yield a pointer whose dereference gives back the original value.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__getAddress(obj)` is an alternative to `&obj` for obtaining a pointer.
// Calls `__getAddress(x)` and dereferences the result; CHECK asserts the
// read-back value equals the original, confirming the builtin works like `&`.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 77;
    int* p = __getAddress(x);
    //CHECK: 77
    printf("%d\n", *p);
}
