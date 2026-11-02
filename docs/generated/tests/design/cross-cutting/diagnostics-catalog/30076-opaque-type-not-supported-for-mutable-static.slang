//META: generated=true
//META: model=codex-gpt-5
//META: generated_at=2026-09-22T18:48:20-07:00
//META: source_commit=6cd41833fb423518bd28ff916d869edbc5a047e8
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=29920ddd070679f18c62e8e6284ecd2353e1535e070d677b6ad901de0d233ae9
//META: purpose=Fires diagnostic E30076 (opaque-type-not-supported-for-mutable-static) - opaque type is not supported for mutable static storage
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30076
//META: catalog_name=opaque-type-not-supported-for-mutable-static
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Constant buffers have a target-dependent legalized representation that resource-global
// replacement does not support, so the front end rejects this declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct MaterialState
{
    float value;
}

static ConstantBuffer<MaterialState> gBuffer;
void main() { }

//CHECK: E30076
