// A `__ref_writeonly` parameter has the `RefWriteOnly` passing mode, so a plain `__ref`
// (`RefReadWrite`) method does not satisfy a `__ref_writeonly` requirement. A `__ref_writeonly`
// method does satisfy a plain `__ref` requirement, because it needs less access. A
// `__ref_writeonly` method and a `__ref_readonly` requirement, or the reverse, each need an
// access the other does not grant, so neither satisfies the other.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -experimental-feature

interface IWriteOnly
{
    void f(__ref_writeonly uint a);
//CHECK: ^ see interface requirement declaration of 'f'
}

struct WriteOnlyMatch : IWriteOnly
{
    void f(__ref_writeonly uint a) { a = 1; }
}

struct ReadWriteForWriteOnly : IWriteOnly
{
    void f(__ref uint a) { a = 1; }
//CHECK:              ^ E38108
//CHECK:              ^ parameter 'a' direction 'ref' does not match interface requirement 'ref_writeonly'.
}

interface IReadWrite
{
    void g(__ref uint a);
}

struct WriteOnlyForReadWrite : IReadWrite
{
    void g(__ref_writeonly uint a) { a = 1; }
}

interface IReadOnly
{
    void h(__ref_readonly uint a);
//CHECK: ^ see interface requirement declaration of 'h'
}

struct WriteOnlyForReadOnly : IReadOnly
{
    void h(__ref_writeonly uint a) { a = 1; }
//CHECK:                        ^ E38108
//CHECK:                        ^ parameter 'a' direction 'ref_writeonly' does not match interface requirement 'ref_readonly'.
}

interface IWriteOnlyAgain
{
    void k(__ref_writeonly uint a);
//CHECK: ^ see interface requirement declaration of 'k'
}

struct ReadOnlyForWriteOnly : IWriteOnlyAgain
{
    void k(__ref_readonly uint a) {}
//CHECK:                       ^ E38108
//CHECK:                       ^ parameter 'a' direction 'ref_readonly' does not match interface requirement 'ref_writeonly'.
}

[numthreads(1, 1, 1)]
void main()
{
}
