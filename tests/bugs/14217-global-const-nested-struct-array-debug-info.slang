// A module-scope `static const` whose constructor calls sit more than one level below the
// constant, or appear more than once in it, compiles with debug information. When global value
// legalization removes such a constant, it queues each operand that was inlined with it and
// removes the operands that are left without uses, so no constructor call is left in a global
// initializer, which CUDA, C++ and GLSL reject.
//
// `nested` is an array of arrays and `outers` an array of structs, so each call is two levels
// below the constant. `twice` uses the call that initializes `shared` for both of its elements,
// so that call is queued twice.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none -g
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

static const S nested[2][2] = { { S(1.0f, 2.0f), S(3.0f, 4.0f) }, { S(5.0f, 6.0f), S(7.0f, 8.0f) } };
static const Outer outers[2] = { { S(0.5f, 1.5f), 1.0f }, { S(2.5f, 3.5f), 2.0f } };
static const S shared = S(9.0f, 10.0f);
static const S twice[2] = { shared, shared };

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// No global constant is initialized by a constructor call.
// CUDA-NOT: static const {{.*}} = S_x24init_0(
// CUDA: void computeMain(
// CPP-NOT: static const {{.*}} = S_x24init_0(
// CPP: void _computeMain(
// GLSL-NOT: const {{.*}} = S_x24init_0(
// GLSL: void main(

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    outputBuffer[0] = nested[i + 1][i].b;
    outputBuffer[1] = outers[i + 1].s.a;
    outputBuffer[2] = twice[i].a;
    outputBuffer[3] = twice[i + 1].b;
    // BUF: 6
    // BUF-NEXT: 2.5
    // BUF-NEXT: 9
    // BUF-NEXT: 10
}
