//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// A type-equality cast is transparent to the abstract-storage use selected by its enclosing
// operation. A read-only borrow through the cast therefore reads a run-time-indexed swizzle from
// the property's getter instead of requiring the property's address through its `ref` accessor.

static int getterCount;
static int refAccessorCount;
static int indexEvaluationCount;
static int4 referencedValue = {};

struct TypeEqualityStorage
{
    property int4 value
    {
        [nonmutating]
        get
        {
            getterCount++;
            return int4(21, 22, 23, 24);
        }

        [nonmutating]
        ref
        {
            refAccessorCount++;
            return referencedValue;
        }
    }
}

int evaluateIndex()
{
    indexEvaluationCount++;
    return 1;
}

int readBorrowedValue<T>(__constref T value)
    where T == int
{
    return value;
}

int readTypeEqualityProjection<T>(TypeEqualityStorage storage)
    where T == int
{
    return readBorrowedValue<T>(storage.value.xy[evaluateIndex()]);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    getterCount = 0;
    refAccessorCount = 0;
    indexEvaluationCount = 0;

    TypeEqualityStorage storage = {};
    outputBuffer[0] = readTypeEqualityProjection<int>(storage);
    outputBuffer[1] = getterCount;
    outputBuffer[2] = refAccessorCount;
    outputBuffer[3] = indexEvaluationCount;
}

// CHECK: type: int32_t
// CHECK-NEXT: 22
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
