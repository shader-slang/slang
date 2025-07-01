//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK): -target cpp

struct MyType
{
    RWStructuredBuffer<int> output;
}

ParameterBlock<MyType> input;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint id : SV_DispatchThreadID)
{
    // make sure we don't emit &(input->output)[idx] where
    // the address operator will take higher precedence than the indexing operator.
    // CHECK: &({{.*}}->output_0[{{.*}}])
    input.output[id] = id * 2;
}

