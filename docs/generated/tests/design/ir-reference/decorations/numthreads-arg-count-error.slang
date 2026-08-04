//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=a78fa1c65d9e97005484d3326f0e07d9af56a6dadaad4955b0d779effe950727
//META: purpose=Negative - [numthreads()] with zero arguments fires E31001 wrong-argument-count, confirming the attribute arity validator runs before lowering.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative - the doc lists numThreads as (variadic, min=3) but the
// front-end accepts a single-arg form (defaulting y, z to 1). At
// zero arguments the arity check fires E31001 before LOWER-TO-IR.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main
uniform RWStructuredBuffer<int> buf;

[numthreads()]
/*CHECK:
 ^^^^^^^^^^ wrong number of attribute arguments
*/
// CHECK: E31001
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}
