//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
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
