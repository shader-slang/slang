// An `inout` or `__constref` argument bound to memory that a nested call writes. The callee's
// copy `y` holds the value at entry. `specializeFuncsForBufferLoadArgs` rewrites each parameter
// to a buffer element address or a plain pointer, which is not storage private to the callee, so
// its load must stay before the nested call (shader-slang/slang#13412). The CPU lane checks the values only: on CPU
// every call is also passed the kernel context pointer, which the call's argument check treats as
// possibly aliasing the load, so the CPU output is correct with or without the fix.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -line-directive-mode none

struct P
{
    uint* p;
}

//TEST_INPUT: set cb = new P{ ubuffer(data=[0], stride=4) }
ConstantBuffer<P> cb;

//TEST_INPUT: set g = ubuffer(data=[1], stride=4)
RWStructuredBuffer<uint> g;

//TEST_INPUT: set outb = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<uint> outb;

[noinline]
void writeG()
{
    g[0] = 2;
}

[noinline]
void writeP()
{
    *cb.p = 2;
}

[noinline]
uint inoutBuffer(inout uint x)
{
    uint y = x;
    writeG();
    return y;
}

[noinline]
uint inoutPointer(inout uint x)
{
    uint y = x;
    writeP();
    return y;
}

[noinline]
uint constrefPointer(__constref uint x)
{
    uint y = x;
    writeP();
    return y;
}

// CUDA-LABEL: uint inoutBuffer_0(
// CUDA: uint y_0 = *
// CUDA-NEXT: writeG_0();
// CUDA-LABEL: uint inoutPointer_0(
// CUDA: uint y_{{[0-9]+}} = *
// CUDA-NEXT: writeP_0();
// CUDA-LABEL: uint constrefPointer_0(
// CUDA: uint y_{{[0-9]+}} = *
// CUDA-NEXT: writeP_0();

// HLSL-LABEL: uint inoutBuffer_0(
// HLSL: uint y_0 = g_0[
// HLSL-NEXT: writeG_0();

[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = inoutBuffer(g[0]);
    *cb.p = 1;
    outb[1] = inoutPointer(*cb.p);
    *cb.p = 1;
    outb[2] = constrefPointer(*cb.p);
}

// BUF: type: uint32_t
// BUF-NEXT: {{^}}1{{$}}
// BUF-NEXT: {{^}}1{{$}}
// BUF-NEXT: {{^}}1{{$}}
