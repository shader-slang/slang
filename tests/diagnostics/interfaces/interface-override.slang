//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

interface IBase
{
    int f();
}

interface IDerived : IBase
{
    __init() {}
/*diag:
    ^^^^^^ interface requirement has body
    ^^^^^^ non-method interface requirement cannot have a body.
*/

    property int value{
        get;

        set{ }
//diag: ^^^ partial interface accessor default implementation
//diag: ^^^ interface accessor default implementation is incomplete; define all accessors or none.
    }

    __subscript(int index)->int
    {
        get;
    }

    override int f() {}
//diag:          ^ interface requirement cannot override
//diag:          ^ interface requirement cannot override a base declaration.
}
