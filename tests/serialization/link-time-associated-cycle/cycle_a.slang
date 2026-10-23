import cycle_contract;

public struct ImplA : IFoo
{
    public typealias Assoc = B.Assoc;
}

export struct A : IFoo = ImplA;
