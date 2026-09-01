//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=bb7deea7eeeccc7c9615fcb5f5768b18c9feabc385f792990703a6dfe2183b21
//META: purpose=Fires diagnostic E00097 (library-does-not-exist) - library '~path' does not exist
//META: intent=negative
//META: pipeline_stage=link
//META: catalog_code=97
//META: catalog_name=library-does-not-exist
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-r` loads a serialized library from disk. A path that does not exist cannot
// be loaded; the CHECK pins the library-does-not-exist error so a missing
// library never degrades into a silent no-op.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-r /nonexistent-dir-xyz/lib.slang-module -target hlsl -entry main -stage compute
[numthreads(1,1,1)]
void main() {}

//CHECK: E00097
