//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=a78fa1c65d9e97005484d3326f0e07d9af56a6dadaad4955b0d779effe950727
//META: purpose=Negative - [numthreads(0,1,1)] is rejected with E31102 "expected a positive integer", confirming the value validator catches the lower-edge invalid input before any numThreads decoration is created.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative - the doc says numThreads carries (variadic, min=3)
// integer operands but does not enumerate the legal range. The
// checker rejects 0 with diagnostic E31102 before LOWER-TO-IR.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -stage compute -entry main
uniform RWStructuredBuffer<int> buf;

[numthreads(0, 1, 1)]
/*CHECK:
 ^^^^^^^^^^ invalid 'numthreads' value
 ^^^^^^^^^^ expected a positive integer in 'numthreads' attribute, got '0'
*/
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}
