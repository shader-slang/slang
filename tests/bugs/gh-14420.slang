// Regression test for #14420: a store through a pointer must not be removed as dead just
// because a later store overwrites it and the call in between is to a `NoSideEffect` callee.
// `NoSideEffect` only rules out observable writes: `readSh` reads `sh[i]` without receiving `p`,
// so dropping the first store changes what `readSh` observes. The expected result is 7 (the
// first store's value 5 read by `readSh`, plus the overwritten value 2); with the bug it is 2.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -g0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -Xslang -g0

groupshared float sh[1];

// Reads `sh` but writes nothing, so func-property inference marks it NoSideEffect.
[noinline]
float readSh(uint i)
{
    return sh[i];
}

// The first store is read by `readSh` before the second store overwrites it, so it must stay.
// CUDA-LABEL: storeReadStorePtr_0(
// CUDA: *[[P:p_[0-9]+]] = [[V:v_[0-9]+]];
// CUDA-NEXT: readSh_0(
// CUDA: *[[P]] = 2.0f;
// METAL-LABEL: storeReadStorePtr_0(
// METAL: *[[P:p_[0-9]+]] = [[V:v_[0-9]+]];
// METAL-NEXT: readSh_0(
// METAL: *[[P]] = 2.0f;
[noinline]
float storeReadStorePtr(Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p, float v, uint i)
{
    *p = v;
    float s = readSh(i);
    *p = 2.0f;
    return s;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outb
RWStructuredBuffer<float> outb;

[numthreads(1, 1, 1)]
void computeMain()
{
    sh[0] = 0.0f;
    float a = storeReadStorePtr(__getAddress(sh[0]), 5.0f, 0);
    outb[0] = a + sh[0];
}

// CHECK: type: float
// CHECK-NEXT: {{^}}7{{$}}
