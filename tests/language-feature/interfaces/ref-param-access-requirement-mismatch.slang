// A `const __ref` parameter has the `RefReadOnly` passing mode and a plain `__ref`
// parameter has `RefReadWrite`, so a method whose parameter uses one mode does not
// satisfy a requirement whose parameter uses the other (shader-slang/slang#13339).

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
//CHECK:              ^ parameter 'a' direction 'ref' does not match interface requirement 'const ref'.
}

interface IReadWrite
{
    void g(__ref uint a[8]);
//CHECK: ^ see interface requirement declaration of 'g'
}

struct ReadsThroughConstRef : IReadWrite
{
    void g(const __ref uint a[8]) {}
//CHECK:                    ^ E38108
//CHECK:                    ^ parameter 'a' direction 'const ref' does not match interface requirement 'ref'.
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
