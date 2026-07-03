//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#include-resolution
//META: doc_section_digest=5fc1e61e2e4e9dfe068749f6d012f4fdc9db1275a2d0ce277dc3ce75ae347eeb
//META: purpose=`#include` resolves the named string against `IncludeSystem` and pushes a fresh input stream so the file's tokens become part of the enclosing translation unit.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an `#include`d file contributes its declarations to
// the parent unit. The helper header defines `included_value`; if
// the include were not resolved, the reference here would not
// compile.

//TEST:INTERPRET(filecheck=CHECK):

#include "include-pushes-fresh-stream.slang.h"

void main()
{
    // CHECK: included=91
    printf("included=%d\n", included_value);
}
