//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/values.md#modifier-values
//META: doc_section_digest=cc9b5af8e7e88defd3d066b93057c4ed4b13419aa0e459c9e09a634d498558b7
//META: purpose=A resource-format modifier value attached to a float or a vector of floats does not change that value's arithmetic semantics, so a unorm-modified operand computes the same result as the bare spelling.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "does not change the semantics" half of the resource-format
// modifier claim. The modifier is a value stored on the *type*, so it travels
// with every later decision about that type; the claim is that arithmetic is
// not one of those decisions. The test computes the same expression twice —
// once through `unorm float` / `unorm float4` operands and once through the
// bare spellings — and CHECKs that both print the identical result. A
// difference in either number would mean the modifier had altered the value
// rather than only its storage format.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    unorm float a = 0.25;
    float b = 0.25;

    unorm float4 v = float4(0.5, 0.25, 0.125, 1.0);
    float4 w = float4(0.5, 0.25, 0.125, 1.0);

    printf("scalar=%f %f vector=%f %f\n",
           a * 3.0f,
           b * 3.0f,
           v.x + v.y + v.z,
           w.x + w.y + w.z);
}

//CHECK: scalar=0.750000 0.750000 vector=0.875000 0.875000
