//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=26d469b87d05f8d3b2e0539d080d7839ae84df294d66110dd1541682d63c62d1
//META: purpose=`RWByteAddressBuffer::Store(offset, value)` surfaces the `byteAddressBufferStore(%buf, %offset, %alignment, %value)` opcode inside the pulled-in library function body.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `byteAddressBufferStore` parallels `byteAddressBufferLoad`. The
// opcode lives in the library `RWByteAddressBuffer.Store` body
// pulled into the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWByteAddressBuffer rwbabuf;
uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    rwbabuf.Store(idx * 4, idx + 7);
    rwbuf[idx] = idx;
}

// IR: byteAddressBufferStore({{.*}})
