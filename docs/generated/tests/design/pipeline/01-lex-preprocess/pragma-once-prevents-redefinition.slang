//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
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
