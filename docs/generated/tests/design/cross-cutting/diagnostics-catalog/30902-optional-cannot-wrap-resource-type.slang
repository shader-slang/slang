//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=29ecac99c559d14c83fe88002bb4fdd8d4f6e2f25cdbd7755b301ae7f76c5321
//META: purpose=Fires diagnostic E30902 (optional-cannot-wrap-resource-type) -- 'Optional<T>' cannot wrap a resource or opaque type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30902
//META: catalog_name=optional-cannot-wrap-resource-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

Optional<Texture2D> t;
[shader("compute")][numthreads(1,1,1)]
void main() {}


//CHECK: E30902
