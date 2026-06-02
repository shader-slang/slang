//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#operators
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Unary arithmetic operator on a vector applies element-wise: `-v` negates every component.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

// Unary `-` on a vector lowers to per-target negation on the vector type.
// The neg() wrapper prevents constant-folding so the operation survives emit.
// HLSL/GLSL preserve the `return - v_0` inside the emitted function body.
// CUDA emits `__device__ int4 neg_0(int4 v_0) { return - v_0; }`.
// CPP emits `Vector<int32_t, 4> neg_0(...) { return - v_0; }`.
// SPIRV constant-folds the negation (no observable opcode); not checked.
// Metal/WGSL: printf unsupported in compute; untested for emission.

int4 neg(int4 v) { return -v; }

void main()
{
    int4 v = neg(int4(1, 2, 3, 4));
    //CHECK: -1 -2 -3 -4
    //HLSL: return - v_0
    //GLSL: return - v_0
    //CUDA: return - v_0
    //CPP: return - v_0
    printf("%d %d %d %d\n", v.x, v.y, v.z, v.w);
}
