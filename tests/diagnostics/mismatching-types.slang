// mismatching-types.slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):-target hlsl

Texture1D<float> tex;

struct GenericOuter<T>
{
    struct GenericInner<Q>
    {
        T val;
    };

    struct NonGenericInner
    {
        T val;
    };

    GenericInner<T> g;
    NonGenericInner ng;
};

struct NonGenericOuter
{
    struct GenericInner<T>
    {
        T val;

        struct ReallyNested
        {
            T val;
        };

        ReallyNested n;
    };

    GenericInner<int> i;
    GenericInner<float> f;
};


[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    GenericOuter<int> a;
    GenericOuter<float> b;
    NonGenericOuter c;
    NonGenericOuter.GenericInner<int> d;

    a = 0;
//CHK:  ^ type mismatch in expression
//CHK:  ^ expected an expression of type 'GenericOuter<int>', got 'int'

    a.g = 0;
//CHK:    ^ type mismatch in expression
//CHK:    ^ expected an expression of type 'GenericOuter<int>.GenericInner<int>', got 'int'
//CHK: explicit conversion from 'int' to 'GenericOuter<int>.GenericInner<int>' is possible

    a.ng = 0;
//CHK:     ^ type mismatch in expression
//CHK:     ^ expected an expression of type 'GenericOuter<int>.NonGenericInner', got 'int'
//CHK: explicit conversion from 'int' to 'GenericOuter<int>.NonGenericInner' is possible

    a.g = b.g;
//CHK:      ^ type mismatch in expression
//CHK:      ^ expected an expression of type 'GenericOuter<int>.GenericInner<int>', got 'GenericOuter<float>.GenericInner<float>'

    a.ng = b.ng;
//CHK:       ^^ type mismatch in expression
//CHK:       ^^ expected an expression of type 'GenericOuter<int>.NonGenericInner', got 'GenericOuter<float>.NonGenericInner'

    c.i = 0;
//CHK:    ^ type mismatch in expression
//CHK:    ^ expected an expression of type 'NonGenericOuter.GenericInner<int>', got 'int'
//CHK: explicit conversion from 'int' to 'NonGenericOuter.GenericInner<int>' is possible

    c.i = c.f;
//CHK:      ^ type mismatch in expression
//CHK:      ^ expected an expression of type 'NonGenericOuter.GenericInner<int>', got 'NonGenericOuter.GenericInner<float>'

    c.i.n = 0;
//CHK:      ^ type mismatch in expression
//CHK:      ^ expected an expression of type 'NonGenericOuter.GenericInner<int>.ReallyNested', got 'int'
//CHK: explicit conversion from 'int' to 'NonGenericOuter.GenericInner<int>.ReallyNested' is possible

    // OK
    c.i.n.val = 0;
    // OK
    d.val = 0;
    // OK
    c.i = d;

    Texture1D<int> t1 = tex;
//CHK:                  ^^^ type mismatch in expression
//CHK:                  ^^^ expected an expression of type 'Texture1D<int>', got 'Texture1D<float>'

    Texture2D<float> t2 = tex;
//CHK:                    ^^^ type mismatch in expression
//CHK:                    ^^^ expected an expression of type 'Texture2D<float>', got 'Texture1D<float>'
}
