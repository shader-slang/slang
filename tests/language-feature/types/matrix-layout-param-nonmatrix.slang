// Regression for shader-slang/slang#13375: a matrix layout modifier on a non-matrix parameter
// written in traditional syntax is diagnosed (E39026), the same as on a struct field. A parameter
// whose type does not resolve reports only the undefined identifier.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -profile cs_6_0 -entry main

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main(uniform row_major int x)
//CHECK:          ^^^^^^^^^ E39026
//CHECK:          ^^^^^^^^^ matrix layout modifier cannot be used on non-matrix type 'int'.
{
    outputBuffer[0] = x;
}

void undefinedElementType(uniform row_major UndefinedMatrix values[2])
//CHECK:                                    ^^^^^^^^^^^^^^^ E30015
//CHECK:                                    ^^^^^^^^^^^^^^^ undefined identifier 'UndefinedMatrix'.
{
}
