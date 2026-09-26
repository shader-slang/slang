//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=d777e7e545183b84ac14d2cc8d732d7c0b1b01910e752fb92143872dd884e3ce
//META: purpose=`StructuredBuffer::Load(INT_MAX)` (largest positive `int` literal) still surfaces the documented `structuredBufferLoad` opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: max positive index literal on the read-only buffer.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform StructuredBuffer<int> sbuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = sbuf.Load(2147483647);
    rwbuf[tid.x] = v;
}

// IR-LABEL: func %main
// IR: structuredBufferLoad(%sbuf, 2147483647 : Int)
