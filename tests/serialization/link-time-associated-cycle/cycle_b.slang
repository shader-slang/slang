import cycle_contract;

public struct ImplB : IFoo
{
    public typealias Assoc = A.Assoc;
}

export struct B : IFoo = ImplB;
