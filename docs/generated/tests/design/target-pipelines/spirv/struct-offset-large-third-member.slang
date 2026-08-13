//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
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
