//TEST:SIMPLE(filecheck=CHECK): -target metal

struct S
{
    // CHECK-DAG: ([[# @LINE+1]]): error 56100:
    ConstantBuffer<int> cb;
}

ParameterBlock<S> s;

// CHECK-DAG: ([[# @LINE+1]]): error 56100:
ParameterBlock<ConstantBuffer<int>> s2;

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void kernelMain()
{
    outputBuffer[0] = s.cb + s2;
}