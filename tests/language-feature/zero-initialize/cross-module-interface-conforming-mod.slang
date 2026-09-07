// Helper module for cross-module-interface-conforming.slang (no //TEST: directive).
// The explicit value constructor is load-bearing: it suppresses member-wise default-constructor
// synthesis, so under `-zero-initialize` the only zero-argument constructor is the synthesized
// `IDefaultInitializable.$init()` witness that the sibling test resolves across modules.
module "cross-module-interface-conforming-mod";

public interface IFoo
{
    public uint get();
}

public struct Concrete : IFoo
{
    uint private_field;
    __init(uint v) { private_field = v; }
    public uint get() { return private_field; }
}
