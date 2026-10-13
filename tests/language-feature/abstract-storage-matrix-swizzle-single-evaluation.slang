//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

// Slots 0-3: diagonal swizzle read; slots 4-6: borrowed indexed diagonal.
// Slots 7-9: explicit-reference indexed diagonal; slots 10-13: discarded row-index projection.
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// The checked access path keeps a getter/setter property deferred through matrix projections,
// including a matrix swizzle and a run-time row index followed by a component projection. Reading
// any of them must call the getter once for the complete matrix rather than once per selected
// element, and must not schedule setter write-back.

static int getterCount;
static int setterCount;
static int refAccessorCount;
static int indexEvaluationCount;
static int2x2 referencedMatrixValue = {};

struct MatrixStorage
{
    property int2x2 value
    {
        [nonmutating]
        get
        {
            getterCount++;
            return int2x2(11, 12, 21, 22);
        }

        [nonmutating]
        set
        {
            setterCount++;
        }

        [nonmutating]
        ref
        {
            refAccessorCount++;
            return referencedMatrixValue;
        }
    }
}

int2 readDiagonal(MatrixStorage storage)
{
    return storage.value._m00_m11;
}

int readBorrowedScalar(__constref int value)
{
    return value;
}

int readIndexedDiagonal(MatrixStorage storage, int index)
{
    return readBorrowedScalar(storage.value._m00_m11[index]);
}

int readIndexedDiagonalThroughExplicitReference(MatrixStorage storage, int index)
{
    Ref<int, Access.Read> reference = storage.value._m00_m11[index];
    return reference;
}

int evaluateMatrixIndex()
{
    indexEvaluationCount++;
    return 1;
}

void discardIndexedMatrixProjection(MatrixStorage storage)
{
    storage.value[evaluateMatrixIndex()].x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    getterCount = 0;
    setterCount = 0;

    MatrixStorage storage = {};
    int2 diagonal = readDiagonal(storage);
    outputBuffer[0] = diagonal.x;
    outputBuffer[1] = diagonal.y;
    outputBuffer[2] = getterCount;
    outputBuffer[3] = setterCount;

    getterCount = 0;
    setterCount = 0;
    outputBuffer[4] = readIndexedDiagonal(storage, 1);
    outputBuffer[5] = getterCount;
    outputBuffer[6] = setterCount;

    getterCount = 0;
    setterCount = 0;
    outputBuffer[7] = readIndexedDiagonalThroughExplicitReference(storage, 1);
    outputBuffer[8] = getterCount;
    outputBuffer[9] = setterCount;

    getterCount = 0;
    setterCount = 0;
    refAccessorCount = 0;
    indexEvaluationCount = 0;
    discardIndexedMatrixProjection(storage);
    outputBuffer[10] = getterCount;
    outputBuffer[11] = refAccessorCount;
    outputBuffer[12] = indexEvaluationCount;
    outputBuffer[13] = setterCount;
}

// CHECK: type: int32_t
// CHECK-NEXT: 11
// CHECK-NEXT: 22
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 22
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 22
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0

// IR-LABEL: [nameHint("MatrixStorage.value.get")]
// IR: func [[GET_VALUE:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readDiagonal")]
// IR: call [[GET_VALUE]]
// IR-NOT: call [[GET_VALUE]]
// IR-NOT: call {{.*MatrixStoragex5Fvaluex5Fset}}
// IR: return_val
// IR-LABEL: [nameHint("readIndexedDiagonal")]
// IR: call [[GET_VALUE]]
// IR-NOT: call [[GET_VALUE]]
// IR-NOT: call {{.*MatrixStoragex5Fvaluex5Fset}}
// IR: call {{.*readBorrowedScalar}}
// IR-LABEL: [nameHint("readIndexedDiagonalThroughExplicitReference")]
// IR: call [[GET_VALUE]]
// IR-NOT: call [[GET_VALUE]]
// IR-NOT: call {{.*MatrixStoragex5Fvaluex5Fset}}
// IR-LABEL: [nameHint("discardIndexedMatrixProjection")]
// IR: call [[GET_VALUE]]
// IR-NOT: call {{.*MatrixStoragex5Fvaluex5Fref}}
