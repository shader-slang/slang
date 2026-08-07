//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#rwstructuredbuffergetelementptr
//META: doc_section_digest=1109150b01fc6cb7960b788c7bb9ca3bbae1748af0ae3a77abc2970881ef9206
//META: purpose=`rwBuf[i].field = v` surfaces as `rwstructuredBufferGetElementPtr(%base, %i)` followed by `get_field_addr` then `store`, the lvalue counterpart to `rwstructuredBufferLoad`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the element-pointer opcode is what makes a nested field
// write possible without a read-modify-write of the whole struct. Two
// different fields of the same element are assigned, and the CHECKs
// pin a `get_field_addr` chained onto each element pointer — the
// pointer-chaining the opcode exists to enable.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Item { int a; float b; }
uniform RWStructuredBuffer<Item> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    rwbuf[idx].a = idx + 1;
    rwbuf[idx].b = 1.5;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%rwbuf, %idx)
// IR: get_field_addr({{.*}}, %a)
// IR: rwstructuredBufferGetElementPtr(%rwbuf, %idx)
// IR: get_field_addr({{.*}}, %b)
