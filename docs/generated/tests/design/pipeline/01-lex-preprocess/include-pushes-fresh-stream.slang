//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#include-resolution
//META: doc_section_digest=15676a5139bfbfb4b33e12ca3d5ef744f531fa5b0c577d5608dc32037e46822a
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
