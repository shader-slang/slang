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

void other()
{
    invalidDefault();
//diag:           ^ could not specialize generic for arguments of type ()
}
