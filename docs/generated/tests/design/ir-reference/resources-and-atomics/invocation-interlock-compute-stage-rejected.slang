//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#barriers-and-synchronization
//META: doc_section_digest=46f99bd5a35473a9163ccbd42c4c906fa8dad92dac17c85d7c15483893456718
//META: purpose=The two interlock opcodes require a `fragment` entry point because the rasterizer ordering they request has no meaning in another stage, so `beginInvocationInterlock()` in a `compute` entry point is rejected with `E36107`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the stage restriction the barrier section attaches to the two
// fragment-shader-interlock rows, and that it is enforced by capability
// checking rather than discovered at emit.
//
// The body is the fragment interlock test's critical section rewritten
// around a buffer store on a `compute` entry point. The diagnostic lands
// on the entry-point name, and a note points back at the first offending
// call. `non-exhaustive` is set because two further notes point into
// `core.meta.slang` (the declaration and its `require` clause) and one
// more names `endInvocationInterlock`, none of which this file can
// annotate.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
/*CHECK:
     ^^^^ E36107
     ^^^^ entrypoint 'main' uses features that are not available in 'compute' stage for 'spirv' compilation target.
*/
{
        beginInvocationInterlock();
//CHECK:^^^^^^^^^^^^^^^^^^^^^^^^ see using of 'beginInvocationInterlock'
    outBuf[tid.x] = 1.0;
    endInvocationInterlock();
}
