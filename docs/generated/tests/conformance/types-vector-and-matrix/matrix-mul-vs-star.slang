//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#operators
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=`*` is element-wise multiplication on same-shaped matrices; `mul()` is matrix multiplication — the doc's explicit GLSL-divergence remark.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc carries an explicit Remark: "The operator `*` performs
// element-wise multiplication... `mul()` differs from GLSL where
// `*` performs matrix multiplication." Verify the distinction by
// comparing the two products of the same input matrices.
//
// 2x2 example:
//   m1 = [[1, 2], [3, 4]]
//   m2 = [[5, 6], [7, 8]]
//   m1 * m2  (element-wise) = [[5, 12], [21, 32]]
//   mul(m1, m2)             = [[1*5+2*7, 1*6+2*8],
//                              [3*5+4*7, 3*6+4*8]]
//                           = [[19, 22], [43, 50]]

//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

// `*` is element-wise; `mul()` is matrix multiplication. The claim is
// pinned by checking both operations appear and differ in emitted code.
// Target HLSL emits m1_0 * m2_0 (element-wise) and mul(m1_0, m2_0) (builtin).
// Targets CUDA and CPP emit m1_0 * m2_0 and a lowered mul_0 helper.
// Target GLSL decomposes matrices to row-arrays; not checked (fragile).
// Target SPIRV is very large (loop expanded); not checked.
// Targets Metal and WGSL reject printf in compute; untested for emission.

void main()
{
    matrix<int, 2, 2> m1 = { 1, 2, 3, 4 };
    matrix<int, 2, 2> m2 = { 5, 6, 7, 8 };

    matrix<int, 2, 2> ew  = m1 * m2;          // element-wise
    matrix<int, 2, 2> mm  = mul(m1, m2);      // matrix multiply

    //CHECK: ew=5 12 21 32
    //CHECK: mm=19 22 43 50
    //HLSL-DAG: ew_0 = m1_0 * m2_0
    //HLSL-DAG: mm_0 = mul(m1_0, m2_0)
    //CUDA-DAG: ew_0 = m1_0 * m2_0
    //CUDA-DAG: mm_0 = mul_0(m1_0, m2_0)
    //CPP-DAG: ew_0 = m1_0 * m2_0
    //CPP-DAG: mm_0 = mul_0(m1_0, m2_0)
    printf("ew=%d %d %d %d\n", ew[0][0], ew[0][1], ew[1][0], ew[1][1]);
    printf("mm=%d %d %d %d\n", mm[0][0], mm[0][1], mm[1][0], mm[1][1]);
}
