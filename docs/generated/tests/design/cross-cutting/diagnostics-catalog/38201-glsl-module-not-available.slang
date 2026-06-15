//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38201 (glsl-module-not-available) - 'glsl' module not available
//META: intent=negative
//META: pipeline_stage=link
//META: catalog_code=38201
//META: catalog_name=glsl-module-not-available
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Importing the `glsl` builtin module without enabling GLSL
// support on the global session triggers E38201.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

import glsl;

void main() {}

// CHECK: E38201
