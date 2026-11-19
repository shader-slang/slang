//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target hlsl -entry computeMain -stage compute

// A method's reference parameter must not need more access than the interface requirement's
// grants (E38108): a read-only method can satisfy a read-write requirement, but a read-write
// method cannot satisfy a read-only one. It satisfies the requirement directly only if it has the
// same access and refers to storage in the same address space (shader-slang/slang#10641).
// Otherwise, a synthesized witness forwards the requirement's parameters to the method, which
// works where the forwarding call type-checks: a
// `groupshared` argument can be passed to a `__ref` parameter, but a `__ref` argument cannot be
// passed to a `groupshared` one (E30711). The witness's parameters keep the requirement's
// `groupshared` and `const`.

// --- Rejected: a `__ref` requirement cannot forward to a `groupshared` parameter ---

interface IRef
{
    void f(__ref uint a[8]);
//diag:  ^ see interface requirement declaration of 'f'
}

struct SharedForRef : IRef
{
    void f(groupshared uint a[8]) { a[0] = 1; }
//diag:  ^ member 'f' does not match interface requirement.
//diag:  ^ interface requirement mismatch
}

// --- Rejected: the method needs more access than the requirement grants ---

interface IReadOnly
{
    void f(const groupshared uint a[8]);
//diag:  ^ see interface requirement declaration of 'f'
}

struct WriterForReadOnly : IReadOnly
{
    void f(groupshared uint a[8]) { a[0] = 1; }
//diag:                     ^ E38108
//diag:                     ^ parameter 'a' direction 'ref' does not match interface requirement 'ref_readonly'.
}

// --- Accepted: the same qualifiers, a `__ref` parameter for a `groupshared` requirement, a
// read-only parameter for a read-write requirement, and an extra defaulted parameter ---

interface ISharedWrite
{
    void touch(groupshared uint a[8]);
}

struct ReadsOnly : ISharedWrite
{
    void touch(const groupshared uint a[8]) {}
}

interface IShared
{
    void write(groupshared uint a[8]);
    uint read(const groupshared uint a[8]);
}

struct Exact : IShared
{
    void write(groupshared uint a[8]) { a[0] = 1; }
    uint read(const groupshared uint a[8]) { return a[0]; }
}

struct AnyStorage : IShared
{
    void write(__ref uint a[8]) { a[1] = 2; }
    uint read(const groupshared uint a[8]) { return a[1]; }
}

struct ExtraDefaultedParameter : IShared
{
    void write(groupshared uint a[8], uint x = 0) { a[0] = x; }
    uint read(const groupshared uint a[8], uint x = 0) { return a[0] + x; }
}

groupshared uint sh[8];
RWStructuredBuffer<uint> output;

uint use<T : IShared>(T t)
{
    t.write(sh);
    return t.read(sh);
}

[numthreads(8, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    Exact exact;
    AnyStorage anyStorage;
    ExtraDefaultedParameter extraDefaultedParameter;
    output[tid] = use(exact) + use(anyStorage) + use(extraDefaultedParameter);
}
