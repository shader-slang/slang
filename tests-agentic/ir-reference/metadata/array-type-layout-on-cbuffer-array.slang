//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/metadata.md#layout-family
//META: doc_section_digest=295e5ac09f4b7f77dd6669103c943586264fcbb5b6e2283e5226b00e48c0393b
//META: purpose=An array-typed cbuffer field induces an arrayTypeLayout opcode in the layout dump.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Layout-family table: `arrayTypeLayout` is the layout for
// an array type (variadic, min=1, hoistable, synthesized).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

cbuffer CB
{
    float arr[4];
}

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = arr[tid.x % 4];
}

// CHECK: arrayTypeLayout
