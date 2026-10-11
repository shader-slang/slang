// A `__ref_writeonly` parameter has the `RefWriteOnly` passing mode, so it matches only a
// `__ref_writeonly` requirement parameter and not a plain `__ref` one, in either direction.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

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
//CHECK:              ^ parameter 'a' direction 'ref' does not match interface requirement 'writeonly ref'.
}

interface IReadWrite
{
    void g(__ref uint a);
//CHECK: ^ see interface requirement declaration of 'g'
}

struct WriteOnlyForReadWrite : IReadWrite
{
    void g(__ref_writeonly uint a) { a = 1; }
//CHECK:                        ^ E38108
//CHECK:                        ^ parameter 'a' direction 'writeonly ref' does not match interface requirement 'ref'.
}

[numthreads(1, 1, 1)]
void main()
{
}
