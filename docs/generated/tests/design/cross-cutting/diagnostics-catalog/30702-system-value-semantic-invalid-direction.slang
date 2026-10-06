//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ad37e7b1ded73fd7e89b3b464848cba5a05fb9dd3437486581cebc69d650462d
//META: purpose=Fires diagnostic E30702 (system-value-semantic-invalid-direction) -- system value semantic '~semantic' cannot be used as ~direction in '~stage' shader stage
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30702
//META: catalog_name=system-value-semantic-invalid-direction
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[shader("vertex")]
void main(out float3 d : SV_VertexID) {}


//CHECK: E30702
