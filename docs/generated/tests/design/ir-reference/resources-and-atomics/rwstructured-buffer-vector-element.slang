//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#rwstructuredbuffergetelementptr
//META: doc_section_digest=07cc97a7459e17e20f0600d7dea0434758c9364177e70232fe8f6681dd755c8f
//META: purpose=`rwbuf[i] = vector_expr` for a vector element type also routes through `rwstructuredBufferGetElementPtr` to produce the lvalue, then stores the vector.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Re-exercise the lvalue path with a vector element type so the
// `rwstructuredBufferGetElementPtr` claim is also covered for
// non-scalar elements.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<float4> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    rwbuf[idx] = float4((float)idx, 1, 2, 3);
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%rwbuf, %idx)
