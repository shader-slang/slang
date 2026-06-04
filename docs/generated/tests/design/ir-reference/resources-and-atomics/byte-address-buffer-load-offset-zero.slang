//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=a5cc503aaed86633e5970cfcf6ca63262c9a68fc95f6b94e9a9eaf7fe399c9fd
//META: purpose=`ByteAddressBuffer::Load(0)` (offset 0, lower edge) still surfaces the documented `byteAddressBufferLoad` opcode inside the pulled-in library helper body.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: byte offset 0, the lower edge.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform ByteAddressBuffer babuf;
uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint v = babuf.Load(0);
    rwbuf[tid.x] = v;
}

// IR: byteAddressBufferLoad(%{{[^,]+}}, {{.*}})
