//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#rwstructuredbuffergetelementptr
//META: doc_section_digest=c1926379566a798f6e26928651f281675df5e998bf0bf008f9481bf70aad885c
//META: purpose=`rwbuf[i] = vector_expr` for a vector element type also routes through `rwstructuredBufferGetElementPtr` to produce the lvalue, then stores the vector.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Re-exercise the lvalue path with a vector element type so the
// `rwstructuredBufferGetElementPtr` claim is also covered for
// non-scalar elements.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<float4> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    rwbuf[idx] = float4((float)idx, 1, 2, 3);
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%rwbuf, %idx)
