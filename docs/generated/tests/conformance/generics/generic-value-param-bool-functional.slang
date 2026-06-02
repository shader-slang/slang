//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic value parameter of Boolean type (`let B : bool`) is accepted and usable as a compile-time boolean to switch struct layout.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: a generic parameter can be a Boolean value.
// This verifies a bool value parameter is accepted and works correctly.

//TEST:INTERPRET(filecheck=CHECK):

struct FlaggedInt<let B : bool>
{
    int value;
}

void main()
{
    FlaggedInt<true>  t;
    t.value = 1;
    FlaggedInt<false> f;
    f.value = 0;
    // CHECK: 1
    printf("%d\n", t.value);
    // CHECK: 0
    printf("%d\n", f.value);
}
