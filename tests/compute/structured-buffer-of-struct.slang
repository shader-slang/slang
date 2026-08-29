//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: .visible .entry computeMain()
// NVVM: ld.global.u32
// NVVM: st.global.u32

// CHECK:      2
// CHECK-NEXT: 1
// CHECK-NEXT: 4
// CHECK-NEXT: 3
// CHECK-NEXT: 6
// CHECK-NEXT: 5
// CHECK-NEXT: 8
// CHECK-NEXT: 7

struct S
{
    int a;
    int b;
};

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=8):out,name=buf
RWStructuredBuffer<S> buf;

S test(S s)
{
    s.a = s.a ^ s.b;
    s.b = s.a ^ s.b;
    s.a = s.a ^ s.b;
    return s;
}

[numthreads(4, 1, 1)]
// void computeMain(uint i : SV_GroupIndex)
// {
//     buf[i] = test(buf[i]);
// }
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    buf[dispatchThreadID.x] = test(buf[dispatchThreadID.x]);
}
