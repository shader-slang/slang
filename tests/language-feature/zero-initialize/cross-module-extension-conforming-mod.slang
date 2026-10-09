// Helper module for cross-module-extension-conforming.slang (issue #12917).
// `Concrete` gets `IDefaultInitializable` via a `public extension` rather than a direct base
// clause — the reporter's documented workaround. No //TEST directive: imported, not run directly.

module "cross-module-extension-conforming-mod";

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

public extension Concrete : IDefaultInitializable {}
