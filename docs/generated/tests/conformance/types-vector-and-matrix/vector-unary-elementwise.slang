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

// Reworked to buffer-output so the shader compiles on every target
// (printf blocks Metal/WGSL with E36107). The neg() wrapper defeats
// constant-folding so the negation survives in the emitted function body.
// COMPARE_COMPUTE -cpu verifies the runtime values; SIMPLE directives
// fan out emission to all text-emit targets including Metal and WGSL.
// WGSL lowers integer vector negation to (vec4<i32>(0) - v_0) since it
// lacks a direct unary-minus-on-int-vectors intrinsic.
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

int4 neg(int4 v) { return -v; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int4 v = neg(int4(1, 2, 3, 4));
    outputBuffer[0] = v.x;
    outputBuffer[1] = v.y;
    outputBuffer[2] = v.z;
    outputBuffer[3] = v.w;
}

// CHECK: -1
// CHECK: -2
// CHECK: -3
// CHECK: -4

//HLSL: return - v_0
//GLSL: return - v_0
//METAL: return - v_0
//WGSL: vec4<i32>(0) - v_0
//CUDA: return - v_0
//CPP: return - v_0

// slangi companion: verify runtime values via printf.
void main()
{
    int4 v = neg(int4(1, 2, 3, 4));
    //ICHECK: -1 -2 -3 -4
    printf("%d %d %d %d\n", v.x, v.y, v.z, v.w);
}
