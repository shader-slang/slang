//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

[__NonCopyableType]
struct MoveOnly
{
    int value;
    __init() { value = 1; }
}

interface IFactory
{
    associatedtype Output;
    Output make();
//CHECK:   ^^^^ see interface requirement declaration of 'make'
}

struct ConcreteFactory : IFactory
{
    typedef MoveOnly Output;

    MoveOnly make()
//CHECK:     ^^^^ return type copyability mismatch
//CHECK:     ^^^^ member 'make' return type 'MoveOnly' and interface requirement return type 'MoveOnly' differ in copyability: one is copyable and the other is not.
    {
        return MoveOnly();
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
