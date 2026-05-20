//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=The first struct member always receives OpMemberDecorate ... 0 Offset 0 (lower edge of the offset space).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: lower edge of the struct-member Offset decoration
// (member 0 lives at byte offset 0).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

struct Pair { float4 a; float4 b; }
RWStructuredBuffer<Pair> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pair p;
    p.a = float4(1, 2, 3, 4);
    p.b = float4(5, 6, 7, 8);
    outBuf[tid.x] = p;
}

// CHECK: OpMemberDecorate %Pair{{[a-zA-Z_0-9]*}} 0 Offset 0
