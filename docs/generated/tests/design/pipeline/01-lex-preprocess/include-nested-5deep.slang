//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#include-resolution
//META: doc_section_digest=5fc1e61e2e4e9dfe068749f6d012f4fdc9db1275a2d0ce277dc3ce75ae347eeb
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
