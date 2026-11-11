//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#include-resolution
//META: doc_section_digest=c2e213c5ebf251bc754e66bc7a31f528eda69d9722a5e92902578dc1b5bee22d
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
