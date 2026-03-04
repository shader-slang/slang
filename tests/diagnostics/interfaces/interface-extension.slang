//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cpp -stage compute -entry main

interface IFoo{}

extension IFoo
//CHECK:  ^^^^ cannot extend interface type 'IFoo'
//CHECK:  ^^^^ cannot extend interface type 'IFoo'. consider using a generic extension: `extension<T:IFoo> T {...}`
{
    int f() { return 0; }
}