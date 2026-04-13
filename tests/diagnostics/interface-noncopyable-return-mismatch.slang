//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv

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
}

struct ConcreteFactory : IFactory
{
    typedef MoveOnly Output;

    // CHECK: error[E38109]:
    // CHECK-SAME: member 'make' return type 'MoveOnly' and interface requirement return type 'Output' differ in copyability: one is copyable and the other is not
    MoveOnly make()
    {
        return MoveOnly();
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
