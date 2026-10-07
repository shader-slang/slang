//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target hlsl -entry computeMain -stage compute

// Regression guard for https://github.com/shader-slang/slang/issues/10641.
//
// `__ref`, `groupshared` and `const groupshared` parameters all pass by reference, but a
// `groupshared` parameter also requires its argument to name group-shared storage (error 30711), and
// a `const groupshared` one is read-only. A method whose parameter drops one of those guarantees
// does not satisfy an interface requirement that has it, since a caller that goes through the
// interface relies on the requirement's guarantees. A method whose parameter asks for less than the
// requirement does satisfy it.

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

// --- Rejected: the implementation writes through a read-only requirement's parameter ---

interface IReadOnly
{
    void f(const groupshared uint a[8]);
//diag:  ^ see interface requirement declaration of 'f'
}

struct WriterForReadOnly : IReadOnly
{
    void f(groupshared uint a[8]) { a[0] = 1; }
//diag:  ^ member 'f' does not match interface requirement.
//diag:  ^ interface requirement mismatch
}

// --- Accepted: the same qualifiers, or an implementation that asks for less ---

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

interface ISharedWrite
{
    void touch(groupshared uint a[8]);
}

struct ReadsOnly : ISharedWrite
{
    void touch(const groupshared uint a[8]) {}
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
    ReadsOnly readsOnly;
    touch(readsOnly);
    output[tid] = use(exact) + use(asksForLess);
}
