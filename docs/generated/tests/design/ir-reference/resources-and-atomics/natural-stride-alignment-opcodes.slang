//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#getnaturalstride-and-getnaturalalignment
//META: doc_section_digest=f9dd6ab11230caa957996ff65d806f0c11d79b3dcbcd7bdefa014d3e4c00fa43
//META: purpose=`__naturalStrideOf<T>()` and `__naturalAlignmentOf<T>()` surface as the `getNaturalStride` and `getNaturalAlignment` opcodes, each taking a single placeholder *value* operand whose type is what the opcode inspects.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the natural-layout query family reach the IR
// as their own opcodes. The two results are summed and written to a
// buffer so neither is dead, and the CHECKs match each opcode with a
// single operand — the `__declVal<T>()` placeholder the inline wrapper
// passes — which is the shape the operand-naming note describes.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    rwbuf[idx] = __naturalStrideOf<float4>() + __naturalAlignmentOf<float3>();
}

// IR-LABEL: func %main
// IR: getNaturalStride(%{{[A-Za-z0-9_]+}})
// IR: getNaturalAlignment(%{{[A-Za-z0-9_]+}})
