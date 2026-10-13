//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

// Slots 0-2: built-in index read/write; slots 3-5: user-subscript read/write.
// Slots 6-7: projected and whole assignment; slots 8-9: projected and whole `out`.
// Slots 10-11: nested borrow and borrowed method receiver.
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// Semantic checking delays abstract-storage accessor selection until the use is known. These cases
// make each evaluation step append a digit to one trace, so both the runtime result and the initial
// IR reveal whether lowering preserves the operands' written left-to-right evaluation order after
// that selection.

static int evaluationTrace;

void appendTrace(int digit)
{
    evaluationTrace = evaluationTrace * 10 + digit;
}

int evaluateIndex()
{
    appendTrace(2);
    return 0;
}

int evaluateIndexedAssignmentValue()
{
    appendTrace(3);
    return 77;
}

int evaluatePropertyAssignmentValue()
{
    appendTrace(2);
    return 71;
}

int2 evaluateWholePropertyAssignmentValue()
{
    appendTrace(2);
    return int2(81, 82);
}

struct BuiltinIndexStorage
{
    int2 storedValue;

    property int2 item
    {
        [mutating]
        get
        {
            appendTrace(1);
            return storedValue;
        }

        [mutating]
        set
        {
            appendTrace(3);
            storedValue = newValue;
        }
    }
}

struct RefIndexStorage
{
    int2 storedValue;

    property int2 item
    {
        [mutating]
        ref
        {
            appendTrace(1);
            return storedValue;
        }
    }
}

int readBuiltinIndex(inout BuiltinIndexStorage storage)
{
    evaluationTrace = 0;
    int value = storage.item[evaluateIndex()];
    return evaluationTrace * 100 + value;
}

int writeBuiltinIndexThroughRef(inout RefIndexStorage storage)
{
    evaluationTrace = 0;
    storage.item[evaluateIndex()] = evaluateIndexedAssignmentValue();
    return evaluationTrace;
}

struct UserSubscriptStorage
{
    int2 storedValue;

    __subscript(int index) -> int
    {
        [nonmutating]
        get
        {
            appendTrace(4);
            return storedValue[index];
        }

        [mutating]
        set
        {
            appendTrace(4);
            storedValue[index] = newValue;
        }
    }
}

struct UserSubscriptOuterStorage
{
    UserSubscriptStorage storedValue;

    property UserSubscriptStorage item
    {
        [mutating]
        get
        {
            appendTrace(1);
            return storedValue;
        }

        [mutating]
        set
        {
            appendTrace(5);
            storedValue = newValue;
        }
    }
}

int readUserSubscript(inout UserSubscriptOuterStorage storage)
{
    evaluationTrace = 0;
    int value = storage.item[evaluateIndex()];
    return evaluationTrace * 100 + value;
}

int writeUserSubscript(inout UserSubscriptOuterStorage storage)
{
    evaluationTrace = 0;
    storage.item[evaluateIndex()] = evaluateIndexedAssignmentValue();
    return evaluationTrace;
}

int writeProjectedProperty(inout BuiltinIndexStorage storage)
{
    evaluationTrace = 0;
    storage.item.x = evaluatePropertyAssignmentValue();
    return evaluationTrace;
}

int writeWholeProperty(inout BuiltinIndexStorage storage)
{
    evaluationTrace = 0;
    storage.item = evaluateWholePropertyAssignmentValue();
    return evaluationTrace;
}

int observeOutputArgument()
{
    appendTrace(2);
    return evaluationTrace;
}

void overwriteProjectedValue(out int value, int observation)
{
    appendTrace(4);
    value = 91 + observation * 0;
}

void overwriteWholeValue(out int2 value, int observation)
{
    appendTrace(4);
    value = int2(81, 82) + observation * 0;
}

int writeProjectedOutArgument(inout BuiltinIndexStorage storage)
{
    evaluationTrace = 0;
    overwriteProjectedValue(storage.item.x, observeOutputArgument());
    return evaluationTrace;
}

int writeWholeOutArgument(inout BuiltinIndexStorage storage)
{
    evaluationTrace = 0;
    overwriteWholeValue(storage.item, observeOutputArgument());
    return evaluationTrace;
}

struct BorrowedInnerStorage
{
    int storedValue;

    property int value
    {
        [mutating]
        get
        {
            appendTrace(2);
            return storedValue;
        }
    }
}

struct BorrowedOuterStorage
{
    BorrowedInnerStorage storedValue;

    property BorrowedInnerStorage item
    {
        [mutating]
        get
        {
            appendTrace(1);
            return storedValue;
        }

        [mutating]
        set
        {
            appendTrace(3);
            storedValue = newValue;
        }
    }
}

int observeBorrowedArgument()
{
    appendTrace(4);
    return evaluationTrace;
}

void consumeBorrowedValue(__constref int value, int observation)
{
    appendTrace(5);
    if (value + observation == -1)
        appendTrace(9);
}

int readBorrowedTemporary(inout BorrowedOuterStorage storage)
{
    evaluationTrace = 0;
    consumeBorrowedValue(storage.item.value, observeBorrowedArgument());
    return evaluationTrace;
}

struct BorrowedMethodReceiver
{
    int storedValue;

    [nonmutating]
    void consume(int observation)
    {
        appendTrace(5);
        if (storedValue + observation == -1)
            appendTrace(9);
    }
}

struct BorrowedMethodContainer
{
    BorrowedMethodReceiver storedValue;
    int getterCount;

    property BorrowedMethodReceiver receiver
    {
        [mutating]
        get
        {
            appendTrace(2);
            getterCount++;
            return storedValue;
        }
    }
}

struct BorrowedMethodOuterStorage
{
    BorrowedMethodContainer storedValue;

    property BorrowedMethodContainer item
    {
        [mutating]
        get
        {
            appendTrace(1);
            return storedValue;
        }

        [mutating]
        set
        {
            appendTrace(3);
            storedValue = newValue;
        }
    }
}

int observeBorrowedReceiverArgument()
{
    appendTrace(4);
    return evaluationTrace;
}

int callBorrowedReceiver(inout BorrowedMethodOuterStorage storage)
{
    evaluationTrace = 0;
    storage.item.receiver.consume(observeBorrowedReceiverArgument());
    return evaluationTrace;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    BuiltinIndexStorage builtinStorage = {};
    builtinStorage.storedValue = int2(11, 12);
    outputBuffer[0] = readBuiltinIndex(builtinStorage);

    RefIndexStorage refStorage = {};
    refStorage.storedValue = int2(21, 22);
    outputBuffer[1] = writeBuiltinIndexThroughRef(refStorage);
    outputBuffer[2] = refStorage.storedValue.x;

    UserSubscriptOuterStorage subscriptStorage = {};
    subscriptStorage.storedValue.storedValue = int2(41, 42);
    outputBuffer[3] = readUserSubscript(subscriptStorage);
    outputBuffer[4] = writeUserSubscript(subscriptStorage);
    outputBuffer[5] = subscriptStorage.storedValue.storedValue.x;

    BuiltinIndexStorage projectedStorage = {};
    projectedStorage.storedValue = int2(61, 62);
    outputBuffer[6] = writeProjectedProperty(projectedStorage);
    outputBuffer[7] = writeWholeProperty(projectedStorage);

    projectedStorage.storedValue = int2(21, 22);
    outputBuffer[8] = writeProjectedOutArgument(projectedStorage);

    projectedStorage.storedValue = int2(31, 32);
    outputBuffer[9] = writeWholeOutArgument(projectedStorage);

    BorrowedOuterStorage borrowedStorage = {};
    borrowedStorage.storedValue.storedValue = 51;
    outputBuffer[10] = readBorrowedTemporary(borrowedStorage);

    BorrowedMethodOuterStorage borrowedMethodStorage = {};
    borrowedMethodStorage.storedValue.storedValue.storedValue = 61;
    outputBuffer[11] = callBorrowedReceiver(borrowedMethodStorage);
}

// CHECK: type: int32_t
// CHECK-NEXT: 1211
// CHECK-NEXT: 123
// CHECK-NEXT: 77
// CHECK-NEXT: 12441
// CHECK-NEXT: 12345
// CHECK-NEXT: 77
// CHECK-NEXT: 123
// CHECK-NEXT: 23
// CHECK-NEXT: 1243
// CHECK-NEXT: 243
// CHECK-NEXT: 12345
// CHECK-NEXT: 12345

// IR-LABEL: [nameHint("BuiltinIndexStorage.item.get")]
// IR: func [[BUILTIN_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("evaluateIndex")]
// IR: func [[EVALUATE_INDEX:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readBuiltinIndex")]
// IR: call [[BUILTIN_GET]]
// IR: call [[EVALUATE_INDEX]]
// IR: return_val

// IR-LABEL: [nameHint("RefIndexStorage.item.ref")]
// IR: func [[INDEX_REF:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("evaluateIndexedAssignmentValue")]
// IR: func [[EVALUATE_INDEXED_VALUE:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("writeBuiltinIndexThroughRef")]
// IR: call [[INDEX_REF]]
// IR: call [[EVALUATE_INDEX]]
// IR: call [[EVALUATE_INDEXED_VALUE]]
// IR: return_val

// IR-LABEL: [nameHint("UserSubscriptOuterStorage.item.get")]
// IR: func [[SUBSCRIPT_OUTER_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("UserSubscriptStorage.operator[].get")]
// IR: func [[SUBSCRIPT_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readUserSubscript")]
// IR: call [[SUBSCRIPT_OUTER_GET]]
// IR: call [[EVALUATE_INDEX]]
// IR: call [[SUBSCRIPT_GET]]
// IR: return_val

// IR-LABEL: [nameHint("UserSubscriptStorage.operator[].set")]
// IR: func [[SUBSCRIPT_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("UserSubscriptOuterStorage.item.set")]
// IR: func [[SUBSCRIPT_OUTER_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("writeUserSubscript")]
// IR: call [[SUBSCRIPT_OUTER_GET]]
// IR: call [[EVALUATE_INDEX]]
// IR: call [[EVALUATE_INDEXED_VALUE]]
// IR: call [[SUBSCRIPT_SET]]
// IR: call [[SUBSCRIPT_OUTER_SET]]
// IR: return_val

// IR-LABEL: [nameHint("evaluatePropertyAssignmentValue")]
// IR: func [[EVALUATE_PROPERTY_VALUE:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("BuiltinIndexStorage.item.set")]
// IR: func [[BUILTIN_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("writeProjectedProperty")]
// IR: call [[BUILTIN_GET]]
// IR: call [[EVALUATE_PROPERTY_VALUE]]
// IR: call [[BUILTIN_SET]]
// IR: return_val

// IR-LABEL: [nameHint("evaluateWholePropertyAssignmentValue")]
// IR: func [[EVALUATE_WHOLE_PROPERTY_VALUE:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("writeWholeProperty")]
// IR-NOT: call [[BUILTIN_GET]]
// IR: call [[EVALUATE_WHOLE_PROPERTY_VALUE]]
// IR: call [[BUILTIN_SET]]
// IR-NOT: call [[BUILTIN_GET]]
// IR: return_val

// IR-LABEL: [nameHint("observeOutputArgument")]
// IR: func [[OBSERVE_OUTPUT:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("overwriteProjectedValue")]
// IR: func [[OVERWRITE_PROJECTED:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("writeProjectedOutArgument")]
// IR: call [[BUILTIN_GET]]
// IR: call [[OBSERVE_OUTPUT]]
// IR: call [[OVERWRITE_PROJECTED]]
// IR: call [[BUILTIN_SET]]
// IR: return_val

// IR-LABEL: [nameHint("overwriteWholeValue")]
// IR: func [[OVERWRITE_WHOLE:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("writeWholeOutArgument")]
// IR-NOT: call [[BUILTIN_GET]]
// IR: call [[OBSERVE_OUTPUT]]
// IR: call [[OVERWRITE_WHOLE]]
// IR: call [[BUILTIN_SET]]
// IR-NOT: call [[BUILTIN_GET]]
// IR: return_val

// IR-LABEL: [nameHint("BorrowedOuterStorage.item.get")]
// IR: func [[BORROWED_OUTER_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("BorrowedInnerStorage.value.get")]
// IR: func [[BORROWED_INNER_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("BorrowedOuterStorage.item.set")]
// IR: func [[BORROWED_OUTER_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("observeBorrowedArgument")]
// IR: func [[OBSERVE_BORROWED:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("consumeBorrowedValue")]
// IR: func [[CONSUME_BORROWED:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readBorrowedTemporary")]
// IR: call [[BORROWED_OUTER_GET]]
// IR: call [[BORROWED_INNER_GET]]
// IR: call [[BORROWED_OUTER_SET]]
// IR: call [[OBSERVE_BORROWED]]
// IR: call [[CONSUME_BORROWED]]
// IR: return_val

// IR-LABEL: [nameHint("BorrowedMethodOuterStorage.item.get")]
// IR: func [[BORROWED_METHOD_OUTER_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("BorrowedMethodContainer.receiver.get")]
// IR: func [[BORROWED_RECEIVER_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("BorrowedMethodOuterStorage.item.set")]
// IR: func [[BORROWED_METHOD_OUTER_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("observeBorrowedReceiverArgument")]
// IR: func [[OBSERVE_BORROWED_RECEIVER:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("BorrowedMethodReceiver.consume")]
// IR: func [[CONSUME_BORROWED_RECEIVER:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("callBorrowedReceiver")]
// IR: call [[BORROWED_METHOD_OUTER_GET]]
// IR: call [[BORROWED_RECEIVER_GET]]
// IR: call [[BORROWED_METHOD_OUTER_SET]]
// IR: call [[OBSERVE_BORROWED_RECEIVER]]
// IR: call [[CONSUME_BORROWED_RECEIVER]]
// IR: return_val
