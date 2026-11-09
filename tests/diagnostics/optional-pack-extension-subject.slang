//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

interface ITag {}
struct A {}
struct B {}
struct Holder<each T> where optional T : ITag {}

extension Holder<A, uint>
{
    static int value() { return 1; }
}

int mismatch()
{
    return Holder<B, uint>.value();
//CHECK:                   ^^^^^ member not found
//CHECK:                   ^^^^^ 'value' is not a member of 'typeof(Holder<B, uint>)'.
}
