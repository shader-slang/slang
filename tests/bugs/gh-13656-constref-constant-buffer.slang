// Passing a `ConstantBuffer<P>` or `ParameterBlock<P>` argument to a
// `__constref P` parameter used to crash the compiler during type-flow
// specialization (https://github.com/shader-slang/slang/issues/13656).

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type

// SPIRV: OpEntryPoint GLCompute %computeMain "main" {{.*}}%gCB %gPB
// SPIRV-DAG: %gCB = OpVariable %{{.*}} Uniform
// SPIRV-DAG: %gPB = OpVariable %{{.*}} Uniform

// HLSL-DAG: cbuffer gCB
// HLSL-DAG: cbuffer gPB
// HLSL: void computeMain()

interface ISum
{
    int sum();
}

struct Params : ISum
{
    int a;
    int b;

    int sum() { return a + b; }
};

//TEST_INPUT:set gCB = new Params{ 3, 4 }
ConstantBuffer<Params> gCB;

//TEST_INPUT:set gPB = new Params{ 10, 20 }
ParameterBlock<Params> gPB;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int sum(__constref Params p)
{
    return p.a + p.b;
}

[noinline]
int product(__constref Params p)
{
    return p.a * p.b;
}

int genericSum<T : ISum>(__constref T p)
{
    return p.sum();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // BUF: 7
    outputBuffer[0] = sum(gCB);
    // BUF-NEXT: 30
    outputBuffer[1] = sum(gPB);
    // BUF-NEXT: 212
    outputBuffer[2] = product(gCB) + product(gPB);
    // BUF-NEXT: 37
    outputBuffer[3] = genericSum<Params>(gCB) + genericSum<Params>(gPB);
}
