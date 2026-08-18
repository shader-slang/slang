//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=26d469b87d05f8d3b2e0539d080d7839ae84df294d66110dd1541682d63c62d1
//META: purpose=`rwbuf.Load(idx)` on a `RWStructuredBuffer<float4>` produces `rwstructuredBufferLoad(%buf, %idx)` with a vector result type, demonstrating the variadic-min-2 operand shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the load opcode's result type follows the buffer's
// element type without changing the opcode's operand shape. The element
// type here is a vector, and the CHECK still pins the two-operand
// `(buffer, index)` form under `func %main`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<float4> rwbuf;
uniform RWStructuredBuffer<float> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    float4 v = rwbuf.Load(idx);
    outbuf[idx] = v.x + v.y + v.z + v.w;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferLoad(%rwbuf, %idx)
