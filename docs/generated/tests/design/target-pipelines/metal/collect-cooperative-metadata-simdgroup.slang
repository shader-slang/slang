//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=collectCooperativeMetadata runs for Metal when targetCaps imply cooperative_matrix: the CoopMat type lowers to MSL's simdgroup_matrix<T, Cols, Rows>, with fill+store rendered into the simdgroup intrinsics.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `collectCooperativeMetadata` attaches per-entry-point metadata
// when the target capability set includes `cooperative_matrix` or
// `cooperative_vector`. The metadata itself is not visible in
// emit, but the corresponding MSL type rendering (`simdgroup_matrix`)
// is observable; that rendering only runs when the cooperative
// metadata pass has succeeded for this entry point.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

using namespace linalg;

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(32, 1, 1)]
void main()
{
    var acc : CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>;
    acc.fill(7.0);
    acc.Store<CoopMatMatrixLayout.RowMajor>(outBuf, 0u, 8u);
}

// CHECK: #include <metal_stdlib>
// CHECK: simdgroup_matrix<
// CHECK: kernel{{.*}} void main_0
