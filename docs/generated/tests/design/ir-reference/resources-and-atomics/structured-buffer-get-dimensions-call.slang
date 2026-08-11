//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=26d469b87d05f8d3b2e0539d080d7839ae84df294d66110dd1541682d63c62d1
//META: purpose=`StructuredBuffer::GetDimensions(count, stride)` does not reach `func %main` as the `StructuredBufferGetDimensions` opcode — lowering leaves a call whose two `out` parameters are passed as `var` pointers, and the opcode appears only inside the library body.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies where the dimensions query sits relative to user code. Both
// `out` arguments are written back into an output buffer so nothing is
// dead. The IR directive pins the call under `func %main`, and the
// second, unanchored directive shows the opcode itself does exist in the
// dump — inside the specialized library function, not in the user body.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=OPC):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform StructuredBuffer<int> sbuf;
uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint count;
    uint stride;
    sbuf.GetDimensions(count, stride);
    rwbuf[tid.x] = count + stride;
}

// IR-LABEL: func %main
// IR: call {{.*}}(%sbuf, %count, %stride)

// OPC: StructuredBufferGetDimensions(%{{[A-Za-z0-9_]+}})
