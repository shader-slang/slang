//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=26d469b87d05f8d3b2e0539d080d7839ae84df294d66110dd1541682d63c62d1
//META: purpose=`ByteAddressBuffer::Load(offset)` surfaces the `byteAddressBufferLoad(%buf, %offset, %alignment)` opcode inside the pulled-in library function body for `ByteAddressBuffer.Load`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a byte-address load reaches the IR as its own opcode
// carrying buffer, byte offset, and alignment. The CHECK is not
// anchored at `func %main`: the opcode is created inside the
// core-module `Load` body that the dump pulls in, and the user body
// holds only the call that specializes it.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform ByteAddressBuffer babuf;
uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    uint v = babuf.Load(idx * 4);
    rwbuf[idx] = v;
}

// IR: byteAddressBufferLoad(%{{[^,]+}}, %{{[^,]+}}, {{.*}})
