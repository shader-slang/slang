// A swizzle of adjacent components in order, such as `s.v.xy`, is a memory location, so it
// passes semantic checking as an argument for a `__ref_readonly` parameter. Lowering cannot yet
// form a pointer to several vector components, so it rejects the argument, as it does for
// `__ref`.

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
