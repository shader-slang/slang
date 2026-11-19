// A `__ref_readonly` parameter has the `RefReadOnly` passing mode, so a plain `__ref`
// (`RefReadWrite`) method does not satisfy a `__ref_readonly` requirement. A `__ref_readonly`
// method does satisfy a plain `__ref` requirement, because it needs less access.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -experimental-feature

interface IReadOnly
{
    void f(__ref_readonly uint a[8]);
//CHECK: ^ see interface requirement declaration of 'f'
}

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

struct ReadsThroughReadOnlyRef : IReadWrite
{
    void g(__ref_readonly uint a[8]) {}
}

[numthreads(1, 1, 1)]
void main()
{
}
