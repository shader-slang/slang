//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#append-and-consume-buffers
//META: doc_section_digest=9b3916f6b072d28628147b6bbc63ceda33f37e485011da27f65d136633a21691
//META: purpose=`AppendStructuredBuffer<int>::Append(0)` (literal zero element) still surfaces the documented `StructuredBufferAppend(%buf, 0 : Int)` opcode shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: append a literal-zero element. The opcode shape
// is preserved regardless of value.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform AppendStructuredBuffer<int> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    abuf.Append(0);
    rwbuf[tid.x] = (int)tid.x;
}

// IR-LABEL: func %main
// IR: StructuredBufferAppend(%abuf, 0 : Int)
