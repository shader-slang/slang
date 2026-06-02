//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#append-and-consume-buffers
//META: doc_section_digest=e274afd53362e00b53effe2d33f7a820c8f72cd2555df63f0be832ce50f6509d
//META: purpose=`AppendStructuredBuffer<int>::Append(INT_MAX)` (largest signed value) still surfaces the documented `StructuredBufferAppend(%buf, 2147483647 : Int)` opcode shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: append the largest signed int literal.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform AppendStructuredBuffer<int> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    abuf.Append(2147483647);
    rwbuf[tid.x] = (int)tid.x;
}

// IR-LABEL: func %main
// IR: StructuredBufferAppend(%abuf, 2147483647 : Int)
