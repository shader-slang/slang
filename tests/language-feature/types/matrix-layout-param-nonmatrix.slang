// A matrix layout modifier on a non-matrix parameter written in traditional syntax is diagnosed
// (E39026), the same as on a struct field (shader-slang/slang#13375), and so is one written
// before a modern-syntax declaration with an explicit type (shader-slang/slang#13390). This includes a generic
// parameter type and a type alias for an array, whose layout would have to apply to a matrix the
// modifier cannot reach. A parameter whose type does not resolve reports only the undefined
// identifier, and a diagnostic on the type itself still points at the type.

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

float genericType<T>(row_major T x) { return 0; }
//CHECK:             ^^^^^^^^^ E39026
//CHECK:             ^^^^^^^^^ matrix layout modifier cannot be used on non-matrix type 'T'.

typealias MatrixArray = float2x3[2];
float arrayAlias(row_major MatrixArray m) { return 0; }
//CHECK:         ^^^^^^^^^ E39026
//CHECK:         ^^^^^^^^^ matrix layout modifier cannot be used on non-matrix type 'matrix<float,2,3>[2]'.

float voidType(row_major void x) { return 0; }
//CHECK:       ^^^^^^^^^ E39026
//CHECK:       ^^^^^^^^^ matrix layout modifier cannot be used on non-matrix type 'void'.
//CHECK:                 ^^^^ E30009
//CHECK:                 ^^^^ invalid type 'void'.

struct ModernFields
{
    row_major var count: int;
/*CHECK:
    ^^^^^^^^^ E39026
    ^^^^^^^^^ matrix layout modifier cannot be used on non-matrix type 'int'.
*/
}

func modernParameter(row_major value: int) -> int
//CHECK:             ^^^^^^^^^ E39026
//CHECK:             ^^^^^^^^^ matrix layout modifier cannot be used on non-matrix type 'int'.
{
    row_major let local: int = value;
/*CHECK:
    ^^^^^^^^^ E39026
    ^^^^^^^^^ matrix layout modifier cannot be used on non-matrix type 'int'.
*/
    return local;
}
