//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -
// PTX: result code = 0
// PTX: .entry computeMain

// Allocated-address runtime qualification and retained-call inspection are recorded in
// issue-nvvm-backend/RESULTS.md. Std430/Scalar/C strides are independently64/48/40.
// Both offset steps remain inside separately initialized allocations for indices -1/0/1/2.
// Record declarations match tests/language-feature/pointer/data-layouts.slang.
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

// Only numeric values cross the result boundary; record pointers remain address-only.
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

// Five live ABI inputs with initialized interior bases supplied by the host.
[CUDAKernel]
void computeMain(
    uniform LayoutPtr<C, Std430DataLayout> d,
    uniform Ptr<C, Access::ReadWrite, AddressSpace::Device, ScalarDataLayout> s,
    uniform LayoutPtr<C, CDataLayout> c,
    uniform int index,
    uniform Ptr<uint64_t> output)
{
    uint64_t2 addressesD = forwardOffset<Std430DataLayout>(d + index, index);
    uint64_t2 addressesS = forwardOffset<ScalarDataLayout>(s + index, index);
    uint64_t2 addressesC = forwardOffset<CDataLayout>(c + index, index);
    // The moved-address word checks the caller offset. The delta independently checks
    // the nested helper offset, which starts from that already-shifted pointer.
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
