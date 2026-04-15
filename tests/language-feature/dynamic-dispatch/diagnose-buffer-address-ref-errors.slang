// Negative tests for data=[bufferName] error paths.
// Verifies that invalid uses produce clear error messages.
//
// Test: forward reference to undeclared buffer name.
// render-test should report "buffer ... not found" and fail.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck=CHECK):-cpu -compute -shaderobj -output-using-type

interface IFoo { int get(); }
struct FooImpl : IFoo { int v; int get() { return v; } }

//TEST_INPUT: type_conformance FooImpl:IFoo = 1

// Reference a buffer that doesn't exist — should produce an error.
//TEST_INPUT: set badRef = ubuffer(data=[0 0 1 0 doesNotExist 0], stride=4)
uniform IFoo* badRef;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// CHECK: error
// CHECK: not found

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
