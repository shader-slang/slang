// TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -emit-spirv-directly

// Test cases for static const variables without initializers producing an error

// CHECK: ([[# @LINE+1]]): error 31225
static const int globalVar;

// CHECK-NOT: error 31225

// This should NOT cause an error - extern static const
extern static const int externVar;

interface ITest
{
    // This should NOT cause an error - interface member
    static const int interfaceVar;
}

// This should NOT cause an error - has initializer
static const int initializedVar = 42;
const int nonStaticVar;
static int nonConstVar;

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
