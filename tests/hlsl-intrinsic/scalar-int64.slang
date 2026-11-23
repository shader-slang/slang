//TEST:COMPARE_COMPUTE(targets=metal+cuda+cpu+llvm):
// No support for int64_t on D3D11 (no sm 6.0)
// No support with Dx12 with dxbc. Needs SM6.0 + dxil
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0 -render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=vk):-render-feature int64

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int64_t idx = int64_t(dispatchThreadID.x);

    int64_t ti =0;

    ti += max(2, idx);
    ti += min(idx, 1);
    ti += abs(idx - 2);
    ti += clamp(idx * 10, 11, 23);

    // Math
    ti = ti - idx;
    ti = ti + (ti % (idx + 5));
    ti = ti + (ti / (idx + 20));

    // Logical ops
    ti = ti | idx;
    ti = ti ^ (idx & (idx + 1));

    // Shift
    ti = ti + (idx >> 1);
    ti = ti + (idx << 2);

    int64_t v = (ti * 0x400010035435435ll) / 3ll + 7ll - 9ll;

    outputBuffer[uint(idx)] = int(v) ^ int(((v >> 32) & 0xffffffff));
}
