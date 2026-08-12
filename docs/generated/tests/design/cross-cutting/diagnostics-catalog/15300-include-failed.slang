//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=4aac90154fb329a06af74d7313f4aed1528cb2f6d787dda8eccd7417eea70589
//META: purpose=Fires diagnostic E15300 (include-failed) - include file not found
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15300
//META: catalog_name=include-failed
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#include` resolves its path against the search paths. A file present on none
// of them fails the include; the CHECK pins the include-failed error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#include "no-such-file-xyz.h"
void main() {}

//CHECK: E15300
