//TEST:SIMPLE(filecheck=PASS): -entry computeMain -stage compute -target spirv -DPASS1
//TEST:SIMPLE(filecheck=WARN): -entry computeMain -stage compute -target spirv -DWARN1
//TEST:SIMPLE(filecheck=WARN): -entry computeMain -stage compute -target spirv -DWARN2

//PASS-NOT: 31104
#ifdef PASS1
[[vk_binding(0,0)]] uniform ConstantBuffer<uint> addresses;
[[vk_binding(1,0)]] uniform ConstantBuffer<uint> addresses1;
#endif

//WARN: 31104
#ifdef WARN1
[[vk_binding(0,0)]] uniform uint addresses;
[[vk_binding(1,0)]] uniform uint addresses1;
#endif
#ifdef WARN2
layout(location = 0) uniform uint addresses;
layout(location = 1) uniform uint addresses1;
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID, uint groupIndex: SV_GroupIndex)
{
    // TODO(#14247): the explicit cast is needed because printf aborts on a ConstantBuffer<uint> argument.
    printf("\nfrom gpu: %u", uint(addresses));
    printf("\nfrom gpu: %u", uint(addresses1));
}