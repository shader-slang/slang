//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=26d469b87d05f8d3b2e0539d080d7839ae84df294d66110dd1541682d63c62d1
//META: purpose=`StructuredBuffer<S>::Load(idx)` on a struct element type produces `structuredBufferLoad` returning the struct value; field reads through `get_field` follow.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Exercise `structuredBufferLoad` with a struct element type. The
// follow-on `get_field` calls demonstrate that the load produces
// the struct value form (rvalue) — contrasted with the
// `rwstructuredBufferGetElementPtr`+`get_field_addr` lvalue path.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Item { int a; float b; }
uniform StructuredBuffer<Item> sbuf;
uniform RWStructuredBuffer<int> rwbuf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    Item it = sbuf.Load(idx);
    rwbuf[idx] = it.a;
    fbuf[idx] = it.b;
}

// IR-LABEL: func %main
// IR: structuredBufferLoad(%sbuf, %idx)
