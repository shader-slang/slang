//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -
// PTX: result code = 0
// PTX: .entry computeMain

// These compile checks preserve the explicit Scalar/C pointer transport boundary.
// Allocated-address runtime qualification is recorded in issue-nvvm-backend/RESULTS.md:
// live interior bases, signed indices -1/0/1/2, independent 48/40-byte strides and guards.
// This source does not dereference records or create test addresses from null pointers.
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

// No record is dereferenced. Signed offsets remain inside the host allocations.
[CUDAKernel]
void computeMain(
    uniform Ptr<C, Access::ReadWrite, AddressSpace::Device, ScalarDataLayout> s,
    uniform LayoutPtr<C, CDataLayout> c,
    uniform int index,
    uniform Ptr<uint64_t> output)
{
    output[0] = uint64_t(s);
    output[1] = uint64_t(s + index);
    output[2] = output[1] - output[0];
    output[3] = uint64_t(c);
    output[4] = uint64_t(c + index);
    output[5] = output[4] - output[3];
    output[6] = uint64_t(int64_t(index));
    output[7] = 0xc001c0de;
    // output[8] is an independently initialized, untouched host guard.
}
