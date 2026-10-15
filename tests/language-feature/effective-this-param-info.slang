//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain -dump-ir -O0 -o -

// This test checks the IR signatures directly because `in` and `constref` can have identical
// runtime behavior even though they are distinct parameter-passing modes.

struct Counter
{
    int value;

    int readByValue()
    {
        return value;
    }

    [constref]
    int readByBorrow()
    {
        return value;
    }

    [mutating]
    void increment()
    {
        value++;
    }

    [__ref]
    void incrementByReference()
    {
        value++;
    }

    [mutating]
    [constref]
    void incrementWithOverlappingAttributes()
    {
        value++;
    }

    property int item
    {
        get
        {
            return value;
        }
        set
        {
            value = newValue;
        }
    }

    property int nonMutatingItem
    {
        get
        {
            return value;
        }
        [nonmutating]
        set
        {}
    }
}

struct GenericCounter<T>
{
    T value;

    [mutating]
    void replace(T newValue)
    {
        value = newValue;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Counter counter = {0};
    int result = counter.readByValue();
    result += counter.readByBorrow();
    counter.increment();
    counter.incrementByReference();
    counter.incrementWithOverlappingAttributes();
    counter.item = result;
    counter.nonMutatingItem = result;

    GenericCounter<int> genericCounter = {0};
    genericCounter.replace(result);
}

// CHECK-LABEL: [nameHint("Counter.$init")]
// CHECK-NEXT: func {{.*}} : Func(%Counter, Attributed(Int, {{.*}}))
// CHECK-LABEL: [nameHint("Counter.readByValue")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
// CHECK-LABEL: [nameHint("Counter.readByBorrow")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInParam(%Counter{{.*}}))
// CHECK-LABEL: [nameHint("Counter.increment")]
// CHECK-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.incrementByReference")]
// CHECK-NEXT: func {{.*}} : Func(Void, RefParam(%Counter{{.*}}))
// CHECK-LABEL: [nameHint("Counter.incrementWithOverlappingAttributes")]
// CHECK-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.item.set")]
// CHECK-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter), Int)
// CHECK-LABEL: [nameHint("Counter.nonMutatingItem.set")]
// CHECK-NEXT: func {{.*}} : Func(Void, %Counter, Int)
