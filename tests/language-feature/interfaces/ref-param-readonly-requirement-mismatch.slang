// A `readonly __ref` parameter has the `RefReadOnly` passing mode, so it does not match a
// plain `__ref` (`RefReadWrite`) requirement parameter, in either direction.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

interface IReadOnly
{
    void f(readonly __ref uint a[8]);
//CHECK: ^ see interface requirement declaration of 'f'
}

struct WritesThroughRef : IReadOnly
{
    void f(__ref uint a[8]) { a[0] = 1; }
//CHECK:              ^ E38108
//CHECK:              ^ parameter 'a' direction 'ref' does not match interface requirement 'readonly ref'.
}

interface IReadWrite
{
    void g(__ref uint a[8]);
//CHECK: ^ see interface requirement declaration of 'g'
}

struct ReadsThroughReadOnlyRef : IReadWrite
{
    void g(readonly __ref uint a[8]) {}
//CHECK:                       ^ E38108
//CHECK:                       ^ parameter 'a' direction 'readonly ref' does not match interface requirement 'ref'.
}

[numthreads(1, 1, 1)]
void main()
{
}
