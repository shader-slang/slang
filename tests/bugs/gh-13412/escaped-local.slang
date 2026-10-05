// A local variable whose address escapes can be written by a call it is not passed to, so a
// store to it must not be forwarded across such a call (shader-slang/slang#13412). Each entry
// point leaks `x`'s address differently, and `w` then writes `x` through global memory.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

struct H
{
    uint** pp;
}

//TEST_INPUT: set cb = new H{ ubuffer(data=[0 0], stride=8) }
ConstantBuffer<H> cb;

//TEST_INPUT: set outb = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> outb;

struct S
{
    uint* p;
}

[noinline]
void w()
{
    **cb.pp = 2;
}

[noinline]
void writeThrough(S s)
{
    *s.p = 2;
}

[noinline]
void keep(uint* p)
{
    *cb.pp = p;
}

// The address is stored to global memory before the store that would be forwarded.
[noinline]
uint storedBeforeInit()
{
    uint x = 0;
    *cb.pp = &x;
    x = 1;
    w();
    return x;
}

// Control: the address is stored to global memory after `x = 1`, and that store through a
// user pointer already orders the later load of `x`.
[noinline]
uint storedAfterInit()
{
    uint x = 1;
    *cb.pp = &x;
    w();
    return x;
}

// The address travels inside a struct passed by value.
[noinline]
uint insideByValueStruct()
{
    uint x = 1;
    S s;
    s.p = &x;
    writeThrough(s);
    return x;
}

// The address is passed to a plain pointer parameter that keeps it for a later call.
[noinline]
uint retainedByCallee()
{
    uint x = 0;
    keep(&x);
    x = 1;
    w();
    return x;
}

// The address of an `inout` parameter escapes inside the callee itself.
[noinline]
uint inoutAddressEscapes(inout uint x)
{
    *cb.pp = &x;
    x = 1;
    w();
    return x;
}

// CUDA-LABEL: uint storedBeforeInit_0(
// CUDA: w_0();
// CUDA-NEXT: return x_{{[0-9]+}};
// CUDA-LABEL: uint storedAfterInit_0(
// CUDA: w_0();
// CUDA-NEXT: return x_{{[0-9]+}};
// CUDA-LABEL: uint insideByValueStruct_0(
// CUDA: writeThrough_0(
// CUDA-NEXT: return x_{{[0-9]+}};
// CUDA-LABEL: uint retainedByCallee_0(
// CUDA: w_0();
// CUDA-NEXT: return x_{{[0-9]+}};
// CUDA-LABEL: uint inoutAddressEscapes_0(
// CUDA: w_0();
// CUDA-NEXT: return *x_{{[0-9]+}};

// SPV-DAG: %storedBeforeInit = OpFunction
// SPV-DAG: %insideByValueStruct = OpFunction
// SPV-DAG: %retainedByCallee = OpFunction
// SPV-DAG: %inoutAddressEscapes = OpFunction
// SPV-NOT: OpReturnValue %uint_1

[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = storedBeforeInit();
    outb[1] = storedAfterInit();
    outb[2] = insideByValueStruct();
    outb[3] = retainedByCallee();
    uint a = 0;
    outb[4] = inoutAddressEscapes(a);
}

// BUF: type: uint32_t
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
