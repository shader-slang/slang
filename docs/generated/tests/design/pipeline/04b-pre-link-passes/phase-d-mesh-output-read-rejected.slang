//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=de3af7e62fdf97582267b3c674b87776ce5d49cf79a3d54ae3a770a3c679cad0
//META: purpose=Phase D's checkForMeshOutputReads (D9) rejects reading a value back out of a mesh-shader output array.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the mesh-output validator. The shader writes a vertex output
// and then reads the same element back, which is the one pattern the pass
// exists to reject. The code CHECK pins the error identity; the shader is
// only compiled to SPIR-V text, so no mesh-capable device is needed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -stage mesh -entry main

struct Vertex { float4 pos : SV_Position; }

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(out vertices Vertex verts[3], out indices uint3 tris[1])
{
    SetMeshOutputCounts(3, 1);
    verts[0].pos = float4(0, 0, 0, 1);
    float4 p = verts[0].pos;
    // CHECK: E54005
    verts[1].pos = p;
}
