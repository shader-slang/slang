//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -llvm -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -emit-spirv-directly -compute

struct A
{
    uint64_t f0;
    uint32_t f1;
    // Intentional trailing padding here.
};

struct B
{
    float3 f;
};

struct C
{
    A a;            // D:  0-16 | S:  0-12 | C:  0-16
    uint32_t test1; // D: 16-20 | S: 12-16 | C: 16-20
    B b;            // D: 32-48 | S: 16-28 | C: 20-32
    uint32_t test2; // D: 48-52 | S: 28-32 | C: 32-36
    bool c;         // D: 52-56 | S: 32-36 | C: 36-37
    bool d;         // D: 56-60 | S: 36-40 | C: 37-38
    bool e;         // D: 60-64 | S: 40-44 | C: 38-39
    // Trailing padding for alignment:
    // D: 64-64 | S: 44-48 | C: 39-40
};

cbuffer Globals
{
    LayoutPtr<C, Std430DataLayout> d;
    Ptr<C, Access::ReadWrite, AddressSpace::Device, ScalarDataLayout> s;
    LayoutPtr<C, CDataLayout> c;
}

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // This statement should return the stride of the struct in bytes, which
    // is different between the compared layouts with struct C.
    outputBuffer[0] = int(reinterpret<uint64_t>(d+1)-reinterpret<uint64_t>(d)) == 64;
    outputBuffer[1] = int(reinterpret<uint64_t>(s+1)-reinterpret<uint64_t>(s)) == 48;
    outputBuffer[2] = int(reinterpret<uint64_t>(c+1)-reinterpret<uint64_t>(c)) == 40;
}
