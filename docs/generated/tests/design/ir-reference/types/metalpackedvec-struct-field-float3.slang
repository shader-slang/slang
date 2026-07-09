//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:21:07Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/ir-reference/types.md#metalpackedvec
//META: doc_section_digest=99021749fc5be40322edc33a942e6cb039cbc9b5719142b23c52923a90003bdd
//META: purpose=Expansion: a float3 field of a struct used as a Metal device-buffer element is rewritten to packed_float3 storage.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the buffer-element rewrite reaches vector fields nested inside a
// struct element, not just direct vector elements: rewriting the element
// type of a Metal device buffer must repack the struct's contents. The
// kernel reads both fields of a StructuredBuffer<Item> element; METAL
// asserts the emitted struct declaration carries a packed_float3 field.

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main

struct Item
{
    float3 dir;
    float weight;
};

StructuredBuffer<Item> items;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Item it = items[tid.x];
    output[tid.x] = it.dir.x * it.weight;
}

// METAL-DAG: struct Item
// METAL-DAG: packed_float3 dir
