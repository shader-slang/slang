//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#row-and-element-access-
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Matrix element access: `m[r][c]` returns the value at row r, column c. `m[r]` returns a row vector. `m[r].yx` swizzles the row.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Reworked to buffer-output so the shader compiles on every target
// (printf blocks Metal/WGSL with E36107). The row index is read from an
// input buffer to defeat constant-folding so m[r][c], m[r].w, and
// m[r].yx access patterns survive in all emitted function bodies.
// HLSL preserves int3x4 with m_0[r][c] / m_0[r].w / m_0[0].yx notation.
// GLSL/Metal/WGSL lower to an array-of-row-vectors but subscript access survives.
// CUDA/CPP use Matrix<int,3,4>.rows[r] notation.
// INTERPRET companion (main) keeps slangi-VM value coverage.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
//TEST_INPUT: set inputBuffer = ubuffer(data=[1 0 0], stride=4)
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:INTERPRET(filecheck=ICHECK):

RWStructuredBuffer<int> outputBuffer;
StructuredBuffer<int> inputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    matrix<int, 3, 4> m = {
        1,  2,  3,  4,
        5,  6,  7,  8,
        9, 10, 11, 12
    };
    int r = inputBuffer[0]; // r=1 from input
    int t1 = m[r][2];       // row 1, col 2 = 7
    int t2 = m[r].w;        // row 1, col 3 = 8
    int4 t3 = m[r+1];       // row 2 = { 9, 10, 11, 12 }
    int2 t4 = m[0].yx;      // { m[0].y=2, m[0].x=1 } = { 2, 1 }
    outputBuffer[0] = t1;
    outputBuffer[1] = t2;
    outputBuffer[2] = t3.x;
    outputBuffer[3] = t3.y;
    outputBuffer[4] = t3.z;
    outputBuffer[5] = t3.w;
    outputBuffer[6] = t4.x;
    outputBuffer[7] = t4.y;
}

// COMPARE_COMPUTE -cpu: r=1 gives t1=7, t2=8, t3={9,10,11,12}, t4={2,1}.
// CHECK: 7
// CHECK: 8
// CHECK: 9
// CHECK: 10
// CHECK: 11
// CHECK: 12
// CHECK: 2
// CHECK: 1

// HLSL emits int3x4 with m_0[r_0][int(2)] and m_0[r_0].w and m_0[int(0)].yx.
//HLSL-DAG: int3x4
//HLSL-DAG: m_0[r_{{[0-9]+}}][int(2)]
//HLSL-DAG: m_0[int(0)].yx

// GLSL/Metal/WGSL lower matrix to array-of-row-vectors; subscript access survives.
//GLSL-DAG: _S1[r_0][2]
//GLSL-DAG: _S1[r_0].w
//METAL-DAG: _S{{[0-9]+}}[r_{{[0-9]+}}][int(2)]
//METAL-DAG: _S{{[0-9]+}}[r_{{[0-9]+}}].w
//WGSL-DAG: _S{{[0-9]+}}[r_{{[0-9]+}}][i32(2)]
//WGSL-DAG: _S{{[0-9]+}}[r_{{[0-9]+}}].w

// CUDA/CPP use Matrix<int,3,4>.rows[] notation.
//CUDA-DAG: Matrix<int, 3, 4>
//CUDA-DAG: rows[_S{{[0-9]+}}].w
//CPP-DAG: Matrix<int32_t, 3, 4>
//CPP-DAG: rows[r_{{[0-9]+}}].w

// slangi companion: verify runtime values via printf.
void main()
{
    matrix<int, 3, 4> m = {
        1,  2,  3,  4,
        5,  6,  7,  8,
        9, 10, 11, 12
    };
    int t1 = m[1][2];
    int t2 = m[1].w;
    int4 t3 = m[2];
    int2 t4 = m[0].yx;
    //ICHECK: t1=7 t2=8 t3=9 10 11 12 t4=2 1
    printf("t1=%d t2=%d t3=%d %d %d %d t4=%d %d\n",
           t1, t2, t3.x, t3.y, t3.z, t3.w, t4.x, t4.y);
}
