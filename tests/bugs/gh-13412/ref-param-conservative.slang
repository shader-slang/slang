// A `__ref` parameter is a true reference with no copy: the memory behind it may be written by a
// call it is not passed to. Here `x` is bound to `*cb.p`, which `w` writes, so `x` must be
// reloaded after the call (shader-slang/slang#13412). `[noRefInline]` keeps `f` a real call so
// the `__ref` parameter survives to the redundancy passes.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none

struct P
{
    uint* p;
}

//TEST_INPUT: set cb = new P{ ubuffer(data=[0], stride=4) }
ConstantBuffer<P> cb;

//TEST_INPUT: set outb = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outb;

[noinline]
void w()
{
    *cb.p = 2;
}

[noinline]
[noRefInline]
uint f(__ref uint x, uint k)
{
    x = k;
    w();
    return x;
}

// CUDA-LABEL: uint f_0(
// CUDA: w_0();
// CUDA-NEXT: return *
// METAL-LABEL: uint f_0(
// METAL: w_{{[0-9]+}}(
// METAL-NEXT: return *

[numthreads(1, 1, 1)]
void computeMain(uint3 t: SV_DispatchThreadID)
{
    outb[0] = f(*cb.p, t.x + 1);
}

// BUF: type: uint32_t
// BUF-NEXT: {{^}}2{{$}}
