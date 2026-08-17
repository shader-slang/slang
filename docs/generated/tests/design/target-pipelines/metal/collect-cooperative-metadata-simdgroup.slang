//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=5fdd0ac6daa767e15b8f1dad12d2d7a2846fe891f83f100e2466f794797eb465
//META: purpose=`collectCooperativeMetadata` runs when the target capability set implies `cooperative_matrix`, and the cooperative matrix type renders as MSL `simdgroup_matrix<...>`.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the capability-gated cooperative-metadata path is reachable on
// Metal. The metadata record itself is not printed in the emitted source,
// but the MSL type rendering is: a `CoopMat` only reaches emit as
// `simdgroup_matrix<...>` once the cooperative path has accepted the entry
// point. The matrix is filled and stored so it escapes DCE, and CHECK
// asserts both the prelude and the `simdgroup_matrix<` spelling.

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
