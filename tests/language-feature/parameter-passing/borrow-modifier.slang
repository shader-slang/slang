// borrow-modifier.slang
//
// `__borrow` is an internal alias for the existing `BorrowIn` parameter-passing
// mode (the same mode `__constref` maps to). This test confirms it compiles and
// runs end-to-end, and that (like `__constref`) it is passed by reference rather
// than copied into a local.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -slang -compute -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -cpu -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Thing
{
    int value;
    int bigArray[128];
};

// Check that the `__borrow` parameter is passed by pointer, not copied into a
// local variable of struct type.

// CHECK: __device__ int test{{.*}}Thing{{.*}}*{{.*}}thing{{.*}})
// CHECK-NOT: Thing{{[a-zA-Z0-9_]*}} {{[a-zA-Z0-9_]+}}
// CHECK: }

int test(__borrow Thing thing)
{
    int sum = 0;
    for (int i = 0; i < 128; i++)
    {
        sum += thing.bigArray[i];
    }
    return sum;
}

int caller(Thing thing)
{
    return test(thing);
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;

    Thing thing = {};

    thing.bigArray[0] = 100;
    thing.bigArray[99] = 1;

    // BUFFER: 101
    outputBuffer[idx] = caller(thing);
}
