// Diagnostics print the layout of a matrix whose layout is specified, so two types that differ
// only in layout read differently; a matrix with an unspecified layout keeps its usual spelling,
// such as `float2x3` in a candidate note (shader-slang/slang#13383).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

float pick(row_major float2x3 m) { return 1.0; }
/*CHECK:
      ^^^^ candidate: func pick(matrix<float,2,3,MatrixLayoutMode.RowMajor>) -> float
*/
float pick(column_major float2x3 m) { return 2.0; }
/*CHECK:
      ^^^^ candidate: func pick(matrix<float,2,3,MatrixLayoutMode.ColumnMajor>) -> float
*/

float callPick()
{
    float2x3 plain = {};
    return pick(plain);
/*CHECK:
               ^ ambiguous call to 'pick' with arguments of type (matrix<float,2,3>)
*/
}

struct NotAMatrix
{
}

float pickPlain(float2x3 m) { return 1.0; }
/*CHECK:
      ^^^^^^^^^ candidate: func pickPlain(float2x3) -> float
      ^^^^^^^^^ argument 0 does not match: expected 'matrix<float,2,3>', got 'NotAMatrix'
*/
float pickPlain(row_major float2x3 m) { return 2.0; }
/*CHECK:
      ^^^^^^^^^ candidate: func pickPlain(matrix<float,2,3,MatrixLayoutMode.RowMajor>) -> float
      ^^^^^^^^^ argument 0 does not match: expected 'matrix<float,2,3,MatrixLayoutMode.RowMajor>', got 'NotAMatrix'
*/

float callPickPlain()
{
    NotAMatrix notAMatrix;
    return pickPlain(notAMatrix);
/*CHECK:
                    ^ no overload for 'pickPlain' applicable to arguments of type (NotAMatrix)
*/
}

void assignPointer()
{
    var row : row_major float2x3 = {};
    Ptr<column_major float2x3> p = &row;
/*CHECK:
                                   ^ type mismatch in expression
                                   ^ expected an expression of type 'Ptr<matrix<float,2,3,MatrixLayoutMode.ColumnMajor>, Access.ReadWrite, AddressSpace.Device, DefaultDataLayout>', got 'Ptr<matrix<float,2,3,MatrixLayoutMode.RowMajor>, Access.ReadWrite, AddressSpace.Device, DefaultDataLayout>'
                                   ^ explicit conversion from 'Ptr<matrix<float,2,3,MatrixLayoutMode.RowMajor>, Access.ReadWrite, AddressSpace.Device, DefaultDataLayout>' to 'Ptr<matrix<float,2,3,MatrixLayoutMode.ColumnMajor>, Access.ReadWrite, AddressSpace.Device, DefaultDataLayout>' is possible
*/
}
