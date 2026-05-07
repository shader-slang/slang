// extension-cannot-extend-interface.slang

// Test that you cannot directly extend an interface type.
// Instead, one must use `extension<T:IMyInterface> T { ... }`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

interface IMyInterface
{
    int getValue();
}

extension IMyInterface
//CHECK:  ^^^^^^^^^^^^ cannot extend interface type 'IMyInterface'
//CHECK:  ^^^^^^^^^^^^ cannot extend interface type 'IMyInterface'. consider using a generic extension: `extension<T:IMyInterface> T {...}`
{
}
