//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=3eac3aff8a5d5daa072f3cad641be3c7b7a2f0e1ecf59515c5939a7a993d85ec
//META: purpose=The `const T*` C-style const pointer declaration is rejected with E20017; the compiler suggests `Ptr<T, Access.Read>` or `let`/`const Ptr<...>` instead.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `const T*` C-style const pointer syntax is rejected.
// Declares a `const int*` and checks that E20017 fires; proves the
// compiler enforces the language restriction rather than silently accepting it.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cpp

void main()
{
    int x = 5;
    const int* p = &x;
//CHECK: E20017
}
