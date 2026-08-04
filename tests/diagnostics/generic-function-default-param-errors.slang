//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

interface IFoo
{
}

struct NotIFoo
{
}

// The default `T = NotIFoo` does not conform to the required interface `IFoo`,
// so specializing this generic at the call below reports a focused
// non-conformance diagnostic rather than the general specialization-failed
// fallback.
void invalidDefault<T : IFoo = NotIFoo>()
/*diag:
     ^^^^^^^^^^^^^^ see declaration of func invalidDefault<T> -> void
*/
{
}

void badDefaultOrder<A=B,B=int>()
//diag:                ^ forward reference in generic default initializer
//diag:                ^ generic default initializer for parameter 'A' references parameter 'B' before it is declared
{
}

void badDefaultOrder2<int A=B,int B=10>()
//diag:                     ^ forward reference in generic default initializer
//diag:                     ^ generic default initializer for parameter 'A' references parameter 'B' before it is declared
{
}

void other()
{
    invalidDefault();
//diag:           ^ type argument doesn't conform to interface
//diag:           ^ type argument 'NotIFoo' does not conform to the required interface 'IFoo'
}
