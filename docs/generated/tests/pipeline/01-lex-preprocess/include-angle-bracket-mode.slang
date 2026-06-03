//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules
//META: doc_section_digest=ab0cf5e019a8433b239b8a025cd2c3014f7a03ebec00c3c2c6d172456d9b0e3c
//META: purpose=After a `#include` directive the lexer treats `<foo/bar.h>` as a single string-like token rather than a comparison expression.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the special `<...>` lexing mode after `#include`. If the
// lexer treated `<` and `>` as comparison operators, the directive
// would not parse and the included declaration would never become
// visible. The bundle directory is passed via `-I` so the
// `<>`-form include resolves.

//TEST:INTERPRET(filecheck=CHECK):-I docs/generated/tests/pipeline/01-lex-preprocess

#include <include-angle-bracket-mode.slang.h>

void main()
{
    // CHECK: angle=33
    printf("angle=%d\n", angle_included_value);
}
