//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain

//CHECK: OpExecutionMode

struct Data {
    int val;
};

int fetch(__constref ParameterBlock<Data> thisData)
{
    return thisData.val;
}

ParameterBlock<Data> data = { 42 };

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    int res = fetch(data);
}