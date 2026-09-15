//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660: extending an interface is itself rejected
// ("cannot extend interface"), but the ambiguous-extension-member check still runs on the
// rejected extension. An `InterfaceDecl` is an `AggTypeDecl`, so it passes the conflict check's
// type guard, yet an interface target is never registered as a candidate extension, so the
// candidate-index lookup returns -1. A previous revision asserted `index >= 0` and aborted in
// debug builds; the graceful early-return means this exhaustive run shows only the interface
// rejection and no E99997 internal-error abort.

interface IFoo{}

extension IFoo
//CHECK:  ^^^^ cannot extend interface type 'IFoo'
//CHECK:  ^^^^ cannot extend interface type 'IFoo'. consider using a generic extension: `extension<T:IFoo> T {...}`
{
    int f() { return 0; }
}
