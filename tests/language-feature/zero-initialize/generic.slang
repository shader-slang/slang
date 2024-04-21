//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -xslang -zero-initialize
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -entry computeMain -emit-spirv-directly -allow-glsl -xslang -zero-initialize
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -entry computeMain -allow-glsl -xslang -zero-initialize
//TEST(smoke,compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-dx12 -use-dxil -compute -entry computeMain -allow-glsl -profile sm_6_2 -xslang -zero-initialize -xslang -DDX12

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

__generic<T : __BuiltinArithmeticType>
bool getValue()
{
    T genericOut;
    return genericOut == T(0);
}
bool getValueB()
{
    bool genericOut;
    return genericOut == bool(0);
}

__generic<T : __BuiltinArithmeticType, let N : int>
bool getValueV()
{
    typealias gvec = vector<T, N>;

    gvec genericOut;
    return genericOut == gvec(T(0));
}
__generic<let N : int>
bool getValueVB()
{
    typealias gvec = vector<bool, N>;

    gvec genericOut;
    return genericOut == gvec(bool(0));
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
// BUF: 1
    outputBuffer[0] = true
    && getValue<int>()
#ifndef DX12
    && getValue<uint8_t>()
    && getValue<int8_t>()
#endif
    && getValue<uint16_t>()
    && getValue<int16_t>()
    && getValue<uint32_t>()
    && getValue<int32_t>()
    && getValue<uint64_t>()
    && getValue<int64_t>()
    && getValue<half>()
    && getValue<float>()
    && getValue<double>()
    && getValueB()

    && getValueV<int, 2>()
#ifndef DX12
    && getValueV<uint8_t, 2>()
    && getValueV<int8_t, 2>()
#endif
    && getValueV<uint16_t, 2>()
    && getValueV<int16_t, 2>()
    && getValueV<uint32_t, 2>()
    && getValueV<int32_t, 2>()
    && getValueV<uint64_t, 2>()
    && getValueV<int64_t, 2>()
    && getValueV<half, 2>()
    && getValueV<float, 2>()
    && getValueV<double, 2>()
    && getValueVB<2>()

    && getValueV<int, 3>()
#ifndef DX12
    && getValueV<uint8_t, 3>()
    && getValueV<int8_t, 3>()
#endif
    && getValueV<uint16_t, 3>()
    && getValueV<int16_t, 3>()
    && getValueV<uint32_t, 3>()
    && getValueV<int32_t, 3>()
    && getValueV<uint64_t, 3>()
    && getValueV<int64_t, 3>()
    && getValueV<half, 3>()
    && getValueV<float, 3>()
    && getValueV<double, 3>()
    && getValueVB<3>()

    && getValueV<int, 4>()
#ifndef DX12
    && getValueV<uint8_t, 4>()
    && getValueV<int8_t, 4>()
#endif
    && getValueV<uint16_t, 4>()
    && getValueV<int16_t, 4>()
    && getValueV<uint32_t, 4>()
    && getValueV<int32_t, 4>()
    && getValueV<uint64_t, 4>()
    && getValueV<int64_t, 4>()
    && getValueV<half, 4>()
    && getValueV<float, 4>()
    && getValueV<double, 4>()
    && getValueVB<4>()
    ;
}
