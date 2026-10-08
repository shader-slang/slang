//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#element-access
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Vector named-component access: `.x` is index 0, `.y` is 1, `.z` is 2, `.w` is 3.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Adds buffer-output emission coverage for the .x/.w named-component
// access claim. Helper functions getX()/getW() defeat constant-folding
// so the .x and .w access patterns survive in all emitted function bodies.
// All text-emit targets emit `return v_0.x` and `return v_1.w` in the
// helper bodies. INTERPRET companion (main) keeps slangi-VM value coverage.

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

int getX(int4 v) { return v.x; }
int getW(int4 v) { return v.w; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int4 v = int4(7, 8, 9, 10);
    outputBuffer[0] = getX(v);   // .x = index 0 = 7
    outputBuffer[1] = v.y;       // .y = index 1 = 8
    outputBuffer[2] = v.z;       // .z = index 2 = 9
    outputBuffer[3] = getW(v);   // .w = index 3 = 10
}

// CHECK: 7
// CHECK: 8
// CHECK: 9
// CHECK: 10

// All targets preserve .x and .w named-component access in function bodies.
//HLSL-DAG: return v_0.x
//HLSL-DAG: return v_{{[0-9]+}}.w
//GLSL-DAG: return v_0.x
//GLSL-DAG: return v_{{[0-9]+}}.w
//METAL-DAG: return v_0.x
//METAL-DAG: return v_{{[0-9]+}}.w
//WGSL-DAG: return v_0.x
//WGSL-DAG: return v_{{[0-9]+}}.w
//CUDA-DAG: return v_0.x
//CUDA-DAG: return v_{{[0-9]+}}.w
//CPP-DAG: return v_0.x
//CPP-DAG: return v_{{[0-9]+}}.w

// slangi companion: verify runtime values via printf.
void main()
{
    int4 v = int4(7, 8, 9, 10);
    //ICHECK: x=7 y=8 z=9 w=10
    printf("x=%d y=%d z=%d w=%d\n", v.x, v.y, v.z, v.w);
}
