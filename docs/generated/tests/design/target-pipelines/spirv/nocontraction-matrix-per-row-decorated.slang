//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#capabilities-and-extensions-decided-at-emit-time
//META: doc_section_digest=a1bda696e594d0d008416384d3cd2b7b449a57153c72f78ceef01aa513a35c5e
//META: purpose=For a matrix operation under precise mode the per-row results are decorated with NoContraction individually and the OpCompositeConstruct that reassembles the matrix is left undecorated.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the matrix carve-out of the NoContraction rule. A matrix add has
// no single emitted opcode to decorate: it becomes one OpFAdd per row plus a
// composite construct, and the construct is not a valid decoration target.
// The matrix comes from a constant buffer so the addition is not folded. The
// CHECKs assert a decorated per-row OpFAdd and then that nothing after the
// reassembling OpCompositeConstruct carries the decoration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -fp-mode precise

struct M { float4x4 m; };
ConstantBuffer<M> cb;
RWStructuredBuffer<float4x4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4x4 a = cb.m;
    outBuf[tid.x] = a + a;
}

// CHECK: OpDecorate %{{[A-Za-z0-9_]+}} NoContraction
// CHECK: OpFAdd %v4float
// CHECK-SAME: NoContraction
// CHECK: OpCompositeConstruct %{{[A-Za-z0-9_]+}}
// CHECK-NOT: NoContraction
