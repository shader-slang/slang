// A module-scope `static const` built by a `[ForceInline]` constructor whose body
// works through a local variable must not have that body inlined at the global
// scope: the `var` and its stores would be emitted as statements at file scope.
// (shader-slang/slang#14133)

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type

struct S
{
    float a, b;
    [ForceInline]
    __init(float x, float y)
    {
        a = x;
        b = y;
    }
}

static const S kValue = S(1.5f, 2.5f);

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// The constructor body must be inlined inside the function, not at file scope.
// CUDA-NOT: a_0 = 1.5
// CUDA: void computeMain(
// CUDA: a_0 = 1.5
// CPP-NOT: a_0 = 1.5
// CPP: void _computeMain(
// CPP: a_0 = 1.5
// HLSL-NOT: a_0 = 1.5
// HLSL: void computeMain(
// HLSL: a_0 = 1.5

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    S s = kValue;
    outputBuffer[0] = s.a;
    outputBuffer[1] = s.b;
    outputBuffer[2] = kValue.a + kValue.b;
    outputBuffer[3] = 1.0f;
    // BUF: 1.5
    // BUF-NEXT: 2.5
    // BUF-NEXT: 4.0
    // BUF-NEXT: 1.0
}
