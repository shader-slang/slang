//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#include-resolution
//META: doc_section_digest=84fa78a16b12acc1c4a16a6900c45945fcf27cbb6ff16cb5e1d86c53e4450f09
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
