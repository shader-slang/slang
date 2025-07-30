//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -emit-spirv-directly
// CHECK: error 31225
// CHECK-NOT: error 31225
// CHECK-NOT: error 31225

// Test cases for static const variables without initializers

// This should cause an error - static const without initializer
static const int globalVar;

// This should NOT cause an error - extern static const
extern static const int externVar;

interface ITest
{
    // This should NOT cause an error - interface member
    static const int interfaceVar;
}

// This should NOT cause an error - has initializer
static const int initializedVar = 42;

// This should NOT cause an error - not static
const int nonStaticVar;

// This should NOT cause an error - not const
static int nonConstVar;

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Empty compute shader for testing
}