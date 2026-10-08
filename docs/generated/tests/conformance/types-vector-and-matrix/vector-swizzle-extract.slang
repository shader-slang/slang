//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#element-access
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Swizzle extract yields a smaller vector; elements may repeat (e.g. `.xww`).
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Doc example: `v.xww` returns { v.x, v.w, v.w }. Repeats are allowed
// for extract. Adds buffer-output emission coverage so the claim is
// verified on all text-emit targets. Function wrappers defeat constant-
// folding so the swizzle patterns survive in emitted function bodies.
// HLSL/GLSL/Metal/WGSL emit `return v_N.xy` and `return v_N.xww`
// directly. CUDA/CPP expand swizzles to component-by-component
// struct constructors, showing the repeat as two `v_N.w` references.
// INTERPRET companion (main) keeps slangi-VM value coverage.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:INTERPRET(filecheck=ICHECK):

RWStructuredBuffer<int> outputBuffer;

int2 getXY(int4 v) { return v.xy; }
int3 getXWW(int4 v) { return v.xww; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int4 v = int4(1, 2, 3, 4);
    int2 xy = getXY(v);
    int3 xww = getXWW(v);
    outputBuffer[0] = xy.x;
    outputBuffer[1] = xy.y;
    outputBuffer[2] = xww.x;
    outputBuffer[3] = xww.y;
    outputBuffer[4] = xww.z;
}

// CHECK: 1
// CHECK: 2
// CHECK: 1
// CHECK: 4
// CHECK: 4

// All targets preserve swizzle patterns in function bodies.
//HLSL-DAG: return v_0.xy
//HLSL-DAG: return v_{{[0-9]+}}.xww
//GLSL-DAG: return v_0.xy
//GLSL-DAG: return v_{{[0-9]+}}.xww
//METAL-DAG: return v_0.xy
//METAL-DAG: return v_{{[0-9]+}}.xww
//WGSL-DAG: return v_0.xy
//WGSL-DAG: return v_{{[0-9]+}}.xww

// CUDA/CPP expand swizzle to component struct; repeat .w appears twice.
//CUDA-DAG: v_{{[0-9]+}}.x, v_{{[0-9]+}}.y
//CUDA-DAG: v_{{[0-9]+}}.x, v_{{[0-9]+}}.w, v_{{[0-9]+}}.w
//CPP-DAG: v_{{[0-9]+}}.x, v_{{[0-9]+}}.y
//CPP-DAG: v_{{[0-9]+}}.x, v_{{[0-9]+}}.w, v_{{[0-9]+}}.w

// slangi companion: verify runtime values via printf.
void main()
{
    int4 v = int4(1, 2, 3, 4);
    int2 xy = v.xy;
    int3 xww = v.xww;
    //ICHECK: xy=1,2 xww=1,4,4
    printf("xy=%d,%d xww=%d,%d,%d\n", xy.x, xy.y, xww.x, xww.y, xww.z);
}
