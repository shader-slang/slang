// A module-scope `static const` whose initializer calls a helper is also evaluated per thread
// when it is read by the initializer of a `static` global variable. Global value legalization
// copies the constant into that initializer, which `moveGlobalVarInitializationToEntryPoints`
// later moves into the entry point, so no call is left in a global initializer, which CUDA, C++
// and GLSL reject.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none -g
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type

[ForceInline]
int ceilDivNotReadNone(int a, int b)
{
    return (a + b - 1) / b;
}

struct S
{
    int a, b;
    __init(int x, int y)
    {
        a = x;
        b = y;
    }
}

struct Holder
{
    static const int arr[2] = { ceilDivNotReadNone(10, 4), ceilDivNotReadNone(7, 2) };
    static const S structs[2] = { S(5, 6), S(7, 8) };
}

static int fromArray = Holder.arr[1];
static S fromStructs = Holder.structs[1];

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// No global constant is initialized by a call.
// CUDA-NOT: static const {{.*}} = {{[A-Za-z0-9_]+}}_0(
// CUDA: void computeMain(
// CPP-NOT: static const {{.*}} = {{[A-Za-z0-9_]+}}_0(
// CPP: void _computeMain(
// GLSL-NOT: const {{.*}} = {{[A-Za-z0-9_]+}}_0(
// GLSL: void main(

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = fromArray;
    outputBuffer[1] = Holder.arr[tid.x];
    outputBuffer[2] = fromStructs.a;
    outputBuffer[3] = Holder.structs[tid.x].b;
    // BUF: type: int32_t
    // BUF-NEXT: 4
    // BUF-NEXT: 3
    // BUF-NEXT: 7
    // BUF-NEXT: 6
}
