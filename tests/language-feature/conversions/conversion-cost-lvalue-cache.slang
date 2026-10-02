// Overload resolution must not depend on an earlier call that converted an l-value of the same
// types. Converting an l-value argument (for an `inout` parameter) costs extra, so a cached cost
// that ignores l-value-ness makes the by-value `f(make())` calls below pick the `float` overload
// instead of the cheaper `uint` one. The matrix cases come from shader-slang/slang#13383; the
// generic-vector case is shader-slang/slang#13394.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -Xslang -matrix-layout-column-major

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float pickMatrix(row_major uint2x3 m) { return 1.0; }
float pickMatrix(row_major float2x3 m) { return 2.0; }
void touchMatrix(inout row_major uint2x3 m) {}
row_major int2x3 makeMatrix() { return {}; }

float pickGenericLayout<let L : MatrixLayoutMode>(matrix<uint, 2, 3, L> m) { return 1.0; }
float pickGenericLayout<let L : MatrixLayoutMode>(matrix<float, 2, 3, L> m) { return 2.0; }
void touchGenericLayout<let L : MatrixLayoutMode>(inout matrix<uint, 2, 3, L> m) {}
matrix<int, 2, 3, L> makeGenericLayout<let L : MatrixLayoutMode>() { return {}; }
float testGenericLayout<let L : MatrixLayoutMode>()
{
    matrix<int, 2, 3, L> a = {};
    touchGenericLayout<L>(a);
    return pickGenericLayout<L>(makeGenericLayout<L>());
}

float pickVector<let N : int>(vector<uint, N> v) { return 1.0; }
float pickVector<let N : int>(vector<float, N> v) { return 2.0; }
void touchVector<let N : int>(inout vector<uint, N> v) {}
vector<int, N> makeVector<let N : int>() { return vector<int, N>(0); }
float testVector<let N : int>()
{
    vector<int, N> a = vector<int, N>(0);
    touchVector<N>(a);
    return pickVector<N>(makeVector<N>());
}

[numthreads(1, 1, 1)]
void computeMain()
{
    row_major int2x3 a = {};
    touchMatrix(a);
    outputBuffer[0] = pickMatrix(makeMatrix());
    outputBuffer[1] = testGenericLayout<MatrixLayoutMode.RowMajor>();
    outputBuffer[2] = testVector<3>();
}

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
