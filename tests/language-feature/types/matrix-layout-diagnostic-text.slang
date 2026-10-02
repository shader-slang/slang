// Regression for shader-slang/slang#13383: diagnostics print the layout of a matrix whose layout is
// not the default, so two types that differ only in layout no longer read as the same type. A
// plain matrix keeps its usual spelling.

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
