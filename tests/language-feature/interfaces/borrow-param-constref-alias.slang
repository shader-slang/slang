// `__constref` is the legacy alias of `__borrow`, so both spellings give a parameter the
// `BorrowIn` passing mode. A method declared with one spelling satisfies a requirement declared
// with the other, and a mismatch against an `inout` requirement names the mode `borrow`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

interface IBorrow
{
    int f(__borrow int a);
}

struct ConstRefSatisfiesBorrow : IBorrow
{
    int f(__constref int a) { return a; }
}

interface IConstRef
{
    int g(__constref int a);
}

struct BorrowSatisfiesConstRef : IConstRef
{
    int g(__borrow int a) { return a; }
}

interface IInOut
{
    void h(inout int a);
//CHECK: ^ see interface requirement declaration of 'h'
}

struct BorrowDoesNotSatisfyInOut : IInOut
{
    void h(__borrow int a) {}
//CHECK:                ^ E38108
//CHECK:                ^ parameter 'a' direction 'borrow' does not match interface requirement 'inout'.
}

[numthreads(1, 1, 1)]
void main()
{
}
