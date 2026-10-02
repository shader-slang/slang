// Functions that differ only in the layout of a matrix in their signature are distinct functions,
// so each call runs its own body after linking (shader-slang/slang#13383).
//
// render-test defaults to row-major, so the first line resolves an unspecified layout to row-major
// and the second to column-major. Each layout-qualified overload must keep its own body in both,
// even though `specializeMatrixLayout` makes the unspecified matrix the same IR type as one of
// them. Pointer overloads are in matrix-layout-overload-pointer.slang.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -Xslang -matrix-layout-column-major

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=24):name=rowBuffer
StructuredBuffer<row_major float2x3> rowBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=24):name=columnBuffer
StructuredBuffer<column_major float2x3> columnBuffer;

// `pickArray`, `pickBuffer` and the `ITag` extensions differ only in the mangled names of
// functions and witness tables; without the layout in the name, every call runs one body.

func pickArray(m : row_major float2x3[2]) -> float { return 1.0; }
func pickArray(m : column_major float2x3[2]) -> float { return 2.0; }
func pickArray(m : float2x3[2]) -> float { return 3.0; }

// An argument whose layout matches one overload exactly picks it over the overload that needs a
// layout conversion, for an explicit and for an unspecified layout.
func exactLayout(x : row_major float4x4[2]) -> float { return 1.0; }
func exactLayout(x : float4x4[2]) -> float { return 2.0; }

float pickBuffer(StructuredBuffer<row_major float2x3> b) { return 8.0; }
float pickBuffer(StructuredBuffer<column_major float2x3> b) { return 9.0; }

// `pickBare` and `pickGeneric` also depend on conversion costs: when the cost cache ignores the
// layout, every call is ambiguous (E39999).
float pickBare(row_major float2x3 m) { return 4.0; }
float pickBare(column_major float2x3 m) { return 5.0; }

float pickGeneric<T : __BuiltinFloatingPointType>(matrix<T, 2, 3, MatrixLayoutMode.RowMajor> m)
{
    return 6.0;
}
float pickGeneric<T : __BuiltinFloatingPointType>(matrix<T, 2, 3, MatrixLayoutMode.ColumnMajor> m)
{
    return 7.0;
}

// A function generic over the layout is specialized once per layout, and each specialization
// returns its `MatrixLayoutMode` value (RowMajor = 1, ColumnMajor = 2).
float layoutOf<let L : MatrixLayoutMode>(matrix<float, 2, 3, L> m) { return float(int(L)); }

interface ITag
{
    float tag();
}
extension row_major float2x3 : ITag
{
    float tag() { return 10.0; }
}
extension column_major float2x3 : ITag
{
    float tag() { return 11.0; }
}
extension float2x3 : ITag
{
    float tag() { return 12.0; }
}
float callTag<T : ITag>(T t) { return t.tag(); }
float callExistential(ITag t) { return t.tag(); }

[numthreads(1, 1, 1)]
void computeMain()
{
    var rowArray : row_major float2x3[2] = {};
    var columnArray : column_major float2x3[2] = {};
    var plainArray : float2x3[2] = {};
    outputBuffer[0] = pickArray(rowArray);
    outputBuffer[1] = pickArray(columnArray);
    outputBuffer[2] = pickArray(plainArray);

    var row : row_major float2x3 = {};
    var column : column_major float2x3 = {};
    outputBuffer[3] = pickBare(row);
    outputBuffer[4] = pickBare(column);
    outputBuffer[5] = pickGeneric(row);
    outputBuffer[6] = pickGeneric(column);
    var plain : float2x3 = {};
    outputBuffer[7] = pickBuffer(rowBuffer);
    outputBuffer[8] = pickBuffer(columnBuffer);

    outputBuffer[9] = layoutOf(row);
    outputBuffer[10] = layoutOf(column);

    outputBuffer[11] = callTag(row);
    outputBuffer[12] = callTag(column);
    outputBuffer[13] = callTag(plain);
    outputBuffer[14] = callExistential(column);
    outputBuffer[15] = callExistential(plain);

    row_major float4x4 rowMajor4x4[2] = { float4x4(1), float4x4(2) };
    float4x4 plain4x4[2] = { float4x4(1), float4x4(2) };
    outputBuffer[16] = exactLayout(rowMajor4x4);
    outputBuffer[17] = exactLayout(plain4x4);
}

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 6.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 8.0
// CHECK-NEXT: 9.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 10.0
// CHECK-NEXT: 11.0
// CHECK-NEXT: 12.0
// CHECK-NEXT: 11.0
// CHECK-NEXT: 12.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
