//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#operators
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Binary arithmetic between two same-length vectors is applied element-wise; the doc example: `v1 + v2 == { 6, 8, 10, 12 }`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Reworked to buffer-output so the shader compiles on every target
// (printf blocks Metal/WGSL with E36107). Function wrappers defeat
// constant-folding so the `+` and `*` operations survive in emitted
// function bodies. COMPARE_COMPUTE -cpu verifies runtime values;
// SIMPLE directives fan out emission to all text-emit targets.
// SPIRV constant-folds the inlined constant vector inputs; not checked.
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

int4 addVec(int4 a, int4 b) { return a + b; }
int4 mulVec(int4 a, int4 b) { return a * b; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int4 v1 = int4(1, 2, 3, 4);
    int4 v2 = int4(5, 6, 7, 8);
    int4 sum = addVec(v1, v2);
    int4 prod = mulVec(v1, v2);
    outputBuffer[0] = sum.x;
    outputBuffer[1] = sum.y;
    outputBuffer[2] = sum.z;
    outputBuffer[3] = sum.w;
    outputBuffer[4] = prod.x;
    outputBuffer[5] = prod.y;
    outputBuffer[6] = prod.z;
    outputBuffer[7] = prod.w;
}

// COMPARE_COMPUTE -cpu: sum={6,8,10,12} prod={5,12,21,32}.
// CHECK: 6
// CHECK: 8
// CHECK: 10
// CHECK: 12
// CHECK: 5
// CHECK: 12
// CHECK: 21
// CHECK: 32

// All text-emit targets: addVec returns (a+b), mulVec returns (a*b).
//HLSL-DAG: return a_0 + b_0
//HLSL-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}
//GLSL-DAG: return a_0 + b_0
//GLSL-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}
//METAL-DAG: return a_0 + b_0
//METAL-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}
//WGSL-DAG: return a_0 + b_0
//WGSL-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}
//CUDA-DAG: return a_0 + b_0
//CUDA-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}
//CPP-DAG: return a_0 + b_0
//CPP-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}

// slangi companion: verify runtime values via printf.
void main()
{
    int4 v1 = int4(1, 2, 3, 4);
    int4 v2 = int4(5, 6, 7, 8);
    int4 sum = addVec(v1, v2);
    int4 prod = mulVec(v1, v2);
    //ICHECK: sum=6 8 10 12
    //ICHECK: prod=5 12 21 32
    printf("sum=%d %d %d %d\n", sum.x, sum.y, sum.z, sum.w);
    printf("prod=%d %d %d %d\n", prod.x, prod.y, prod.z, prod.w);
}
