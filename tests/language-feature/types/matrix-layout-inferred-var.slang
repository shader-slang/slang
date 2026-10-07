// A matrix layout written before a declaration whose type is inferred from its initializer
// applies to the inferred matrix type: `row_major var b = m;` holds a row-major matrix, so its
// address converts to `Ptr<row_major float2x3>` and not to `Ptr<column_major float2x3>`. No type
// expression carries the modifier here, so `maybeApplyLayoutModifier` applies it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -matrix-layout-column-major

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    float2x3 m = float2x3(1, 2, 3, 4, 5, 6);
    row_major var inferred = m;
    Ptr<row_major float2x3> rowMajorPointer = &inferred;
    Ptr<column_major float2x3> columnMajorPointer = &inferred;
/*CHECK:
                                                    ^ type mismatch in expression
                                                    ^ expected an expression of type 'Ptr<matrix<float,2,3
                                                    ^ explicit conversion from 'Ptr<matrix<float,2,3
*/
    outputBuffer[0] = (*rowMajorPointer)[0][1] + (*columnMajorPointer)[0][1];
}
