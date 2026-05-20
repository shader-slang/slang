//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=The third member of a struct of three float4 fields receives OpMemberDecorate ... 2 Offset 32 (16-byte stride accumulates to a larger byte offset).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: the upper-edge Offset decoration of this struct
// (member 2 sits at byte 32, accumulating two float4s of
// 16 bytes each).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

struct Three { float4 a; float4 b; float4 c; }
RWStructuredBuffer<Three> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Three t;
    t.a = float4(1, 2, 3, 4);
    t.b = float4(5, 6, 7, 8);
    t.c = float4(9, 10, 11, 12);
    outBuf[tid.x] = t;
}

// CHECK-DAG: OpMemberDecorate %Three{{[a-zA-Z_0-9]*}} 0 Offset 0
// CHECK-DAG: OpMemberDecorate %Three{{[a-zA-Z_0-9]*}} 1 Offset 16
// CHECK-DAG: OpMemberDecorate %Three{{[a-zA-Z_0-9]*}} 2 Offset 32
