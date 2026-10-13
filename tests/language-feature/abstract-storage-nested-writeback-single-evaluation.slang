//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

// Slots 0-5: nested projected read-modify-write; slots 6-11: nested projected assignment.
// Slots 12-14: nested direct-`ref` mutation.
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// A nested getter/setter receiver must be evaluated once for the complete read-modify-write.
// Accessor counters detect repeated reads, while preserved sibling fields detect lossy whole-value
// writeback. The IR checks verify the getter, mutation, inner setter, and outer setter sequence.

void increment(inout int value)
{
    value++;
}

struct LeafStorage
{
    int storedValue;
    int getterCallCount;
    int setterCallCount;

    property int value
    {
        [mutating]
        get
        {
            getterCallCount++;
            return storedValue;
        }
        set
        {
            setterCallCount++;
            storedValue = newValue;
        }
    }
}

struct ProjectedValue
{
    LeafStorage inner;
    int preservedValue;
}

struct ProjectedOuterStorage
{
    ProjectedValue storedValue;
    int getterCallCount;
    int setterCallCount;

    property ProjectedValue value
    {
        [mutating]
        get
        {
            getterCallCount++;
            return storedValue;
        }
        set
        {
            setterCallCount++;
            storedValue = newValue;
        }
    }
}

void exerciseProjectedReadModifyWrite(inout ProjectedOuterStorage storage)
{
    // Lowering must keep the one outer-property value through the `inner` field projection so that
    // the inner getter and setter share it. A second outer getter could combine an updated `inner`
    // with unrelated fields from a different value.
    increment(storage.value.inner.value);
}

void exerciseProjectedAssignment(inout ProjectedOuterStorage storage)
{
    // Direct setter assignment uses the same stable receiver and pending write-back, but it does
    // not read the inner property before writing it.
    storage.value.inner.value = 31;
}

struct RefLeafStorage
{
    int storedValue;
    int refCallCount;

    property int value
    {
        [mutating]
        ref
        {
            refCallCount++;
            return storedValue;
        }
    }
}

struct RefOuterStorage
{
    RefLeafStorage storedValue;
    int refCallCount;

    property RefLeafStorage value
    {
        [mutating]
        ref
        {
            refCallCount++;
            return storedValue;
        }
    }
}

void exerciseRefReadModifyWrite(inout RefOuterStorage storage)
{
    // Both accessors expose stable addresses, so lowering passes the nested address directly and
    // creates no getter/setter materialization or pending write-back.
    increment(storage.value.value);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    ProjectedOuterStorage projectedStorage = {};
    projectedStorage.storedValue.inner.storedValue = 10;
    projectedStorage.storedValue.preservedValue = 17;
    exerciseProjectedReadModifyWrite(projectedStorage);

    outputBuffer[0] = projectedStorage.storedValue.inner.storedValue;
    outputBuffer[1] = projectedStorage.storedValue.inner.getterCallCount;
    outputBuffer[2] = projectedStorage.storedValue.inner.setterCallCount;
    outputBuffer[3] = projectedStorage.storedValue.preservedValue;
    outputBuffer[4] = projectedStorage.getterCallCount;
    outputBuffer[5] = projectedStorage.setterCallCount;

    ProjectedOuterStorage assignmentStorage = {};
    assignmentStorage.storedValue.inner.storedValue = 30;
    assignmentStorage.storedValue.preservedValue = 37;
    exerciseProjectedAssignment(assignmentStorage);

    outputBuffer[6] = assignmentStorage.storedValue.inner.storedValue;
    outputBuffer[7] = assignmentStorage.storedValue.inner.getterCallCount;
    outputBuffer[8] = assignmentStorage.storedValue.inner.setterCallCount;
    outputBuffer[9] = assignmentStorage.storedValue.preservedValue;
    outputBuffer[10] = assignmentStorage.getterCallCount;
    outputBuffer[11] = assignmentStorage.setterCallCount;

    RefOuterStorage refStorage = {};
    refStorage.storedValue.storedValue = 40;
    exerciseRefReadModifyWrite(refStorage);

    outputBuffer[12] = refStorage.storedValue.storedValue;
    outputBuffer[13] = refStorage.storedValue.refCallCount;
    outputBuffer[14] = refStorage.refCallCount;
}

// CHECK: type: int32_t
// CHECK-NEXT: 11
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 17
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 31
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 37
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 41
// CHECK-NEXT: 1
// CHECK-NEXT: 1

// IR-LABEL: [nameHint("ProjectedOuterStorage.value.get")]
// IR: func [[PROJECTED_OUTER_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("LeafStorage.value.get")]
// IR: func [[LEAF_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("increment")]
// IR: func [[INCREMENT:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("LeafStorage.value.set")]
// IR: func [[LEAF_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("ProjectedOuterStorage.value.set")]
// IR: func [[PROJECTED_OUTER_SET:%[A-Za-z0-9_]+]] :

// IR-LABEL: [nameHint("exerciseProjectedReadModifyWrite")]
// IR: call [[PROJECTED_OUTER_GET]]
// IR-NOT: call [[PROJECTED_OUTER_GET]]
// IR: call [[LEAF_GET]]
// IR: call [[INCREMENT]]
// IR: call [[LEAF_SET]]
// IR-NOT: call [[PROJECTED_OUTER_GET]]
// IR: call [[PROJECTED_OUTER_SET]]
// IR-NOT: call [[PROJECTED_OUTER_GET]]
// IR: return_val

// IR-LABEL: [nameHint("exerciseProjectedAssignment")]
// IR: call [[PROJECTED_OUTER_GET]]
// IR-NOT: call [[PROJECTED_OUTER_GET]]
// IR-NOT: call [[LEAF_GET]]
// IR: call [[LEAF_SET]]
// IR-NOT: call [[LEAF_GET]]
// IR-NOT: call [[PROJECTED_OUTER_GET]]
// IR: call [[PROJECTED_OUTER_SET]]
// IR-NOT: call [[PROJECTED_OUTER_GET]]
// IR: return_val

// IR-LABEL: [nameHint("RefOuterStorage.value.ref")]
// IR: func [[OUTER_REF:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("RefLeafStorage.value.ref")]
// IR: func [[LEAF_REF:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("exerciseRefReadModifyWrite")]
// IR-NOT: call [[PROJECTED_OUTER_GET]]
// IR: call [[OUTER_REF]]
// IR: call [[LEAF_REF]]
// IR: call [[INCREMENT]]
// IR-NOT: call [[PROJECTED_OUTER_SET]]
// IR: return_val
