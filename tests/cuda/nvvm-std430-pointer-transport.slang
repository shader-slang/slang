//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -
// PTX: result code = 0
// PTX: .entry computeMain

// These compile checks preserve explicit Std430/Scalar/C Device pointer transport.
// Allocated-address qualification is recorded in issue-nvvm-backend/RESULTS.md:
// initialized interior bases, signed indices -1/0/1/2, strides64/48/40 and full guards.
// No record is dereferenced and no test address is formed from a null pointer.
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

// Five live ABI inputs: three independently bound pointer bases, signed element index, output.
// The host launches one thread and keeps every shifted address inside its allocation.
[CUDAKernel]
void computeMain(
    uniform LayoutPtr<C, Std430DataLayout> d,
    uniform Ptr<C, Access::ReadWrite, AddressSpace::Device, ScalarDataLayout> s,
    uniform LayoutPtr<C, CDataLayout> c,
    uniform int index,
    uniform Ptr<uint64_t> output)
{
    output[0] = uint64_t(d);
    output[1] = uint64_t(d + index);
    output[2] = output[1] - output[0];
    output[3] = uint64_t(s);
    output[4] = uint64_t(s + index);
    output[5] = output[4] - output[3];
    output[6] = uint64_t(c);
    output[7] = uint64_t(c + index);
    output[8] = output[7] - output[6];
    output[9] = uint64_t(int64_t(index));
    output[10] = 0xc001c0de;
    // output[11] is an independently initialized, untouched host guard.
}
