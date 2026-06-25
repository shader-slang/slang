//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#operators
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Binary arithmetic between a vector and a scalar broadcasts the scalar across all vector elements; both `v - 1` and `4 - v` are documented.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

// Scalar-broadcast: the function wrappers prevent constant-folding so both
// forms (`v - 1` and `4 - v`) survive in emitted function bodies.
// HLSL emits `return v_0 - int(1)` and `return int(4) - v_1`.
// GLSL emits `return v_0 - 1` and `return 4 - v_1`.
// CUDA emits `return v_0 - make_int4(int(1))` etc.
// CPP emits `return v_0 - (Vector<int32_t, 4>) int(1)`.
// SPIRV constant-folds entirely; not checked.
// Metal/WGSL: printf unsupported in compute; untested for emission.

int4 subOne(int4 v) { return v - 1; }
int4 fourMinus(int4 v) { return 4 - v; }

void main()
{
    int4 a = subOne(int4(1, 2, 3, 4));    // { 0, 1, 2, 3 }
    int4 b = fourMinus(int4(1, 2, 3, 4)); // { 3, 2, 1, 0 }
    //CHECK: a=0 1 2 3
    //CHECK: b=3 2 1 0
    //HLSL-DAG: return v_0 - int(
    //HLSL-DAG: return int(4) - v_{{[0-9]+}}
    //GLSL-DAG: return v_0 - 1
    //GLSL-DAG: return 4 - v_{{[0-9]+}}
    //CUDA-DAG: return v_0 - make_int4
    //CUDA-DAG: return make_int4 (int(4)) - v_{{[0-9]+}}
    //CPP-DAG: return v_0 -
    //CPP-DAG: return (Vector<int32_t, 4> )int(4) - v_{{[0-9]+}}
    printf("a=%d %d %d %d\n", a.x, a.y, a.z, a.w);
    printf("b=%d %d %d %d\n", b.x, b.y, b.z, b.w);
}
