//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

// Semantic checking records the enclosing property and each later projection in an abstract
// storage access path while opening an existential. Lowering represents a writable getter/setter
// access as pending write-back. These cases prove that lowering carries that state through
// non-consuming swizzle, l-value conversion, and index projections to the eventual read or
// mutation instead of committing it while forming the projection.

void increment(inout int value)
{
    value++;
}

void incrementConverted(inout uint4 value)
{
    value++;
}

interface IVectorStorage
{
    property int4 value
    {
        [mutating] get;
        [mutating] set;
    }

    [nonmutating] int4 getStoredValue();
    [nonmutating] int getGetterCount();
    [nonmutating] int getSetterCount();
}

struct VectorStorage : IVectorStorage
{
    int4 storedValue;
    int getterCount;
    int setterCount;

    property int4 value
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

    [nonmutating] int4 getStoredValue() { return storedValue; }
    [nonmutating] int getGetterCount() { return getterCount; }
    [nonmutating] int getSetterCount() { return setterCount; }
}

struct InterfaceStorage
{
    IVectorStorage backing;
    int getterCount;
    int setterCount;

    property IVectorStorage item
    {
        [mutating]
        get
        {
            getterCount++;
            return backing;
        }

        set
        {
            setterCount++;
            backing = newValue;
        }
    }
}

struct OuterStorage
{
    InterfaceStorage backing;
    int getterCount;
    int setterCount;

    property InterfaceStorage inner
    {
        [mutating]
        get
        {
            getterCount++;
            return backing;
        }

        set
        {
            setterCount++;
            backing = newValue;
        }
    }
}

void mutateSwizzle(inout InterfaceStorage storage)
{
    increment(storage.item.value.x);
}

void mutateConverted(inout OuterStorage storage)
{
    incrementConverted(storage.inner.item.value);
}

static int4 globalRefValue = int4(10, 20, 30, 40);

interface IRefVector
{
    property int4 value
    {
        [nonmutating] ref;
    }

    [nonmutating] int readFirst();
}

struct RefVector : IRefVector
{
    property int4 value
    {
        [nonmutating]
        ref
        {
            return globalRefValue;
        }
    }

    [nonmutating] int readFirst() { return globalRefValue.x; }
}

struct RefInnerStorage
{
    IRefVector backing;
    int getterCount;
    int setterCount;

    property IRefVector item
    {
        [mutating]
        get
        {
            getterCount++;
            return backing;
        }

        set
        {
            setterCount++;
            backing = newValue;
        }
    }
}

struct RefOuterStorage
{
    RefInnerStorage backing;
    int getterCount;
    int setterCount;

    property RefInnerStorage inner
    {
        [mutating]
        get
        {
            getterCount++;
            return backing;
        }

        set
        {
            setterCount++;
            globalRefValue.x = 99;
            backing = newValue;
        }
    }
}

int combine(int first, int second)
{
    return first * 100 + second;
}

int readRefArgumentOrder(inout RefOuterStorage storage)
{
    // Invoke lowering must complete the first argument's pending receiver write-back before
    // lowering the second argument, which observes the global value changed by the outer setter.
    return combine(storage.inner.item.value[0], globalRefValue.x);
}

int readMethodArgumentOrder(inout RefOuterStorage storage)
{
    // Invoke lowering must complete the selected interface call's pending receiver write-back
    // before lowering the next argument, which reads `globalRefValue`, just like the equivalent
    // `ref` read above.
    return combine(storage.inner.item.readFirst(), globalRefValue.x);
}

// Slots 0-4: swizzle mutation; slots 5-12: converted nested mutation.
// Slots 13-16: `ref` argument ordering; slots 17-20: method argument ordering.
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    InterfaceStorage swizzleStorage = {VectorStorage(int4(1, 2, 3, 4), 0, 0), 0, 0};
    mutateSwizzle(swizzleStorage);
    outputBuffer[0] = swizzleStorage.backing.getStoredValue().x;
    outputBuffer[1] = swizzleStorage.backing.getGetterCount();
    outputBuffer[2] = swizzleStorage.backing.getSetterCount();
    outputBuffer[3] = swizzleStorage.getterCount;
    outputBuffer[4] = swizzleStorage.setterCount;

    OuterStorage convertedStorage = {
        {VectorStorage(int4(10, 20, 30, 40), 0, 0), 0, 0},
        0,
        0};
    mutateConverted(convertedStorage);
    outputBuffer[5] = convertedStorage.backing.backing.getStoredValue().x;
    outputBuffer[6] = convertedStorage.backing.backing.getStoredValue().w;
    outputBuffer[7] = convertedStorage.backing.backing.getGetterCount();
    outputBuffer[8] = convertedStorage.backing.backing.getSetterCount();
    outputBuffer[9] = convertedStorage.backing.getterCount;
    outputBuffer[10] = convertedStorage.backing.setterCount;
    outputBuffer[11] = convertedStorage.getterCount;
    outputBuffer[12] = convertedStorage.setterCount;

    globalRefValue = int4(10, 20, 30, 40);
    RefOuterStorage refArgumentStorage = {{RefVector(), 0, 0}, 0, 0};
    outputBuffer[13] = readRefArgumentOrder(refArgumentStorage);
    outputBuffer[14] = globalRefValue.x;
    outputBuffer[15] = refArgumentStorage.getterCount;
    outputBuffer[16] = refArgumentStorage.setterCount;

    globalRefValue = int4(10, 20, 30, 40);
    RefOuterStorage methodArgumentStorage = {{RefVector(), 0, 0}, 0, 0};
    outputBuffer[17] = readMethodArgumentOrder(methodArgumentStorage);
    outputBuffer[18] = globalRefValue.x;
    outputBuffer[19] = methodArgumentStorage.getterCount;
    outputBuffer[20] = methodArgumentStorage.setterCount;
}

// CHECK: type: int32_t
// CHECK-NEXT: 2
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 11
// CHECK-NEXT: 41
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1099
// CHECK-NEXT: 99
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1099
// CHECK-NEXT: 99
// CHECK-NEXT: 1
// CHECK-NEXT: 1

// IR-LABEL: [nameHint("InterfaceStorage.item.get")]
// IR: func [[ITEM_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("increment")]
// IR: func [[INCREMENT:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("InterfaceStorage.item.set")]
// IR: func [[ITEM_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("mutateSwizzle")]
// IR: call [[ITEM_GET]]
// IR: [[SWIZZLE_SET:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness
// IR: [[SWIZZLE_GET:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness
// IR: call [[SWIZZLE_GET]]
// IR: call [[INCREMENT]]
// IR: call [[SWIZZLE_SET]]
// IR: call [[ITEM_SET]]
// IR-NOT: call [[ITEM_GET]]
// IR: return_val

// IR-LABEL: [nameHint("OuterStorage.inner.get")]
// IR: func [[OUTER_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("incrementConverted")]
// IR: func [[INCREMENT_CONVERTED:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("OuterStorage.inner.set")]
// IR: func [[OUTER_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("mutateConverted")]
// IR: call [[OUTER_GET]]
// IR: call [[ITEM_GET]]
// IR: [[CONVERTED_SET:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness
// IR: [[CONVERTED_GET:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness
// IR: call [[CONVERTED_GET]]
// IR: call [[INCREMENT_CONVERTED]]
// IR: call [[CONVERTED_SET]]
// IR: call [[ITEM_SET]]
// IR: call [[OUTER_SET]]
// IR-NOT: call [[OUTER_GET]]
// IR-NOT: call [[ITEM_GET]]
// IR: return_val

// IR-LABEL: [nameHint("RefOuterStorage.inner.get")]
// IR: func [[REF_OUTER_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("RefInnerStorage.item.get")]
// IR: func [[REF_ITEM_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("RefOuterStorage.inner.set")]
// IR: func [[REF_OUTER_SET:%[A-Za-z0-9_]+]] :

// IR-LABEL: [nameHint("combine")]
// IR: func [[COMBINE:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readRefArgumentOrder")]
// IR: call [[REF_OUTER_GET]]
// IR: call [[REF_ITEM_GET]]
// IR: [[ORDER_ACCESSOR:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness
// IR: [[ORDER_REF:%[A-Za-z0-9_]+]] : {{.*}} = call [[ORDER_ACCESSOR]]
// Lowering snapshots the complete vector for the pending receiver write-back before its setter
// changes the global value. Extracting the requested element afterward is safe because
// `ORDER_VALUE` is an SSA value rather than the accessor's live address.
// IR: [[ORDER_VALUE:%[A-Za-z0-9_]+]] : {{.*}} = load([[ORDER_REF]])
// IR: call [[REF_OUTER_SET]]
// IR: [[ORDER_FIRST:%[A-Za-z0-9_]+]] : {{.*}} = getElement([[ORDER_VALUE]], 0 : Int)
// IR: [[ORDER_GLOBAL:%[A-Za-z0-9_]+]] : {{.*}} = load(%globalRefValue)
// IR: [[ORDER_SECOND:%[A-Za-z0-9_]+]] : {{.*}} = swizzle([[ORDER_GLOBAL]], 0 : Int)
// IR: call [[COMBINE]]([[ORDER_FIRST]], [[ORDER_SECOND]])
// IR-NOT: call [[REF_OUTER_GET]]
// IR-NOT: call [[REF_ITEM_GET]]
// IR: return_val

// IR-LABEL: [nameHint("readMethodArgumentOrder")]
// IR: call [[REF_OUTER_GET]]
// IR: call [[REF_ITEM_GET]]
// IR: [[READ_METHOD:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness
// IR: [[METHOD_FIRST:%[A-Za-z0-9_]+]] : {{.*}} = call [[READ_METHOD]]
// IR: call [[REF_OUTER_SET]]
// IR: [[METHOD_GLOBAL:%[A-Za-z0-9_]+]] : {{.*}} = load(%globalRefValue)
// IR: [[METHOD_SECOND:%[A-Za-z0-9_]+]] : {{.*}} = swizzle([[METHOD_GLOBAL]], 0 : Int)
// IR: call [[COMBINE]]([[METHOD_FIRST]], [[METHOD_SECOND]])
// IR-NOT: call [[REF_OUTER_GET]]
// IR-NOT: call [[REF_ITEM_GET]]
// IR: return_val
