//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6165fbd3f46bb23f31d4481667f01c7f841053292ade0127deceffcbed0876dc
//META: purpose=Decl checking rejects a `static` global variable with an opaque (resource) type (E30076).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Decl-checking boundary: opaque types like `Texture2D` cannot
// back a static global variable; the diagnostic suggests `uniform`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static Texture2D g;
//CHECK:         ^ global variable cannot have opaque type
//CHECK:         ^ global variable cannot have opaque type.
// CHECK: E30076

void main() {}
