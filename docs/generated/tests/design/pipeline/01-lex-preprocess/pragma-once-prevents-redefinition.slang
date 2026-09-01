//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=81abdf54ca8e6b727b5142e63c4757374b3f566da88d8b9672ba7862e21bd136
//META: purpose=Boundary probe of `#pragma once`: a header with `#pragma once` is only processed once even when included twice, preventing duplicate declarations.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Includes the same `#pragma once`-guarded header twice. Without
// the guard, the second inclusion would re-declare `once_value`
// and the program would fail to compile. With the guard, the
// second inclusion is a no-op.

//TEST:INTERPRET(filecheck=CHECK):

#include "pragma-once-prevents-redefinition.slang.h"
#include "pragma-once-prevents-redefinition.slang.h"

void main()
{
    // CHECK: once=64
    printf("once=%d\n", once_value);
}
