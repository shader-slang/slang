// A store through a pointer must not be forwarded to a later load through the same pointer
// across a call that may write that memory (shader-slang/slang#13412).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

struct P
{
    uint* p;
}

//TEST_INPUT: set cb = new P{ ubuffer(data=[0 0], stride=4) }
ConstantBuffer<P> cb;

//TEST_INPUT: set outb = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<uint> outb;

[noinline]
void w()
{
    *cb.p = 2;
}

// The pointer is a parameter: `w` writes the same memory through `cb.p`.
[noinline]
uint f(uint* q)
{
    *q = 1;
    w();
    return *q;
}

// CUDA-LABEL: uint f_0(
// CUDA: w_0();
// CUDA-NEXT: return *q_0;
// METAL-LABEL: uint f_0(
// METAL: w_{{[0-9]+}}(
// METAL-NEXT: return *q_0;

[numthreads(1, 1, 1)]
void computeMain()
{
    // The pointer is a value loaded inside this function.
    uint* q = cb.p;
    *q = 1;
    w();
    outb[0] = *q;

    outb[1] = f(cb.p);
}

// CUDA-LABEL: void computeMain(
// CUDA: w_0();
// CUDA-NOT: = 1U;
// CUDA: outb_0)[int(0)]) = *
// HLSL-LABEL: void computeMain(
// HLSL: w_0();
// HLSL-NEXT: outb_0[int(0)] = cb_0.p_0;
// SPV: %computeMain = OpFunction
// SPV: OpFunctionCall %void %w
// SPV-NOT: OpStore {{.*}} %uint_1
// SPV: OpLoad %uint

// BUF: type: uint32_t
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
