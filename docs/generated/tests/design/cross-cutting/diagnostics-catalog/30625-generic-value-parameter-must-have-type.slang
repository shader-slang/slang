//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f52608ae8913d1bd43d8cdae24d612733800fd9cbea6acc2b22265de4c81b8db
//META: purpose=Fires diagnostic E30625 (generic-value-parameter-must-have-type) -- a generic value parameter must be given an explicit type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30625
//META: catalog_name=generic-value-parameter-must-have-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S<let x> {}


// CHECK: E30625
