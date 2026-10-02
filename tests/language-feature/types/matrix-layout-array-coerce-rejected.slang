//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

// Regression for shader-slang/slang#13376: the conversion between arrays of matrices that differ
// only in layout does not relate other element types or lengths, and an `inout`/`out` argument may
// use it only for an array variable, never for a single matrix or an explicit cast.

RWStructuredBuffer<float> output;

float useFloats(float x[2]) { return x[0]; }
float useThree(float4x4 x[3]) { return x[0][0][0]; }
float useFourByFour(float4x4 x[2]) { return x[0][0][0]; }
void addOne(inout float4x4 x) { x[0][0] += 1; }
void setOne(out float4x4 x) { x = float4x4(1); }
void addOneArray(inout float4x4 x[2]) { x[0][0][0] += 1; }

[numthreads(1, 1, 1)]
void main()
{
    int ints[2] = { 1, 2 };
    row_major float4x4 two[2] = { float4x4(1), float4x4(2) };
    row_major float4x3 fourByThree[2];
    row_major half4x4 halves[2];
    row_major float4x4 single = float4x4(0);

    output[0] = useFloats(ints);
//CHECK:                  ^^^^ error E30019
//CHECK:                  ^^^^ expected an expression of type
    output[1] = useThree(two);
//CHECK:                 ^^^ error E30019
//CHECK:                 ^^^ expected an expression of type
    output[2] = useFourByFour(fourByThree);
//CHECK:                      ^^^^^^^^^^^ error E30019
//CHECK:                      ^^^^^^^^^^^ expected an expression of type
    output[3] = useFourByFour(halves);
//CHECK:                      ^^^^^^ error E30019
//CHECK:                      ^^^^^^ expected an expression of type
    addOne(single);
//CHECK:   ^^^^^^ error E30047
//CHECK:   ^^^^^^ must be l-value
    setOne(single);
//CHECK:   ^^^^^^ error E30047
//CHECK:   ^^^^^^ must be l-value
    addOne((float4x4)single);
//CHECK:             ^^^^^^ error E30047
//CHECK:             ^^^^^^ must be l-value
    addOneArray((float4x4[2])two);
//CHECK:                     ^^^ error E30047
//CHECK:                     ^^^ must be l-value
}
