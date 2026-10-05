// A store to storage private to the function is forwarded across a call that is not passed its
// address (see shader-slang/slang#13412). The cases are: a local array kept in memory by a dynamic
// index; a local var whose address is only lent to an `inout` parameter; and an `inout`
// parameter, which has copy-in/copy-out semantics.
// The CPU lane passes `-g0`: render-test compiles with debug info by default, and that keeps the
// load the forwarding would remove, so the lane would pass even on a miscompiling compiler.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none

//TEST_INPUT: set outb = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outb;

[noinline]
void w()
{
    outb[3] = 5;
}

[noinline]
void bump(inout uint y)
{
    y += 1;
}

[noinline]
uint localArray(uint i)
{
    uint arr[4];
    arr[i] = 1;
    w();
    return arr[i];
}

[noinline]
uint lentToInout()
{
    uint x = 0;
    bump(x);
    x = 7;
    w();
    return x;
}

[noinline]
uint inoutParam(inout uint x, uint t)
{
    x = t;
    w();
    return x;
}

// CUDA-LABEL: uint localArray_0(
// CUDA: w_0();
// CUDA-NEXT: return 1U;
// CUDA-LABEL: uint lentToInout_0(
// CUDA: w_0();
// CUDA-NEXT: return 7U;
// CUDA-LABEL: uint inoutParam_0(
// CUDA: *x_{{[0-9]+}} = t_{{[0-9]+}};
// CUDA-NEXT: w_0();
// CUDA-NEXT: return t_{{[0-9]+}};

// METAL-LABEL: uint localArray_0(
// METAL: w_{{[0-9]+}}(kernelContext
// METAL-NEXT: return 1U;
// METAL-LABEL: uint lentToInout_0(
// METAL: w_{{[0-9]+}}(kernelContext
// METAL-NEXT: return 7U;
// METAL-LABEL: uint inoutParam_0(
// METAL: w_{{[0-9]+}}(kernelContext
// METAL-NEXT: return t_{{[0-9]+}};

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outb[0] = localArray(tid.x + 2);
    outb[1] = lentToInout();
    uint a = 0;
    outb[2] = inoutParam(a, tid.x + 3) + a;
}

// BUF: type: uint32_t
// BUF-NEXT: {{^}}1{{$}}
// BUF-NEXT: {{^}}7{{$}}
// BUF-NEXT: {{^}}6{{$}}
// BUF-NEXT: {{^}}5{{$}}
