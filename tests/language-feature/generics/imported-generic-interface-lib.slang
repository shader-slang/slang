//TEST_IGNORE_FILE:

// Library half of `imported-generic-interface.slang`.
//
// The interface is generic, so the symbol registered under its mangled name is
// the outer `IRGeneric` rather than the `IRInterfaceType` itself --
// `addLinkageDecoration` hoists linkage to the outermost generic. Borrowing it
// is what drives the `getGenericReturnVal` unwrap and the
// `emitOuterGenerics`/`finishOuterGenerics` pair, all of which are inert for a
// non-generic interface.

module "imported-generic-interface-lib";

public interface IContainer<T>
{
    T get(uint i);
    uint count();
}

public struct Doubling<T : IArithmetic> : IContainer<T>
{
    public T base;

    public __init(T b) { base = b; }

    public T get(uint i) { return base + base; }
    public uint count() { return 4; }
}
