//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target hlsl -entry computeMain -stage compute

// Regression guard for https://github.com/shader-slang/slang/issues/10641.
//
// `__ref`, `groupshared` and `const groupshared` parameters all pass by reference, but a
// `groupshared` parameter also requires its argument to name group-shared storage (error 30711), and
// a `const groupshared` one is a read-only reference. A method satisfies an interface requirement
// only if each reference parameter has the requirement's exact access (read-write or read-only),
// as for `__ref` and `const __ref`. A method whose parameter requires group-shared storage does not
// satisfy a requirement that does not promise it, while one that accepts any memory location does
// satisfy a `groupshared` requirement.

// --- Rejected: the implementation needs group-shared storage the requirement does not promise ---

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

// --- Rejected: a reference parameter must have the requirement's exact access ---

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

interface ISharedWrite
{
    void touch(groupshared uint a[8]);
//diag:  ^^^^^ see interface requirement declaration of 'touch'
}

struct ReadsOnly : ISharedWrite
{
    void touch(const groupshared uint a[8]) {}
//diag:                               ^ E38108
//diag:                               ^ parameter 'a' direction 'ref_readonly' does not match interface requirement 'ref'.
}

// --- Accepted: the same qualifiers, or an implementation that asks for less storage ---

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

struct AsksForLess : IShared
{
    void write(__ref uint a[8]) { a[1] = 2; }
    uint read(const groupshared uint a[8]) { return a[1]; }
}

groupshared uint sh[8];
RWStructuredBuffer<uint> output;

uint use<T : IShared>(T t)
{
    t.write(sh);
    return t.read(sh);
}

void touch<T : ISharedWrite>(T t)
{
    t.touch(sh);
}

[numthreads(8, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    Exact exact;
    AsksForLess asksForLess;
    output[tid] = use(exact) + use(asksForLess);
}
