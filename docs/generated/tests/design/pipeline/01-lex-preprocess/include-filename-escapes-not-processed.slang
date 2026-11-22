//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=6c5b24137d8a00d45340c81231f003fe2cc4825dcc7a3203d181feeb7da8b977
//META: purpose=`#include` filenames are decoded without escape processing, so a backslash-x sequence in the path is looked up (and reported) verbatim.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the filename variant of string decoding: the include path
// below contains the characters `\x41`, which would decode to `A`
// under normal string-escape processing. The missing-include
// diagnostic must report the path with `\x41` intact — a report of
// `no-such-dirA-missing...` would mean escapes were processed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#include "no-such-dir\x41-missing.slang.h"
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ include file not found
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ failed to find include file 'no-such-dir\x41-missing.slang.h'
// CHECK: E15300
// CHECK: no-such-dir\x41-missing.slang.h

void main() {}
