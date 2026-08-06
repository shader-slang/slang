//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//
// Companion to tests/diagnostics/entry-point-cannot-throw.slang (issue #12330).
//
// The sanctioned shape: a callee declares `throws`, and the entry point handles the error
// itself with `do`/`catch` rather than propagating it out of the shader. This test exists so
// that the new `entry-point-cannot-throw` diagnostic cannot be satisfied by over-rejecting --
// error handling *inside* an entry point must keep working, on both the success and the
// failure path. Lane 0 takes the throwing path; lanes 1..3 take the returning path.

// The thrown error code is printed in hex, the returned lane values in decimal.
// CHECK: BAD
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

uint g(uint n) throws uint
{
    if (n < 1)
        throw 0xBADU;

    return n;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    do
    {
        outputBuffer[tid.x] = try g(tid.x);
    }
    catch (err : uint)
    {
        outputBuffer[tid.x] = err;
    }
}
