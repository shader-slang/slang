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
//diag: ^^^ interface requirement has body
//diag: ^^^ non-method interface requirement cannot have a body.
    }

    __subscript(int index)->int
    {
        get{}
//diag: ^^^ interface requirement has body
//diag: ^^^ non-method interface requirement cannot have a body.
    }

    override int f() {}
//diag:          ^ interface requirement cannot override
//diag:          ^ interface requirement cannot override a base declaration.
}
