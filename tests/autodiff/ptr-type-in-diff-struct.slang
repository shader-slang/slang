//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Test : IDifferentiable 
{
    no_diff Ptr<float> value;

    [Differentiable]
    float foo() 
    {
        if (value != nullptr) 
        {
            return *value;
        }
        return 1.0;
    }
}

[Differentiable]
float foo(Test t) {
    return t.foo();
}

[shader("compute"), numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    DifferentialPair<Test> dp = DifferentialPair<Test>( {}, {} );
    let dpOut = fwd_diff(foo)(dp);
    outputBuffer[tid.x] = dpOut.p; // BUFFER: 1.0
    outputBuffer[tid.x + 1] = dpOut.d; // BUFFER: 0.0
}