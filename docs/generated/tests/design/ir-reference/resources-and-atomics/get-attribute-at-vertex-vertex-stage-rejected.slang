//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=212a9e3fd4ca324be91b034872f2b7a9afdb1fe8ac5acfc9f068664d69c3e326
//META: purpose=The `fragment`-stage requirement behind `ResolveVaryingInputRef` / `GetPerVertexInputArray` is enforced by capability checking, so the same `GetAttributeAtVertex` call in a `vertex` entry point is rejected with `E36107`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out attached to the two per-vertex Shader-IO opcodes:
// the surface that produces them is stage-restricted, and the restriction
// is a capability check rather than a lowering failure.
//
// The shader is the fragment test's body moved verbatim onto a `vertex`
// entry point. The diagnostic is reported against the entry-point name
// rather than the call site, so the caret span sits on `main`; a following
// note points back at the offending use, which the second annotation
// consumes. `non-exhaustive` is set because two further notes point into
// `hlsl.meta.slang` (the declaration and its `require` clause), which this
// file cannot annotate.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage vertex -entry main

struct VIn
{
    nointerpolation float3 color : COLOR;
};

float4 main(VIn input) : SV_Position
/*CHECK:
       ^^^^ E36107
       ^^^^ entrypoint 'main' uses features that are not available in 'vertex' stage for 'hlsl' compilation target.
*/
{
        return float4(GetAttributeAtVertex(input.color, 1), 1.0);
//CHECK:              ^^^^^^^^^^^^^^^^^^^^ see using of 'GetAttributeAtVertex'
}
