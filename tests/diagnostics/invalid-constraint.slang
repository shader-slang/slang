//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

interface IFoo
{
    This foo();
}

struct Foo : IFoo
{
    float x;
    This foo()
    {
        This f;
        f.x = 0.0;
        return f;
    }
}

RWStructuredBuffer<float> output;

float compute<T:Foo>(T f) // error, should be IFoo here.
/*diag:
                ^^^ invalid constraint type
                ^^^ type 'Foo' cannot be used as a constraint.
*/
{
}

void main()
{}
