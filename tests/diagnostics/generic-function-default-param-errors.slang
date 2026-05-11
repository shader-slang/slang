//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

interface IFoo
{
}

struct NotIFoo
{
}

// TODO: Ideally, we'd get the error directly here already, as NotIFoo does not
// conform to IFoo.
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
//diag:           ^ could not specialize generic for arguments of type ()
}
