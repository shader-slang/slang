//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#element-access
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Vector subscript: `v[i]` returns the i-th element, 0-indexed.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Adds buffer-output emission coverage for the v[i] subscript claim.
// A getElem(v, i) helper with dynamic index `i` defeats constant-folding
// so the subscript operation survives in all emitted function bodies.
// HLSL/GLSL/Metal/WGSL emit `return v_0[i_0]`; CUDA/CPP emit
// `_slang_vector_get_element(v_0, i_0)` for dynamic-index subscript.
// INTERPRET companion (main) keeps slangi-VM value coverage.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:INTERPRET(filecheck=ICHECK):

RWStructuredBuffer<int> outputBuffer;

int getElem(int4 v, int i) { return v[i]; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int4 v = int4(10, 20, 30, 40);
    outputBuffer[0] = getElem(v, 0);
    outputBuffer[1] = getElem(v, 1);
    outputBuffer[2] = getElem(v, 2);
    outputBuffer[3] = getElem(v, 3);
}

// CHECK: 10
// CHECK: 20
// CHECK: 30
// CHECK: 40

// Direct subscript v_0[i_0] in the helper body (all text-emit targets).
//HLSL: return v_0[i_0]
//GLSL: return v_0[i_0]
//METAL: return v_0[i_0]
//WGSL: return v_0[i_0]
// CUDA and CPP use _slang_vector_get_element for dynamic-index subscript.
//CUDA: _slang_vector_get_element(v_0, i_0)
//CPP: _slang_vector_get_element(v_0, i_0)

// slangi companion: verify runtime values via printf.
void main()
{
    int4 v = int4(10, 20, 30, 40);
    //ICHECK: 10 20 30 40
    printf("%d %d %d %d\n", v[0], v[1], v[2], v[3]);
}
