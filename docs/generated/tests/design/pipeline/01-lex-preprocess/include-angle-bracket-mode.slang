//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#include-resolution
//META: doc_section_digest=15676a5139bfbfb4b33e12ca3d5ef744f531fa5b0c577d5608dc32037e46822a
//META: purpose=An angle-form `#include` builds its path by concatenating the raw tokens between `<` and `>` and resolves it in system-include mode.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the angle-form `#include` path handling: the header name
// here spans several raw tokens (identifiers, `-`, `.`), so it only
// resolves if the preprocessor concatenates the token run between
// `<` and `>` into one path string. If `<` and `>` were treated as
// ordinary comparison operators, the directive would not parse and
// the included declaration would never become visible. The bundle
// directory is passed via `-I` so the search-path lookup succeeds.

//TEST:INTERPRET(filecheck=CHECK):-I docs/generated/tests/design/pipeline/01-lex-preprocess

#include <include-angle-bracket-mode.slang.h>

void main()
{
    // CHECK: angle=33
    printf("angle=%d\n", angle_included_value);
}
