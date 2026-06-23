//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=6781a875f1a289487fa03eec21a40443d8dfa61c8336599fcc900f8a67cb0b2d
//META: purpose=Fires diagnostic E31000 (unknown-attribute-name) -- unknown attribute
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31000
//META: catalog_name=unknown-attribute-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[unknownAttr]
[shader("compute")][numthreads(1,1,1)]
void main() {}


// CHECK: E31000
