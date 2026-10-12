//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

// Regression for shader-slang/slang#13376: the conversion between arrays of matrices that differ
// only in layout does not relate other element types or lengths.

struct S
{
    row_major float4x4 two[2];
    row_major float4x3 fourByThree[2];
    row_major half4x4 halves[2];
}

StructuredBuffer<S> sb;
RWStructuredBuffer<float> output;

float useFloats(float x[2]) { return x[0]; }
float useThree(float4x4 x[3]) { return x[0][0][0]; }
float useFourByFour(float4x4 x[2]) { return x[0][0][0]; }

[numthreads(1, 1, 1)]
void main()
{
    int ints[2] = { 1, 2 };

    output[0] = useFloats(ints);
//CHECK:                  ^^^^ error E30019
//CHECK:                  ^^^^ expected an expression of type
    output[1] = useThree(sb[0].two);
//CHECK:                       ^^^ error E30019
//CHECK:                       ^^^ expected an expression of type
    output[2] = useFourByFour(sb[0].fourByThree);
//CHECK:                            ^^^^^^^^^^^ error E30019
//CHECK:                            ^^^^^^^^^^^ expected an expression of type
    output[3] = useFourByFour(sb[0].halves);
//CHECK:                            ^^^^^^ error E30019
//CHECK:                            ^^^^^^ expected an expression of type
}
