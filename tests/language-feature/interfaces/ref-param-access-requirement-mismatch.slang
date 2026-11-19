// A `const __ref` parameter has the `RefReadOnly` passing mode and a plain `__ref`
// parameter has `RefReadWrite`. A method that needs more access than the requirement grants
// does not satisfy it (shader-slang/slang#13339), but a method that needs less does.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

interface IReadOnly
{
    void f(const __ref uint a[8]);
//CHECK: ^ see interface requirement declaration of 'f'
}

// A method that writes through its reference must not satisfy a read-only requirement.
struct WritesThroughRef : IReadOnly
{
    void f(__ref uint a[8]) { a[0] = 1; }
//CHECK:              ^ E38108
//CHECK:              ^ parameter 'a' direction 'ref' does not match interface requirement 'ref_readonly'.
}

interface IReadWrite
{
    void g(__ref uint a[8]);
}

// A method that only reads satisfies a read-write requirement, the same way an `in` method
// satisfies an `inout` requirement.
struct ReadsThroughConstRef : IReadWrite
{
    void g(const __ref uint a[8]) {}
}

groupshared uint sh[8];

void callReadOnly<T : IReadOnly>(T t) { t.f(sh); }
void callReadWrite<T : IReadWrite>(T t) { t.g(sh); }

[numthreads(1, 1, 1)]
void main()
{
    WritesThroughRef w;
    callReadOnly(w);
    ReadsThroughConstRef r;
    callReadWrite(r);
}
