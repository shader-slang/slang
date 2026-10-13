//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// This test intentionally uses concrete struct inheritance to exercise the inherited-subobject
// projection, so the unstable-inheritance warning is not relevant to the result.
#pragma warning(disable: 30816)

struct BaseValue
{
    int value;

    [mutating]
    void setValue(int newValue)
    {
        value = newValue;
    }
}

struct DerivedValue : BaseValue
{
    int padding;
}

struct Storage
{
    DerivedValue getterValues[2];
    DerivedValue refValues[2];
    int getterCallCount;
    int refCallCount;

    property DerivedValue[2] values
    {
        [mutating]
        get
        {
            getterCallCount++;
            return getterValues;
        }

        [mutating]
        ref
        {
            refCallCount++;
            return refValues;
        }
    }
}

int evaluateIndex(inout int count)
{
    count++;
    return 1;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Storage storage;
    storage.getterValues[1].value = 31;
    storage.refValues[1].value = 41;
    storage.getterCallCount = 0;
    storage.refCallCount = 0;
    int indexEvaluationCount = 0;

    // Member lookup records a concrete-supertype breadcrumb from the indexed `DerivedValue` to its
    // `BaseValue` subobject. `ConstructLookupResultExpr()` replays that breadcrumb as a
    // storage-preserving upcast before selecting `setValue`. The mutating receiver requires the
    // indexed element's address, so the enclosing storage use must pass through that upcast and
    // select `ref`. Treating the upcast operand as an r-value would instead call `get` and try to
    // mutate its temporary.
    storage.values[evaluateIndex(indexEvaluationCount)].setValue(42);

    outputBuffer[0] = storage.refValues[1].value;
    outputBuffer[1] = storage.getterValues[1].value;
    outputBuffer[2] = storage.getterCallCount;
    outputBuffer[3] = storage.refCallCount;
    outputBuffer[4] = indexEvaluationCount;
}

// CHECK: type: int32_t
// CHECK-NEXT: 42
// CHECK-NEXT: 31
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
