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
// `*` performs matrix multiplication." Reworked to buffer-output so
// the shader compiles on Metal/WGSL (printf blocks with E36107).
// Function wrappers defeat constant-folding so both operations are
// visible in emitted function bodies. COMPARE_COMPUTE -cpu verifies
// the runtime values for both products.
// HLSL/CUDA/CPP emit the element-wise * and the mul() builtin/helper
// distinctly. GLSL/Metal/WGSL lower int-matrix to array-of-vectors;
// ewMul emits component-wise *, matMul emits a separate function.
// INTERPRET companion (main) keeps slangi-VM value coverage.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:INTERPRET(filecheck=ICHECK):

RWStructuredBuffer<int> outputBuffer;

matrix<int, 2, 2> ewMul(matrix<int, 2, 2> a, matrix<int, 2, 2> b) { return a * b; }
matrix<int, 2, 2> matMul(matrix<int, 2, 2> a, matrix<int, 2, 2> b) { return mul(a, b); }

[numthreads(1, 1, 1)]
void computeMain()
{
    matrix<int, 2, 2> m1 = { 1, 2, 3, 4 };
    matrix<int, 2, 2> m2 = { 5, 6, 7, 8 };

    matrix<int, 2, 2> ew  = ewMul(m1, m2);   // element-wise: {5,12,21,32}
    matrix<int, 2, 2> mm  = matMul(m1, m2);  // matrix mul:   {19,22,43,50}

    outputBuffer[0] = ew[0][0];
    outputBuffer[1] = ew[0][1];
    outputBuffer[2] = ew[1][0];
    outputBuffer[3] = ew[1][1];
    outputBuffer[4] = mm[0][0];
    outputBuffer[5] = mm[0][1];
    outputBuffer[6] = mm[1][0];
    outputBuffer[7] = mm[1][1];
}

// COMPARE_COMPUTE -cpu: ew={5,12,21,32} mm={19,22,43,50}.
// CHECK: 5
// CHECK: 12
// CHECK: 21
// CHECK: 32
// CHECK: 19
// CHECK: 22
// CHECK: 43
// CHECK: 50

// int2x2 on HLSL; ewMul_0 uses native * (element-wise), matMul_0 uses mul().
//HLSL-DAG: ewMul_0
//HLSL-DAG: return a_0 * b_0
//HLSL-DAG: matMul_0
//HLSL-DAG: return mul(a_{{[0-9]+}}, b_{{[0-9]+}})

// GLSL lowers int-matrix to array-of-ivec2; ewMul_0 element-wise, matMul_0 separate.
//GLSL-DAG: ewMul_0
//GLSL-DAG: a_0[0] * b_0[0]
//GLSL-DAG: matMul_0

// Metal lowers int-matrix to array<int2,...>; ewMul_0 component *, matMul_0 separate.
//METAL-DAG: ewMul_0
//METAL-DAG: (*a_{{[0-9]+}})[int(0)] * (*b_{{[0-9]+}})[int(0)]
//METAL-DAG: matMul_0

// WGSL lowers int-matrix to array<vec2<i32>,...>; ewMul_0 component *, matMul_0 separate.
//WGSL-DAG: ewMul_0
//WGSL-DAG: a_0[i32(0)] * b_0[i32(0)]
//WGSL-DAG: matMul_0

// CUDA uses Matrix<int,2,2> with * (element-wise) and mul_0() helper.
//CUDA-DAG: ewMul_0
//CUDA-DAG: return a_0 * b_0
//CUDA-DAG: matMul_0
//CUDA-DAG: mul_0(

// CPP uses Matrix<int32_t,2,2> with * (element-wise) and mul_0() helper.
//CPP-DAG: ewMul_0
//CPP-DAG: return a_0 * b_0
//CPP-DAG: matMul_0
//CPP-DAG: mul_0(

// slangi companion: verify runtime values via printf.
void main()
{
    matrix<int, 2, 2> m1 = { 1, 2, 3, 4 };
    matrix<int, 2, 2> m2 = { 5, 6, 7, 8 };

    matrix<int, 2, 2> ew  = m1 * m2;
    matrix<int, 2, 2> mm  = mul(m1, m2);

    //ICHECK: ew=5 12 21 32
    //ICHECK: mm=19 22 43 50
    printf("ew=%d %d %d %d\n", ew[0][0], ew[0][1], ew[1][0], ew[1][1]);
    printf("mm=%d %d %d %d\n", mm[0][0], mm[0][1], mm[1][0], mm[1][1]);
}
