// A swizzle of several distinct components passes semantic checking as an argument for a
// `__ref_readonly` parameter, but like any reference argument it must have a single address,
// and the components of `s.v.xy` do not.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -experimental-feature

struct S { int4 v; }

int leaf2(__ref_readonly int2 x) { return x.x; }

int forward(__ref_readonly S s)
{
    return leaf2(s.v.xy);
//CHECK:             ^^ the form of this l-value argument is not valid for a `ref` parameter
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    S s;
    s.v = int4(1, 2, 3, 4);
    outputBuffer[0] = forward(s);
}
