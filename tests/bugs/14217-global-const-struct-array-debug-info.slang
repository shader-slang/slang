// A module-scope `static const` array of structs, or struct with a struct field, built by a
// user-written constructor compiles with debug information. With debug information the constructor
// is not known to be free of side effects, so its calls are not removed as dead code. Global value
// legalization copies each constant, with its constructor calls, into the function that uses it,
// and removes the calls it left behind at module scope, which CUDA, C++ and GLSL cannot initialize.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute -g
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type

struct S
{
    float a, b;
    __init(float x, float y)
    {
        a = x;
        b = y;
    }
}

struct Outer
{
    S s;
    float k;
}

static const S arr[2] = { S(0.5f, 1.5f), S(2.5f, 3.5f) };
static const Outer outer = { S(4.5f, 5.5f), 6.5f };

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// No global is initialized by a constructor call.
// CUDA-NOT: {{S_x24init_0\([0-9]}}
// CUDA: void computeMain(
// CPP-NOT: {{S_x24init_0\([0-9]}}
// CPP: void _computeMain(
// SPV: OpEntryPoint

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    S first = arr[tid.x];
    S second = arr[tid.x + 1];
    outputBuffer[0] = first.a;
    outputBuffer[1] = first.b;
    outputBuffer[2] = second.a;
    outputBuffer[3] = second.b;
    outputBuffer[4] = outer.s.a;
    outputBuffer[5] = outer.s.b;
    outputBuffer[6] = outer.k;
    // BUF: 0.5
    // BUF-NEXT: 1.5
    // BUF-NEXT: 2.5
    // BUF-NEXT: 3.5
    // BUF-NEXT: 4.5
    // BUF-NEXT: 5.5
    // BUF-NEXT: 6.5
}
