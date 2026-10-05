//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -source-language hlsl -xslang -Gec -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -source-language hlsl -xslang -Gec -output-using-type

// Every invocation starts with the input values, even after earlier invocations mutate their
// private copies. Helper calls and inout arguments must observe the same copy within an invocation.
//TEST_INPUT:uniform(data=[5]):name=x
uniform uint x;

// Vector elements keep the supplied byte layout identical on CPU and constant-buffer targets.
//TEST_INPUT:uniform(data=[13 0 0 0 17 0 0 0]):name=values
uniform uint4 values[2];
struct Data { uint value; };
//TEST_INPUT:uniform(data=[19]):name=data
uniform Data data;

//TEST_INPUT:cbuffer(data=[7 11 0 0]):name=Settings
cbuffer Settings
{
    uint y;
    uint z;
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

void increment(inout uint value)
{
    value++;
}

void replaceValues(uint value)
{
    x = value;
    y = value + 1;
    increment(z);
    values[1].x = value + 30;
    data.value = value + 50;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint original = x + y + z + values[0].x + values[1].x + data.value;
    if (tid.x != 0)
        replaceValues(tid.x);
    output[tid.x] = original + x + y + z + values[0].x + values[1].x + data.value;
}

// CHECK: 144
// CHECK: 182
// CHECK: 186
// CHECK: 190
