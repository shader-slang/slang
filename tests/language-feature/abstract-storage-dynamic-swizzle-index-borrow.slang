//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

// Slots 0 and 4: plain-vector values borrowed by `__constref` and explicit read-only `Ref`.
// Slots 1-3 and 5-7: property [value, getter count, setter count] for those two borrow forms.
// Slots 8-10 and 11-13: [getter count, setter count, trace] for two discarded-expression forms.
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// A run-time index into a swizzle cannot preserve an address for writing. Both a `__constref`
// argument and an explicit read-only `Ref` only borrow the selected value, however, so lowering
// reads the swizzle into a temporary without requiring an address or scheduling a write-back.

static int getterCount;
static int setterCount;
static int evaluationTrace;

struct DynamicIndexStorage
{
    property int4 item
    {
        [nonmutating]
        get
        {
            getterCount++;
            evaluationTrace = evaluationTrace * 10 + 1;
            return int4(21, 22, 23, 24);
        }

        [nonmutating]
        set
        {
            setterCount++;
        }
    }
}

int readBorrowedScalar(__constref int value)
{
    return value;
}

int readVectorProjection(int4 value, int index)
{
    return readBorrowedScalar(value.xy[index]);
}

int readPropertyProjection(DynamicIndexStorage storage, int index)
{
    return readBorrowedScalar(storage.item.xy[index]);
}

int readVectorProjectionThroughExplicitReference(int4 value, int index)
{
    Ref<int, Access.Read> reference = value.xy[index];
    return reference;
}

int readPropertyProjectionThroughExplicitReference(DynamicIndexStorage storage, int index)
{
    Ref<int, Access.Read> reference = storage.item.xy[index];
    return reference;
}

int getTrackedIndex()
{
    evaluationTrace = evaluationTrace * 10 + 2;
    return 1;
}

void discardPropertyProjectionInStatement(DynamicIndexStorage storage)
{
    storage.item.xy[getTrackedIndex()];
}

void discardPropertyProjectionInForIncrement(DynamicIndexStorage storage)
{
    for (int iteration = 0; iteration < 1; storage.item.xy[getTrackedIndex()])
    {
        iteration++;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    getterCount = 0;
    setterCount = 0;

    outputBuffer[0] = readVectorProjection(int4(11, 12, 13, 14), 1);

    DynamicIndexStorage storage = {};
    outputBuffer[1] = readPropertyProjection(storage, 0);
    outputBuffer[2] = getterCount;
    outputBuffer[3] = setterCount;

    getterCount = 0;
    setterCount = 0;
    outputBuffer[4] = readVectorProjectionThroughExplicitReference(int4(11, 12, 13, 14), 1);
    outputBuffer[5] = readPropertyProjectionThroughExplicitReference(storage, 0);
    outputBuffer[6] = getterCount;
    outputBuffer[7] = setterCount;

    getterCount = 0;
    setterCount = 0;
    evaluationTrace = 0;
    discardPropertyProjectionInStatement(storage);
    outputBuffer[8] = getterCount;
    outputBuffer[9] = setterCount;
    outputBuffer[10] = evaluationTrace;

    getterCount = 0;
    setterCount = 0;
    evaluationTrace = 0;
    discardPropertyProjectionInForIncrement(storage);
    outputBuffer[11] = getterCount;
    outputBuffer[12] = setterCount;
    outputBuffer[13] = evaluationTrace;
}

// CHECK: type: int32_t
// CHECK-NEXT: 12
// CHECK-NEXT: 21
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 12
// CHECK-NEXT: 21
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 12
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 12

// IR-LABEL: [nameHint("readVectorProjection")]
// IR: swizzle
// IR: getElement
// IR: call {{.*readBorrowedScalar}}
// IR-LABEL: [nameHint("DynamicIndexStorage.item.get")]
// IR: func [[GET_ITEM:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readPropertyProjection")]
// IR: call [[GET_ITEM]]
// IR-NOT: call [[GET_ITEM]]
// IR-NOT: call {{.*DynamicIndexStoragex5Fitemx5Fset}}
// IR-LABEL: [nameHint("readVectorProjectionThroughExplicitReference")]
// IR: swizzle
// IR: getElement
// IR-LABEL: [nameHint("readPropertyProjectionThroughExplicitReference")]
// IR: call [[GET_ITEM]]
// IR-NOT: call [[GET_ITEM]]
// IR-NOT: call {{.*DynamicIndexStoragex5Fitemx5Fset}}
// IR-LABEL: [nameHint("discardPropertyProjectionInStatement")]
// IR: call [[GET_ITEM]]
// IR-NOT: getElement
// IR-LABEL: [nameHint("discardPropertyProjectionInForIncrement")]
// IR: call [[GET_ITEM]]
// IR-NOT: getElement
