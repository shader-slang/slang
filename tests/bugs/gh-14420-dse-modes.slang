// Companion to gh-14420.slang: pins both directions of the call classification in
// canInstHaveSideEffectAtAddress, so neither can regress silently. The dead-store scan must
// still REMOVE a store before a `ReadNone` callee (a blanket tightening would be a missed
// optimization caught by no value test), and the load-forwarding scan must still forward
// across a `NoSideEffect` callee that may read the memory (only a write invalidates it).

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -g0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -Xslang -g0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -g0

groupshared float sh[1];

// Reads `sh`, writes nothing: NoSideEffect but not ReadNone.
[noinline]
float readSh(uint i)
{
    return sh[i];
}

// Reads nothing, so a store before a call to it is dead.
[__readNone] [noinline]
float pure(uint i)
{
    return float(i) + 1.0f;
}

// Reads groupshared, so it is not read-none; it is only attached as the forward derivative
// of derivPure, and a plain call to derivPure does not execute it.
[noinline]
DifferentialPair<float> derivReadsShared(DifferentialPair<float> dpx)
{
    return diffPair(dpx.p, dpx.d * 2.0f + sh[0] * 0.0f);
}

// The call site executes only this ReadNone callee, not its attached derivative.
[__readNone] [ForwardDerivative(derivReadsShared)] [noinline]
float derivPure(float x)
{
    return x * 2.0f;
}

// The first store is removed in the next function: `pure` reads nothing.
// CUDA-LABEL: float storePureCall_0(
// CUDA-NOT: {{\*p_[0-9]+ = v_[0-9]+;}}
// CUDA: *[[PA:p_[0-9]+]] = 2.0f;
// METAL-LABEL: float storePureCall_0(
// METAL-NOT: {{\*p_[0-9]+ = v_[0-9]+;}}
// METAL: *[[PA:p_[0-9]+]] = 2.0f;
[noinline]
float storePureCall(Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p, float v, uint i)
{
    *p = v;
    float s = pure(i);
    *p = 2.0f;
    return s;
}

// `readSh` may read `*p`, but only a write could invalidate the value the load is forwarded
// from, so the load of `*p` after the call becomes the stored 1.0f.
// CUDA-LABEL: float forwardAfterRead_0(
// CUDA: *[[PF:p_[0-9]+]] = 1.0f;
// CUDA-NOT: *[[PF]]
// CUDA: {{.*\+ 1\.0f;}}
// METAL: *[[PF:p_[0-9]+]] = 1.0f;
// METAL-NOT: *[[PF]]
// METAL: {{.*\+ 1\.0f;}}
[noinline]
float forwardAfterRead(Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p, uint i)
{
    *p = 1.0f;
    float s = readSh(i);
    float t = *p;
    return s + t;
}

// The first store is removed here too: a call executes only `derivPure`, whose attached
// derivative does not run.
// CUDA-LABEL: float storeDerivCall_0(
// CUDA-NOT: {{\*p_[0-9]+ = v_[0-9]+;}}
// CUDA: *[[PD:p_[0-9]+]] = 2.0f;
// METAL-LABEL: float storeDerivCall_0(
// METAL-NOT: {{\*p_[0-9]+ = v_[0-9]+;}}
// METAL: *[[PD:p_[0-9]+]] = 2.0f;
[noinline]
float storeDerivCall(Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p, float v, float x)
{
    *p = v;
    float s = derivPure(x);
    *p = 2.0f;
    return s;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outb
RWStructuredBuffer<float> outb;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    sh[0] = 0.0f;
    float a = storePureCall(__getAddress(sh[0]), 5.0f, 0);
    float b = forwardAfterRead(__getAddress(sh[0]), 0);
    float c = storeDerivCall(__getAddress(sh[0]), 5.0f, 3.0f);
    outb[0] = a + b + c + sh[0];
}

// CHECK: type: float
// CHECK-NEXT: {{^}}11.000000{{$}}
