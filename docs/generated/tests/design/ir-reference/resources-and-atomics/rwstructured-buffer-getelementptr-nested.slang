//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#rwstructuredbuffergetelementptr
//META: doc_section_digest=07cc97a7459e17e20f0600d7dea0434758c9364177e70232fe8f6681dd755c8f
//META: purpose=`rwBuf[i].field = v` surfaces as `rwstructuredBufferGetElementPtr(%base, %i)` followed by `get_field_addr` then `store`, the lvalue counterpart to `rwstructuredBufferLoad`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `rwstructuredBufferGetElementPtr` notable-opcode note
// says the opcode returns a pointer to the indexed element so that
// `Store` / `FieldAddress` / `GetElementPtr` opcodes can apply.
// A nested-struct-field write is the demonstration case.

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
