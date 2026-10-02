// Regression for shader-slang/slang#13383: functions that differ only in the layout of a matrix in
// their signature are distinct functions, so each one must keep its own body after linking. The
// layout used to be left out of mangled names, so the linker kept one body for all of them.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -Xslang -matrix-layout-row-major

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

func pickArray(m : row_major float2x3[2]) -> float { return 1.0; }
func pickArray(m : column_major float2x3[2]) -> float { return 2.0; }
func pickArray(m : float2x3[2]) -> float { return 3.0; }

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

float pickPointer(Ptr<row_major float2x3> p) { return 8.0; }
float pickPointer(Ptr<float2x3> p) { return 9.0; }

// A function generic over the layout is specialized once per layout; each specialization returns
// the layout it was instantiated with.
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
float callTag<T : ITag>(T t) { return t.tag(); }

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
    outputBuffer[7] = pickPointer(&row);
    outputBuffer[8] = pickPointer(&plain);

    outputBuffer[9] = layoutOf(row);
    outputBuffer[10] = layoutOf(column);

    outputBuffer[11] = callTag(row);
    outputBuffer[12] = callTag(column);
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
