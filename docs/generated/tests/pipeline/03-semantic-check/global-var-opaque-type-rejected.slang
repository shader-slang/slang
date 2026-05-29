//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=9163d71b0ed4735c5c28e2877570515476519666c5a6da87256143a8fab12f94
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
