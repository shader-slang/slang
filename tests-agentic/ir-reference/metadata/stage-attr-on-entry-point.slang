//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/metadata.md#attr-family
//META: doc_section_digest=f3430c3e5f3d15f08a7b508e5bb6b1ba5106fa40b74c0d99d17989a7e31e58a8
//META: purpose=The stage attribute tags an EntryPointLayout with its pipeline-stage operand; compute is stage 6.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Attr-family table: `stage` is a hoistable attribute whose
// single operand is an `IRIntLit` stage tag. The dump prints it as
// `stage(N : Int)` inside the EntryPointLayout tree. Compute is 6.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = stage(6 : Int)
