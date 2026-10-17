// Runtime check for the casts out of a `DescriptorHandle<T>` that shader-slang/slang#13633 keeps
// legal. A same-size `bit_cast` out of the handle must read the handle's own value, and
// `reinterpret` must round-trip it while an unrelated `bit_cast` makes `lowerBitCast` run over the
// generated casts, so the handle still reaches its buffer. A buffer handle is a `uint2` on Vulkan
// and the 16-byte native buffer on CUDA.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type -render-feature bindless -emit-spirv-directly -Xslang -DHANDLE_BITS=uint2
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -cuda -shaderobj -output-using-type -Xslang -DHANDLE_BITS=uint4

//TEST_INPUT:ubuffer(data=[10.0 20.0 30.0 40.0], stride=4):name=bindless
uniform RWStructuredBuffer<float>.Handle bindless;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<float> gOutput;

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    HANDLE_BITS bits = reinterpret<HANDLE_BITS>(bindless);
    float same = all(bit_cast<HANDLE_BITS>(bindless) == bits) ? 1.0 : 0.0;
    let roundTrip = reinterpret<RWStructuredBuffer<float>.Handle>(bits);
    gOutput[i] = roundTrip[i] + same + bit_cast<float>(0u);
}

//CHK:11.0
//CHK-NEXT:21.0
//CHK-NEXT:31.0
//CHK-NEXT:41.0
