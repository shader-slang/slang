//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> output;
struct Counters { float value; uint count; };
struct Record { Counters counters[2]; };
//TEST_INPUT: set records = ubuffer(data=[0 0 0 0], stride=16)
RWStructuredBuffer<Record> records;
groupshared half h;
groupshared half2 h2;
groupshared float f;
groupshared double d;
[noinline] void addFloat(Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> value)
{
    __atomic_add(*value, 1.0f);
}
[numthreads(32,1,1)]
void computeMain(uint index : SV_GroupIndex)
{
    if (index == 0) { h = half(0); h2 = half2(0); f = 0; d = 0; }
    GroupMemoryBarrierWithGroupSync();
    __atomic_add(h, half(1));
    __atomic_add(h2, half2(1,2));
    addFloat(__getAddress(f));
    __atomic_add(d, 1.0);
    __atomic_add(records[0].counters[1].value, 1.0f);
    __atomic_add(records[0].counters[1].count, 1u);
    AllMemoryBarrierWithGroupSync();
    if (index == 0)
    {
        output[0] = float(h);
        output[1] = float(h2.x);
        output[2] = float(h2.y);
        output[3] = f;
        output[4] = float(d);
        output[5] = 123;
        output[6] = records[0].counters[1].value;
        output[7] = float(records[0].counters[1].count);
    }
}
// CHECK: 32
// CHECK-NEXT: 32
// CHECK-NEXT: 64
// CHECK-NEXT: 32
// CHECK-NEXT: 32
// CHECK-NEXT: 123
// CHECK-NEXT: 32
// CHECK-NEXT: 32
