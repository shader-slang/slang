//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target glsl

struct Data {
    RWStructuredBuffer<int> input;
    RWStructuredBuffer<int> output;
    int val;  
};

int fetchInner(Data thisData, int index)
{
    return thisData.input.Load(index) + thisData.val;
}

int fetch(__constref ParameterBlock<Data> thisData, int index)
{
    return fetchInner(thisData, index);
}

ParameterBlock<Data> data;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let res = fetch(data, data.input[0]);

    data.output[0] = res;
}

