//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/metadata.md#attr-family
//META: doc_section_digest=f3430c3e5f3d15f08a7b508e5bb6b1ba5106fa40b74c0d99d17989a7e31e58a8
//META: purpose=The size Attr opcode records per-resource-kind size on a typeLayout; it appears as size(kind, N) in the layout dump.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Attr-family table: `size` is a hoistable attribute
// (variadic, min=2) keyed by resource kind. It records a type's
// size in a typeLayout. The first operand is the resource-kind
// enum (integer); the second is the size value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = size({{[0-9]+}} : Int, {{[0-9]+}} : Int)
