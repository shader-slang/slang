//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=90e08e718ef3c42629a46041937d0ea97ce192cb87a0b3e414306237480cd0b9
//META: purpose=Fires diagnostic E1 (cannot-open-file) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=1
//META: catalog_name=cannot-open-file
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E1.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
import nonexistent_module_xyz_pqrs;
void main() {}

//CHECK: E00001
