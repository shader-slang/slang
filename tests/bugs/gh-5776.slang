//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile sm_6_0 -use-dxil -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cuda -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -wgpu -output-using-type
//
struct WrappedBuffer<T : IDifferentiable>
{
    StructuredBuffer<T> buffer;
    int shape;

    T get(int idx) { return buffer[idx]; }
}

struct GradInBuffer<T : IDifferentiable>
{
    WrappedBuffer<T.Differential> grad_in;
}

struct CallData
{
    WrappedBuffer<float[1]> grad_in;
}


//TEST_INPUT: set call_data.grad_in.buffer = ubuffer(data=[1.0 2.0], stride=4);
ParameterBlock<CallData> call_data;


//TEST_INPUT:ubuffer(data=[0.0 0.0], stride=4):out, name outputBuffer
RWStructuredBuffer<float> outputBuffer;


[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{

    float[1] data1 = call_data.grad_in.buffer[0];
    float[1] data2 = call_data.grad_in.get(1);
    outputBuffer[0] = data1[0];
    outputBuffer[1] = data2[0];
}
