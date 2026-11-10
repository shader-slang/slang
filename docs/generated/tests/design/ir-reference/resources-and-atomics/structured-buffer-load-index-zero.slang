//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=539f4d826195c55c809a7bdee601b791ce9a4d320d577f17983cfe774c4e7973
//META: purpose=`StructuredBuffer::Load(0)` (lower-edge index) still surfaces `structuredBufferLoad(%buf, 0 : Int)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: index 0 on the read-only structured-buffer path.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform StructuredBuffer<int> sbuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = sbuf.Load(0);
    rwbuf[tid.x] = v;
}

// IR-LABEL: func %main
// IR: structuredBufferLoad(%sbuf, 0 : Int)
