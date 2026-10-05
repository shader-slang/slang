// A local variable whose address escapes can be read or written by a call it is not passed to.
// A store to it is therefore neither forwarded to a later load across such a call nor removed as
// dead before one (shader-slang/slang#13412). Each helper function below leaks the address in a
// different way, and `w` then writes the variable, or `r` reads it, through global memory.
// The CPU lane passes `-g0`: render-test compiles with debug info by default, and that keeps the
// load the forwarding would remove, so the lane would pass even on a miscompiling compiler.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

struct H
{
    uint** pp;
}

//TEST_INPUT: set cb = new H{ ubuffer(data=[0 0], stride=8) }
ConstantBuffer<H> cb;

//TEST_INPUT: set outb = out ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4)
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
void r()
{
    outb[8] = **cb.pp;
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

// An escaping element address makes the whole array non-private.
[noinline]
uint elementAddressEscapes()
{
    uint a[2];
    a[0] = 0;
    a[1] = 0;
    *cb.pp = &a[1];
    a[1] = 1;
    w();
    return a[1];
}

struct F
{
    uint g;
    uint f;
}

// An escaping field address makes the whole struct non-private.
[noinline]
uint fieldAddressEscapes()
{
    F s;
    s.g = 0;
    s.f = 0;
    *cb.pp = &s.f;
    s.f = 1;
    w();
    return s.f;
}

// `x = 1` must survive the later `x = 3`, because `r` reads it through the escaped address.
[noinline]
uint storeBeforeReadingCall()
{
    uint x = 0;
    *cb.pp = &x;
    x = 1;
    r();
    x = 3;
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
// CUDA-LABEL: uint elementAddressEscapes_0(
// CUDA: w_0();
// CUDA-NEXT: return a_{{[0-9]+}}[int(1)];
// CUDA-LABEL: uint fieldAddressEscapes_0(
// CUDA: w_0();
// CUDA-NEXT: return (&s_{{[0-9]+}})->f_0;
// CUDA-LABEL: uint storeBeforeReadingCall_0(
// CUDA: x_{{[0-9]+}} = 1U;
// CUDA-NEXT: r_0();

// SPV-LABEL: %storedBeforeInit = OpFunction
// SPV-NOT: OpReturnValue %uint_1
// SPV: OpFunctionEnd
// SPV-LABEL: %insideByValueStruct = OpFunction
// SPV-NOT: OpReturnValue %uint_1
// SPV: OpFunctionEnd
// SPV-LABEL: %retainedByCallee = OpFunction
// SPV-NOT: OpReturnValue %uint_1
// SPV: OpFunctionEnd
// SPV-LABEL: %inoutAddressEscapes = OpFunction
// SPV-NOT: OpReturnValue %uint_1
// SPV: OpFunctionEnd
// SPV-LABEL: %elementAddressEscapes = OpFunction
// SPV-NOT: OpReturnValue %uint_1
// SPV: OpFunctionEnd
// SPV-LABEL: %fieldAddressEscapes = OpFunction
// SPV-NOT: OpReturnValue %uint_1
// SPV: OpFunctionEnd

[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = storedBeforeInit();
    outb[1] = storedAfterInit();
    outb[2] = insideByValueStruct();
    outb[3] = retainedByCallee();
    uint a = 0;
    outb[4] = inoutAddressEscapes(a);
    outb[5] = elementAddressEscapes();
    outb[6] = fieldAddressEscapes();
    outb[7] = storeBeforeReadingCall();
}

// BUF: type: uint32_t
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}2{{$}}
// BUF-NEXT: {{^}}3{{$}}
// BUF-NEXT: {{^}}1{{$}}
