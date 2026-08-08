//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
//META: purpose=A StructuredBuffer of a 3-float4 struct emits ArrayStride 48 on the runtime array (16 bytes per float4 * 3 fields).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: larger stride than the int(4) baseline. A struct
// with three float4 members is 48 bytes; the runtime-array
// inside the block carries ArrayStride 48.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

struct Big { float4 a; float4 b; float4 c; }
RWStructuredBuffer<Big> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Big v;
    v.a = float4(1, 2, 3, 4);
    v.b = float4(5, 6, 7, 8);
    v.c = float4(9, 10, 11, 12);
    outBuf[tid.x] = v;
}

// CHECK: OpDecorate %_runtimearr_Big{{[a-zA-Z_0-9]*}} ArrayStride 48
