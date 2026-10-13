//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// The first five cases bind or discard a selected callable or property projection. Their deferred
// property receivers must invoke the getter exactly once and must not invoke the setter. The later
// cases distinguish discarding an already completed assignment from consuming its result.

struct Receiver
{
    int value;

    [nonmutating]
    int read()
    {
        return value;
    }
}

struct Holder
{
    Receiver storedReceiver;
    int getterCount;
    int setterCount;

    property Receiver receiver
    {
        [mutating]
        get
        {
            getterCount++;
            return storedReceiver;
        }

        set
        {
            setterCount++;
            storedReceiver = newValue;
        }
    }
}

struct SetterOnlyHolder
{
    Receiver storedReceiver;
    int setterCount;

    property Receiver receiver
    {
        set
        {
            setterCount++;
            storedReceiver = newValue;
        }
    }
}

// Slots 0-9: five [getter count, setter count] pairs for the binding/discard cases.
// Slots 10-15: three [setter count, stored value] assignment-discard pairs.
// Slots 16-18: consumed assignment result [value, getter count, setter count].
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Holder bindingHolder = {{7}, 0, 0};
    let callable = bindingHolder.receiver.read;
    outputBuffer[0] = bindingHolder.getterCount;
    outputBuffer[1] = bindingHolder.setterCount;

    Holder statementHolder = {{7}, 0, 0};
    statementHolder.receiver.read;
    outputBuffer[2] = statementHolder.getterCount;
    outputBuffer[3] = statementHolder.setterCount;

    Holder loopHolder = {{7}, 0, 0};
    for (int i = 0; i < 1; loopHolder.receiver.read)
    {
        i++;
    }
    outputBuffer[4] = loopHolder.getterCount;
    outputBuffer[5] = loopHolder.setterCount;

    Holder storageHolder = {{7}, 0, 0};
    storageHolder.receiver;
    outputBuffer[6] = storageHolder.getterCount;
    outputBuffer[7] = storageHolder.setterCount;

    Holder projectionHolder = {{7}, 0, 0};
    projectionHolder.receiver.value;
    outputBuffer[8] = projectionHolder.getterCount;
    outputBuffer[9] = projectionHolder.setterCount;

    // `visitAssignExpr()` has already emitted the setter call before `visitMemberExpr()` retains
    // the assignment result for member selection. `materialize()` must preserve that completed
    // assignment without reading the setter-only property as though the callable had captured its
    // receiver.
    SetterOnlyHolder assignmentHolder = {{7}, 0};
    Receiver replacement = {8};
    let assignedCallable = (assignmentHolder.receiver = replacement).read;
    outputBuffer[10] = assignmentHolder.setterCount;
    outputBuffer[11] = assignmentHolder.storedReceiver.value;

    // `checkExpressionForDiscard()` validates these completed assignment results with
    // `DiscardResult`. Lowering must likewise preserve the setter call without trying to read the
    // setter-only property after the write.
    SetterOnlyHolder discardedAssignmentHolder = {{7}, 0};
    discardedAssignmentHolder.receiver = replacement;
    outputBuffer[12] = discardedAssignmentHolder.setterCount;
    outputBuffer[13] = discardedAssignmentHolder.storedReceiver.value;

    SetterOnlyHolder loopAssignmentHolder = {{7}, 0};
    for (int i = 0; i < 1; loopAssignmentHolder.receiver = replacement)
    {
        i++;
    }
    outputBuffer[14] = loopAssignmentHolder.setterCount;
    outputBuffer[15] = loopAssignmentHolder.storedReceiver.value;

    // Consuming an assignment result instead asks the destination for its newly written value.
    // The getter and setter counters distinguish that read-after-write from the discard cases.
    Holder consumedAssignmentHolder = {{7}, 0, 0};
    Receiver assignedReceiver = (consumedAssignmentHolder.receiver = replacement);
    outputBuffer[16] = assignedReceiver.value;
    outputBuffer[17] = consumedAssignmentHolder.getterCount;
    outputBuffer[18] = consumedAssignmentHolder.setterCount;
}

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 8
// CHECK-NEXT: 1
// CHECK-NEXT: 8
// CHECK-NEXT: 1
// CHECK-NEXT: 8
// CHECK-NEXT: 8
// CHECK-NEXT: 1
// CHECK-NEXT: 1
