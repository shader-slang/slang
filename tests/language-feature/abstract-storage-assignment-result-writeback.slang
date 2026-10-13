//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// Slang assignment expressions denote their left-side storage after the assignment completes.
// Most cases below select an inherited interface property, which stabilizes an existential receiver
// before a later read, write, or projection consumes the assignment result. The final case uses a
// concrete `ref` property to prove that a result with a direct address still supports indexing.

interface IAssignmentResultStorage
{
    property int item
    {
        [mutating] get;
        [mutating] set;
    }

    [nonmutating] int getStoredValue();
    [nonmutating] int getGetterCount();
    [nonmutating] int getSetterCount();
}

interface IDerivedAssignmentResultStorage : IAssignmentResultStorage
{}

struct AssignmentResultStorage : IDerivedAssignmentResultStorage
{
    int storedValue;
    int getterCount;
    int setterCount;

    property int item
    {
        [mutating]
        get
        {
            getterCount++;
            return storedValue;
        }

        [mutating]
        set
        {
            setterCount++;
            storedValue = newValue;
        }
    }

    [nonmutating] int getStoredValue() { return storedValue; }
    [nonmutating] int getGetterCount() { return getterCount; }
    [nonmutating] int getSetterCount() { return setterCount; }
}

void assignThroughAssignmentResult(inout IDerivedAssignmentResultStorage storage)
{
    // The inner assignment commits `7` before its result becomes the outer destination. The outer
    // write must commit `9` through the same stabilized receiver path without reading the property.
    (storage.item = 7) = 9;
}

void setToEleven(inout int value)
{
    value = 11;
}

void passAssignmentResultToInOut(inout IDerivedAssignmentResultStorage storage)
{
    // The `inout` argument first reads the assignment result and then writes the callee's result
    // back through the same receiver path.
    setToEleven((storage.item = 7));
}

int readAssignmentResult(inout IDerivedAssignmentResultStorage storage)
{
    // Reading the assignment result invokes the mutating getter. Its receiver mutation must be
    // committed even though this expression does not perform another write to `item`.
    return storage.item = 7;
}

struct ProjectedAssignmentValue
{
    int field;
    int2 vector;
    int2x2 matrix;
}

interface IProjectedAssignmentStorage
{
    property ProjectedAssignmentValue item
    {
        [mutating] get;
        [mutating] set;
    }

    [nonmutating] ProjectedAssignmentValue getStoredValue();
    [nonmutating] int getGetterCount();
    [nonmutating] int getSetterCount();
}

interface IDerivedProjectedAssignmentStorage : IProjectedAssignmentStorage
{}

struct ProjectedAssignmentStorage : IDerivedProjectedAssignmentStorage
{
    ProjectedAssignmentValue storedValue;
    int getterCount;
    int setterCount;

    property ProjectedAssignmentValue item
    {
        [mutating]
        get
        {
            getterCount++;
            return storedValue;
        }

        [mutating]
        set
        {
            setterCount++;
            storedValue = newValue;
        }
    }

    [nonmutating] ProjectedAssignmentValue getStoredValue() { return storedValue; }
    [nonmutating] int getGetterCount() { return getterCount; }
    [nonmutating] int getSetterCount() { return setterCount; }
}

ProjectedAssignmentValue makeProjectedValue()
{
    return ProjectedAssignmentValue(1, int2(2, 3), int2x2(4, 5, 6, 7));
}

void assignFieldThroughAssignmentResult(inout IDerivedProjectedAssignmentStorage storage)
{
    (storage.item = makeProjectedValue()).field = 9;
}

void assignVectorElementThroughAssignmentResult(
    inout IDerivedProjectedAssignmentStorage storage)
{
    (storage.item = makeProjectedValue()).vector.y = 19;
}

void assignMatrixElementThroughAssignmentResult(
    inout IDerivedProjectedAssignmentStorage storage)
{
    (storage.item = makeProjectedValue()).matrix._m11 = 29;
}

struct RefAssignmentStorage
{
    int2 storedValue;
    int refCount;

    property int2 item
    {
        [mutating]
        ref
        {
            refCount++;
            return storedValue;
        }
    }
}

void assignIndexThroughDirectAssignmentResult(inout RefAssignmentStorage storage)
{
    // The `ref` accessor gives the inner assignment a direct address, so the assignment result has
    // no retained receiver path and an ordinary index can address its second component.
    (storage.item = int2(1, 2))[1] = 9;
}

// Slots 0-2: whole-value chained assignment.
// Slots 3-5: assignment result passed to `inout`.
// Slots 6-9: pure read through a mutating getter.
// Slots 10-12: field projection.
// Slots 13-16: unique vector swizzle projection.
// Slots 17-20: unique matrix swizzle projection.
// Slots 21-23: ordinary index through a direct `ref` address.
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

static const uint kChainedAssignmentResultBase = 0;
static const uint kInOutAssignmentResultBase = 3;
static const uint kReadAssignmentResultBase = 6;
static const uint kFieldAssignmentResultBase = 10;
static const uint kVectorAssignmentResultBase = 13;
static const uint kMatrixAssignmentResultBase = 17;
static const uint kIndexedAssignmentResultBase = 21;

[numthreads(1, 1, 1)]
void computeMain()
{
    IDerivedAssignmentResultStorage chainedStorage = AssignmentResultStorage(0, 0, 0);
    assignThroughAssignmentResult(chainedStorage);
    outputBuffer[kChainedAssignmentResultBase + 0] = chainedStorage.getStoredValue();
    outputBuffer[kChainedAssignmentResultBase + 1] = chainedStorage.getGetterCount();
    outputBuffer[kChainedAssignmentResultBase + 2] = chainedStorage.getSetterCount();

    IDerivedAssignmentResultStorage inOutStorage = AssignmentResultStorage(0, 0, 0);
    passAssignmentResultToInOut(inOutStorage);
    outputBuffer[kInOutAssignmentResultBase + 0] = inOutStorage.getStoredValue();
    outputBuffer[kInOutAssignmentResultBase + 1] = inOutStorage.getGetterCount();
    outputBuffer[kInOutAssignmentResultBase + 2] = inOutStorage.getSetterCount();

    IDerivedAssignmentResultStorage readStorage = AssignmentResultStorage(0, 0, 0);
    outputBuffer[kReadAssignmentResultBase + 0] = readAssignmentResult(readStorage);
    outputBuffer[kReadAssignmentResultBase + 1] = readStorage.getStoredValue();
    outputBuffer[kReadAssignmentResultBase + 2] = readStorage.getGetterCount();
    outputBuffer[kReadAssignmentResultBase + 3] = readStorage.getSetterCount();

    IDerivedProjectedAssignmentStorage fieldStorage =
        ProjectedAssignmentStorage(makeProjectedValue(), 0, 0);
    assignFieldThroughAssignmentResult(fieldStorage);
    outputBuffer[kFieldAssignmentResultBase + 0] = fieldStorage.getStoredValue().field;
    outputBuffer[kFieldAssignmentResultBase + 1] = fieldStorage.getGetterCount();
    outputBuffer[kFieldAssignmentResultBase + 2] = fieldStorage.getSetterCount();

    IDerivedProjectedAssignmentStorage vectorStorage =
        ProjectedAssignmentStorage(makeProjectedValue(), 0, 0);
    assignVectorElementThroughAssignmentResult(vectorStorage);
    outputBuffer[kVectorAssignmentResultBase + 0] = vectorStorage.getStoredValue().vector.x;
    outputBuffer[kVectorAssignmentResultBase + 1] = vectorStorage.getStoredValue().vector.y;
    outputBuffer[kVectorAssignmentResultBase + 2] = vectorStorage.getGetterCount();
    outputBuffer[kVectorAssignmentResultBase + 3] = vectorStorage.getSetterCount();

    IDerivedProjectedAssignmentStorage matrixStorage =
        ProjectedAssignmentStorage(makeProjectedValue(), 0, 0);
    assignMatrixElementThroughAssignmentResult(matrixStorage);
    outputBuffer[kMatrixAssignmentResultBase + 0] = matrixStorage.getStoredValue().matrix._m00;
    outputBuffer[kMatrixAssignmentResultBase + 1] = matrixStorage.getStoredValue().matrix._m11;
    outputBuffer[kMatrixAssignmentResultBase + 2] = matrixStorage.getGetterCount();
    outputBuffer[kMatrixAssignmentResultBase + 3] = matrixStorage.getSetterCount();

    RefAssignmentStorage indexedStorage = {int2(0), 0};
    assignIndexThroughDirectAssignmentResult(indexedStorage);
    outputBuffer[kIndexedAssignmentResultBase + 0] = indexedStorage.storedValue.x;
    outputBuffer[kIndexedAssignmentResultBase + 1] = indexedStorage.storedValue.y;
    outputBuffer[kIndexedAssignmentResultBase + 2] = indexedStorage.refCount;
}

// CHECK: type: int32_t
// Slots 0-2: whole-value chained assignment.
// CHECK-NEXT: 9
// CHECK-NEXT: 0
// CHECK-NEXT: 2
// Slots 3-5: assignment result passed to `inout`.
// CHECK-NEXT: 11
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// Slots 6-9: pure read through a mutating getter.
// CHECK-NEXT: 7
// CHECK-NEXT: 7
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// Slots 10-12: field projection.
// CHECK-NEXT: 9
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// Slots 13-16: unique vector swizzle projection.
// CHECK-NEXT: 2
// CHECK-NEXT: 19
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// Slots 17-20: unique matrix swizzle projection.
// CHECK-NEXT: 4
// CHECK-NEXT: 29
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// Slots 21-23: ordinary index through a direct `ref` address.
// CHECK-NEXT: 1
// CHECK-NEXT: 9
// CHECK-NEXT: 1
