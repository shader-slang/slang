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

//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

// Element-wise `+` and `*` on same-length int4 vectors. Function wrappers
// defeat constant-folding so the operations survive in emitted bodies.
// All text-emit targets emit `return a_0 + b_0` and `return a_1 * b_1`
// inside helper functions. SPIRV constant-folds entirely; not checked.
// Metal and WGSL reject printf in compute; untested for emission.

int4 addVec(int4 a, int4 b) { return a + b; }
int4 mulVec(int4 a, int4 b) { return a * b; }

void main()
{
    int4 v1 = int4(1, 2, 3, 4);
    int4 v2 = int4(5, 6, 7, 8);
    int4 sum = addVec(v1, v2);
    int4 prod = mulVec(v1, v2);
    //CHECK: sum=6 8 10 12
    //CHECK: prod=5 12 21 32
    //HLSL-DAG: return a_0 + b_0
    //HLSL-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}
    //GLSL-DAG: return a_0 + b_0
    //GLSL-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}
    //CUDA-DAG: return a_0 + b_0
    //CUDA-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}
    //CPP-DAG: return a_0 + b_0
    //CPP-DAG: return a_{{[0-9]+}} * b_{{[0-9]+}}
    printf("sum=%d %d %d %d\n", sum.x, sum.y, sum.z, sum.w);
    printf("prod=%d %d %d %d\n", prod.x, prod.y, prod.z, prod.w);
}
