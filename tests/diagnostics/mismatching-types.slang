// mismatching-types.slang
//TEST:SIMPLE(filecheck=CHECK): -target hlsl

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
    
//CHECK: tests/diagnostics/mismatching-types.slang(51): error 30019: expected an expression of type 'GenericOuter<int>', got 'int'
    a = 0;
//CHECK: tests/diagnostics/mismatching-types.slang(53): error 30019: expected an expression of type 'GenericOuter<int>.GenericInner<int>', got 'int'
    a.g = 0;
//CHECK: tests/diagnostics/mismatching-types.slang(55): error 30019: expected an expression of type 'GenericOuter<int>.NonGenericInner', got 'int'
    a.ng = 0;
//CHECK: tests/diagnostics/mismatching-types.slang(57): error 30019: expected an expression of type 'GenericOuter<int>.GenericInner<int>', got 'GenericOuter<float>.GenericInner<float>'
    a.g = b.g;
//CHECK: tests/diagnostics/mismatching-types.slang(59): error 30019: expected an expression of type 'GenericOuter<int>.NonGenericInner', got 'GenericOuter<float>.NonGenericInner'
    a.ng = b.ng;
//CHECK: tests/diagnostics/mismatching-types.slang(61): error 30019: expected an expression of type 'GenericInner<int>', got 'int'
    c.i = 0;
//CHECK: tests/diagnostics/mismatching-types.slang(63): error 30019: expected an expression of type 'GenericInner<int>', got 'GenericInner<float>'
    c.i = c.f;
//CHECK: tests/diagnostics/mismatching-types.slang(65): error 30019: expected an expression of type 'GenericInner<int>.ReallyNested', got 'int'
    c.i.n = 0;
    // OK
    c.i.n.val = 0;
    // OK
    d.val = 0;
    // OK
    c.i = d;

//CHECK: tests/diagnostics/mismatching-types.slang(74): error 30019: expected an expression of type 'Texture1D<int>', got 'Texture1D<float>'
    Texture1D<int> t1 = tex;
//CHECK: tests/diagnostics/mismatching-types.slang(76): error 30019: expected an expression of type 'Texture2D<float>', got 'Texture1D<float>'
    Texture2D<float> t2 = tex;
}