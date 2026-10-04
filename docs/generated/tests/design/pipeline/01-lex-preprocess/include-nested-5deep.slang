//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#include-resolution
//META: doc_section_digest=15676a5139bfbfb4b33e12ca3d5ef744f531fa5b0c577d5608dc32037e46822a
//META: purpose=Stress probe of nested `#include`: chain depth 5 (this file -> d1 -> d2 -> d3 -> d4 -> d5) leaves the leaf declaration visible at the root.
//META: intent=stress
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// This file includes a header that itself includes a header, four
// levels down. The preprocessor must push a fresh input stream at
// each level and pop on EOF, leaving the leaf-defined constant
// `deep_value` visible here.

//TEST:INTERPRET(filecheck=CHECK):

#include "include-nested-5deep-1.slang.h"

void main()
{
    // CHECK: deep=12345
    printf("deep=%d\n", deep_value);
}
