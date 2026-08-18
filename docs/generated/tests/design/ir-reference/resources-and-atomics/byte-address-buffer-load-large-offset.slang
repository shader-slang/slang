//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=26d469b87d05f8d3b2e0539d080d7839ae84df294d66110dd1541682d63c62d1
//META: purpose=`ByteAddressBuffer::Load(2147483644)` (large 4-byte-aligned offset literal, near INT_MAX) still surfaces the documented `byteAddressBufferLoad` opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: large positive byte offset near INT_MAX (rounded down
// to the nearest 4-byte alignment).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform ByteAddressBuffer babuf;
uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint v = babuf.Load(2147483644);
    rwbuf[tid.x] = v;
}

// IR: byteAddressBufferLoad(%{{[^,]+}}, {{.*}})
