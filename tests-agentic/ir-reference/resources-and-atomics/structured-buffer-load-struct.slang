//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=a5cc503aaed86633e5970cfcf6ca63262c9a68fc95f6b94e9a9eaf7fe399c9fd
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
