//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -
// PTX: result code = 0
// PTX: .entry computeMain

// The host supplies initialized interior pointers through Globals. No pointee is dereferenced.
// Allocated-address qualification and the parameter-group ABI are recorded in RESULTS.md.
// These records and address observations match nvvm-layout-pointer-helpers.slang.
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
    Ptr<C, Access::ReadWrite, AddressSpace::Device, ScalarDataLayout> s;
    LayoutPtr<C, CDataLayout> c;
}

[noinline]
uint64_t observeOffset<L : IBufferDataLayout>(LayoutPtr<C, L> p, int index)
{
    return uint64_t(p + index);
}

[noinline]
uint64_t2 forwardOffset<L : IBufferDataLayout>(LayoutPtr<C, L> p, int index)
{
    uint64_t entered = uint64_t(p);
    return uint64_t2(entered, observeOffset<L>(p, index));
}

// Index and output remain direct kernel arguments; only the three record pointers move to Globals.
[CUDAKernel]
void computeMain(uniform int index, uniform Ptr<uint64_t> output)
{
    uint64_t2 addressesD = forwardOffset<Std430DataLayout>(d + index, index);
    uint64_t2 addressesS = forwardOffset<ScalarDataLayout>(s + index, index);
    uint64_t2 addressesC = forwardOffset<CDataLayout>(c + index, index);
    output[0] = uint64_t(d);
    output[1] = addressesD.x;
    output[2] = addressesD.y - addressesD.x;
    output[3] = uint64_t(s);
    output[4] = addressesS.x;
    output[5] = addressesS.y - addressesS.x;
    output[6] = uint64_t(c);
    output[7] = addressesC.x;
    output[8] = addressesC.y - addressesC.x;
    output[9] = uint64_t(int64_t(index));
    output[10] = 0xc001c0de;
    // output[11] and all surrounding host words remain untouched guards.
}
