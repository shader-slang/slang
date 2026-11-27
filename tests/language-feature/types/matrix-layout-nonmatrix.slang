// Regression for shader-slang/slang#12990: a matrix layout modifier on a non-matrix type is
// diagnosed (E39026), including on an ordinary declaration such as a struct field.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -profile cs_6_0 -entry main

struct S
{
    row_major int badField;
    float y;
}
//CHECK: E39026
//CHECK: matrix layout modifier cannot be used on non-matrix type

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    S s;
    s.badField = 1;
    outputBuffer[0] = s.y;
}
