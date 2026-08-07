//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugline
//META: doc_section_digest=eb5cc6b339fed6275ffa177c8c0c47a41583954b25feb65903b85fa94849df4a
//META: purpose=DebugLine is an ordinary instruction in the block's stream rather than a decoration, so the location travels with position in the block.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies where the line marker lives. Decorations print in brackets attached
// to the instruction that owns them, so the CHECK-NOTs on both sides assert no
// bracketed form exists anywhere in the dump, while the positive CHECK finds
// the record as a statement of its own inside the entry-point body. Debug
// level 1 keeps the module's source record from embedding a copy of this file,
// so the patterns cannot match their own text.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int local = int(tid.x) + 7;
    buf[0] = local;
}

// CHECK-NOT: [DebugLine
// CHECK: func %main
// CHECK: DebugLine(%{{[0-9]+}}, {{[0-9]+}} : UInt
// CHECK-NOT: [DebugLine
