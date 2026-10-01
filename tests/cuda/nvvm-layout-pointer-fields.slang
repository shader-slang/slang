//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -
// PTX: result code = 0
// PTX: .entry computeMain

// RESULTS.md records initialized host bindings and the independent full-allocation byte oracle.
struct A
{
    uint64_t f0;
    uint32_t f1;
};
struct B
{
    float3 f;
};
struct C
{
    A a;
    uint32_t test1;
    B b;
    uint32_t test2;
    bool c;
    bool d;
    bool e;
};
cbuffer Globals
{
    LayoutPtr<C, Std430DataLayout> d;
    LayoutPtr<C, ScalarDataLayout> s;
    LayoutPtr<C, CDataLayout> c;
}

[noinline]
void accessRecord<L : IBufferDataLayout>(
    LayoutPtr<C, L> p,
    int index,
    Ptr<uint64_t> output,
    uint firstOutput)
{
    // Observe all original leaves before any write. Float observations retain raw bits.
    output[firstOutput + 0] = p[0].a.f0;
    output[firstOutput + 1] = uint64_t(p[0].a.f1);
    output[firstOutput + 2] = uint64_t(p[0].test1);
    float3 vectorValue = p[0].b.f;
    output[firstOutput + 3] = uint64_t(bit_cast<uint>(vectorValue.x));
    output[firstOutput + 4] = uint64_t(bit_cast<uint>(vectorValue.y));
    output[firstOutput + 5] = uint64_t(bit_cast<uint>(vectorValue.z));
    output[firstOutput + 6] = uint64_t(p[0].test2);
    output[firstOutput + 7] = p[0].c ? 1 : 0;
    output[firstOutput + 8] = p[0].d ? 1 : 0;
    output[firstOutput + 9] = p[0].e ? 1 : 0;

    // Disjoint leaf sets make unchanged neighbors observable, especially packed C Bool fields.
    if ((index & 1) == 0)
    {
        p[0].a.f0 = uint64_t(0x12345678) << 32 | uint64_t(0x9abcdef0);
        p[0].test1 = 0x76543210;
        p[0].b.f = float3(1.25, -2.5, 3.75);
        p[0].test2 = 0x89abcdef;
        p[0].d = true;
    }
    else
    {
        p[0].a.f1 = 0xfedcba98;
        p[0].b.f.y = -6.5;
        p[0].c = false;
        p[0].e = true;
    }
}

[noinline]
void forwardRecord<L : IBufferDataLayout>(
    LayoutPtr<C, L> p,
    int index,
    Ptr<uint64_t> output,
    uint firstOutput)
{
    accessRecord<L>(p + index, index, output, firstOutput);
}

[CUDAKernel]
void computeMain(uniform int index, uniform Ptr<uint64_t> output)
{
    forwardRecord<Std430DataLayout>(d, index, output, 0);
    forwardRecord<ScalarDataLayout>(s, index, output, 10);
    forwardRecord<CDataLayout>(c, index, output, 20);
    output[30] = uint64_t(int64_t(index));
    output[31] = 0xc001c0de;
    // output[32] and surrounding words remain host guards.
}
