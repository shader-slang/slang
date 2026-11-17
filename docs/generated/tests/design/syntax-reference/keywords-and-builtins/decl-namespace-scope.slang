//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=38bbbd7083f0dd5470308b28e015e307c9b17c55d97e2e766bc45629df65ad7a
//META: purpose=`namespace name { ... }` is a decl keyword introducing a named scope whose members are reached with `::` qualification.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declare a `namespace nest` containing a function, then call it from
// `main` via the `::` qualified-name form documented in
// `tokens.md`.

//TEST:INTERPRET(filecheck=CHECK):

namespace nest
{
    int answer() { return 42; }
}

void main()
{
    // CHECK: a=42
    printf("a=%d\n", nest::answer());
}
