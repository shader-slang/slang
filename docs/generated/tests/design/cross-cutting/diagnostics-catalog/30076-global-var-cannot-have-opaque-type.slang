//META: generated=true
//META: model=codex-gpt-5
//META: generated_at=2026-09-22T18:48:20-07:00
//META: source_commit=6cd41833fb423518bd28ff916d869edbc5a047e8
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c9937e090f68861fd3cf6c2dd9e06e19656064efe27f6e29086f92a17cebd282
//META: purpose=Fires diagnostic E30076 (global-var-cannot-have-opaque-type) - global variable cannot have opaque type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30076
//META: catalog_name=global-var-cannot-have-opaque-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Parameter blocks have a target-dependent legalized representation that resource-global
// replacement does not yet support, so the front end rejects this declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct MaterialState
{
    float value;
}

static ParameterBlock<MaterialState> gParams;
void main() { }

//CHECK: E30076
