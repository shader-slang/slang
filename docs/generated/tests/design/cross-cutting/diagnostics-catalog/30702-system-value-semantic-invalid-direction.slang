//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=533a54cf9ac3cc62ed0f94be2773b306310d4c70af89eeb972128d5794162ad6
//META: purpose=Fires diagnostic E30702 (system-value-semantic-invalid-direction) -- system value semantic '~semantic' cannot be used as ~direction in '~stage' shader stage
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30702
//META: catalog_name=system-value-semantic-invalid-direction
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[shader("vertex")]
void main(out float3 d : SV_VertexID) {}


// CHECK: E30702
