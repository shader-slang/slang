//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -target spirv -entry computeMain -stage compute -no-codegen -dump-ir

// Semantic checking records one stabilizing local binding when it opens an existential. Lowering
// shares that checked binding while extracting the payload, concrete type, and witness. The member
// case proves that lowering performs one field load. The nested property cases additionally prove
// that lowering carries the original property as the distinct write-back destination for the one
// getter result.

interface IReadableValue
{
    [nonmutating]
    int read();
}

struct ReadableValue : IReadableValue
{
    int value;

    [nonmutating]
    int read()
    {
        return value;
    }
}

struct ExistentialMemberStorage
{
    IReadableValue item;
}

int readMember(inout ExistentialMemberStorage storage)
{
    return storage.item.read();
}

interface IMutableValue
{
    [mutating]
    void increment();

    [nonmutating]
    int read();
}

struct MutableValue : IMutableValue
{
    int value;

    [mutating]
    void increment()
    {
        value++;
    }

    [nonmutating]
    int read()
    {
        return value;
    }
}

struct ExistentialFieldContainer
{
    IMutableValue item;
}

struct GetterOnlyExistentialProjectionStorage
{
    ExistentialFieldContainer backing;
    int getterCount;

    property ExistentialFieldContainer value
    {
        [mutating]
        get
        {
            getterCount++;
            return backing;
        }
    }
}

int readGetterOnlyExistentialProjection(inout GetterOnlyExistentialProjectionStorage storage)
{
    // Opening the interface-valued field reads the enclosing getter-only property. The
    // nonmutating method does not need to write the getter result back to the property.
    return storage.value.item.read();
}

struct MutableExistentialProjectionStorage
{
    ExistentialFieldContainer backing;
    int getterCount;
    int setterCount;

    property ExistentialFieldContainer value
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

int readMutableExistentialProjection(inout MutableExistentialProjectionStorage storage)
{
    // A nonmutating method only reads the projected interface value. The available setter must
    // remain unused because the local binding's cached value is never modified.
    return storage.value.item.read();
}

void mutateExistentialProjection(inout MutableExistentialProjectionStorage storage)
{
    // The mutating method updates the opened interface value, so lowering must write the
    // containing field and property value back exactly once.
    storage.value.item.increment();
}

struct MutableExistentialPropertyStorage
{
    IMutableValue backing;
    int getterCount;
    int setterCount;

    property IMutableValue item
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

struct NestedMutableExistentialStorage
{
    MutableExistentialPropertyStorage backing;
    int getterCount;
    int setterCount;

    property MutableExistentialPropertyStorage inner
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

void mutateNestedProperty(inout NestedMutableExistentialStorage storage)
{
    // The stabilizing existential binding must share the outer property's one materialized
    // receiver between the inner getter and setter, then write the updated receiver back once.
    storage.inner.item.increment();
}

int readNestedProperty(inout NestedMutableExistentialStorage storage)
{
    // The mutating inner getter updates the stabilized outer receiver even though `read` does not
    // assign the opened existential value.
    return storage.inner.item.read();
}

void increment(inout int value)
{
    value++;
}

interface IIndexedValue
{
    __subscript(int index) -> int
    {
        [mutating] get;
        [mutating] set;
    }

    [nonmutating] int getValue();
    [nonmutating] int getGetterCount();
    [nonmutating] int getSetterCount();
}

struct IndexedValue : IIndexedValue
{
    int value;
    int getterCount;
    int setterCount;

    __subscript(int index) -> int
    {
        [mutating]
        get
        {
            getterCount++;
            return value;
        }

        [mutating]
        set
        {
            setterCount++;
            value = newValue;
        }
    }

    [nonmutating] int getValue() { return value; }
    [nonmutating] int getGetterCount() { return getterCount; }
    [nonmutating] int getSetterCount() { return setterCount; }
}

struct IndexedExistentialPropertyStorage
{
    IIndexedValue backing;
    int getterCount;
    int setterCount;

    property IIndexedValue item
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

void mutateIndexedProperty(inout IndexedExistentialPropertyStorage storage)
{
    // Semantic checking keeps the selected subscript deferred beyond its existential-opening
    // binding. Lowering then commits the complete indexed read-modify-write to the property once.
    increment(storage.item[0]);
}

interface IMutableValueContainer
{
    __subscript(int index) -> IMutableValue
    {
        [mutating] get;
        [mutating] set;
    }

    [nonmutating] int getStoredValue();
    [nonmutating] int getGetterCount();
    [nonmutating] int getSetterCount();
}

struct MutableValueContainer : IMutableValueContainer
{
    IMutableValue value;
    int getterCount;
    int setterCount;

    __subscript(int index) -> IMutableValue
    {
        [mutating]
        get
        {
            getterCount++;
            return value;
        }

        [mutating]
        set
        {
            setterCount++;
            value = newValue;
        }
    }

    [nonmutating] int getStoredValue() { return value.read(); }
    [nonmutating] int getGetterCount() { return getterCount; }
    [nonmutating] int getSetterCount() { return setterCount; }
}

void mutateNestedIndexedValue(inout IMutableValueContainer container)
{
    // Semantic checking records a second stabilizing binding while opening the subscript result.
    // Lowering writes the updated indexed value through that checked binding before remaking the
    // outer container existential once.
    container[0].increment();
}

// Slot 0: existential member read; slots 1-10: nested mutation and read.
// Slots 11-18: indexed existential mutation; slots 19-20: getter-only projection.
// Slots 21-26: nonmutating and mutating projections with getter/setter counts.
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ExistentialMemberStorage memberStorage = {ReadableValue(23)};
    outputBuffer[0] = readMember(memberStorage);

    NestedMutableExistentialStorage nestedStorage = {{MutableValue(41), 0, 0}, 0, 0};
    mutateNestedProperty(nestedStorage);
    outputBuffer[1] = nestedStorage.backing.backing.read();
    outputBuffer[2] = nestedStorage.backing.getterCount;
    outputBuffer[3] = nestedStorage.backing.setterCount;
    outputBuffer[4] = nestedStorage.getterCount;
    outputBuffer[5] = nestedStorage.setterCount;

    NestedMutableExistentialStorage readNestedStorage = {{MutableValue(51), 0, 0}, 0, 0};
    outputBuffer[6] = readNestedProperty(readNestedStorage);
    outputBuffer[7] = readNestedStorage.backing.getterCount;
    outputBuffer[8] = readNestedStorage.backing.setterCount;
    outputBuffer[9] = readNestedStorage.getterCount;
    outputBuffer[10] = readNestedStorage.setterCount;

    IndexedExistentialPropertyStorage indexedStorage = {IndexedValue(71, 0, 0), 0, 0};
    mutateIndexedProperty(indexedStorage);
    outputBuffer[11] = indexedStorage.backing.getValue();
    outputBuffer[12] = indexedStorage.backing.getGetterCount();
    outputBuffer[13] = indexedStorage.backing.getSetterCount();
    outputBuffer[14] = indexedStorage.getterCount;
    outputBuffer[15] = indexedStorage.setterCount;

    IMutableValueContainer mutableContainer =
        MutableValueContainer(MutableValue(81), 0, 0);
    mutateNestedIndexedValue(mutableContainer);
    outputBuffer[16] = mutableContainer.getStoredValue();
    outputBuffer[17] = mutableContainer.getGetterCount();
    outputBuffer[18] = mutableContainer.getSetterCount();

    GetterOnlyExistentialProjectionStorage getterOnlyStorage = {{MutableValue(91)}, 0};
    outputBuffer[19] = readGetterOnlyExistentialProjection(getterOnlyStorage);
    outputBuffer[20] = getterOnlyStorage.getterCount;

    MutableExistentialProjectionStorage readMutableProjectionStorage =
        {{MutableValue(101)}, 0, 0};
    outputBuffer[21] = readMutableExistentialProjection(readMutableProjectionStorage);
    outputBuffer[22] = readMutableProjectionStorage.getterCount;
    outputBuffer[23] = readMutableProjectionStorage.setterCount;

    MutableExistentialProjectionStorage mutableProjectionStorage = {{MutableValue(111)}, 0, 0};
    mutateExistentialProjection(mutableProjectionStorage);
    outputBuffer[24] = mutableProjectionStorage.backing.item.read();
    outputBuffer[25] = mutableProjectionStorage.getterCount;
    outputBuffer[26] = mutableProjectionStorage.setterCount;

}

// CHECK: type: int32_t
// CHECK-NEXT: 23
// CHECK-NEXT: 42
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 51
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 72
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 82
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 91
// CHECK-NEXT: 1
// CHECK-NEXT: 101
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 112
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// Lowering also uses the stabilizing local to snapshot an ordinary l-value. The opened payload,
// type, and witness must share the one load from the interface-valued field.
// IR-LABEL: [nameHint("readMember")]
// IR: [[ITEM_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(%IReadableValue) = get_field_addr
// IR: [[ITEM_VALUE:%[A-Za-z0-9_]+]] : %IReadableValue = load([[ITEM_ADDRESS]])
// IR-NOT: load([[ITEM_ADDRESS]])
// IR: return_val

// IR-LABEL: [nameHint("IMutableValue.increment")]
// IR: [[MUTABLE_VALUE_INCREMENT_KEY:%[A-Za-z0-9_]+]] : _ = key
// IR-LABEL: [nameHint("NestedMutableExistentialStorage.inner.get")]
// IR: func [[NESTED_GETTER:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("MutableExistentialPropertyStorage.item.get")]
// IR: func [[MUTABLE_GETTER:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("MutableExistentialPropertyStorage.item.set")]
// IR: func [[MUTABLE_SETTER:%[A-Za-z0-9_]+]] :

// Lowering shares the one outer getter result across the nested call's inner get/set and applies
// both setter write-backs from the inside out.
// IR-LABEL: [nameHint("NestedMutableExistentialStorage.inner.set")]
// IR: func [[NESTED_SETTER:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("mutateNestedProperty")]
// IR: call [[NESTED_GETTER]]
// IR-NOT: call [[NESTED_GETTER]]
// IR: call [[MUTABLE_GETTER]]
// IR-NOT: call [[MUTABLE_GETTER]]
// IR: call [[MUTABLE_SETTER]]
// IR-NOT: call [[MUTABLE_SETTER]]
// IR: call [[NESTED_SETTER]]
// IR-NOT: call [[NESTED_SETTER]]
// IR: return_val

// A nonmutating interface method still commits the mutating inner getter's receiver changes.
// IR-LABEL: [nameHint("readNestedProperty")]
// IR: call [[NESTED_GETTER]]
// IR-NOT: call [[NESTED_GETTER]]
// IR: call [[MUTABLE_GETTER]]
// IR-NOT: call [[MUTABLE_GETTER]]
// IR-NOT: call [[MUTABLE_SETTER]]
// IR: [[READ_METHOD:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness
// IR-NOT: call [[NESTED_SETTER]]
// IR: call [[READ_METHOD]]
// IR-NOT: call [[MUTABLE_SETTER]]
// IR: call [[NESTED_SETTER]]
// IR-NOT: call [[NESTED_SETTER]]
// IR: return_val

// Lowering uses one concrete payload for both accessors of the property-qualified subscript.
// IR-LABEL: [nameHint("IIndexedValue.operator[].get")]
// IR: [[INDEXED_GETTER_KEY:%[A-Za-z0-9_]+]] : _ = key
// IR-LABEL: [nameHint("IIndexedValue.operator[].set")]
// IR: [[INDEXED_SETTER_KEY:%[A-Za-z0-9_]+]] : _ = key
// The checked access path retains the selected subscript while lowering carries its pending
// write-back through the deferred accessor.
// IR-LABEL: [nameHint("IndexedExistentialPropertyStorage.item.get")]
// IR: func [[INDEXED_PROPERTY_GETTER:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("IndexedExistentialPropertyStorage.item.set")]
// IR: func [[INDEXED_PROPERTY_SETTER:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("mutateIndexedProperty")]
// IR: call [[INDEXED_PROPERTY_GETTER]]
// IR-NOT: call [[INDEXED_PROPERTY_GETTER]]
// IR: [[INDEXED_SETTER:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness({{.*}}, [[INDEXED_SETTER_KEY]])
// IR: [[INDEXED_GETTER:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness({{.*}}, [[INDEXED_GETTER_KEY]])
// IR: call [[INDEXED_GETTER]]
// IR-NOT: call [[INDEXED_GETTER]]
// IR: call [[INDEXED_SETTER]]
// IR-NOT: call [[INDEXED_SETTER]]
// IR: call [[INDEXED_PROPERTY_SETTER]]
// IR-NOT: call [[INDEXED_PROPERTY_SETTER]]
// IR: return_val

// Lowering preserves both nested binding write-backs while opening the interface-valued subscript
// result.
// IR-LABEL: [nameHint("IMutableValueContainer.operator[].get")]
// IR: [[MUTABLE_CONTAINER_GETTER_KEY:%[A-Za-z0-9_]+]] : _ = key
// IR-LABEL: [nameHint("IMutableValueContainer.operator[].set")]
// IR: [[MUTABLE_CONTAINER_SETTER_KEY:%[A-Za-z0-9_]+]] : _ = key
// IR-LABEL: [nameHint("mutateNestedIndexedValue")]
// IR: [[MUTABLE_CONTAINER_SETTER:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness({{.*}}, [[MUTABLE_CONTAINER_SETTER_KEY]])
// IR: [[MUTABLE_CONTAINER_GETTER:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness({{.*}}, [[MUTABLE_CONTAINER_GETTER_KEY]])
// IR: call [[MUTABLE_CONTAINER_GETTER]]
// IR-NOT: call [[MUTABLE_CONTAINER_GETTER]]
// IR: [[MUTABLE_VALUE_INCREMENT:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness({{.*}}, [[MUTABLE_VALUE_INCREMENT_KEY]])
// IR: call [[MUTABLE_VALUE_INCREMENT]]
// IR: call [[MUTABLE_CONTAINER_SETTER]]
// IR-NOT: call [[MUTABLE_CONTAINER_SETTER]]
// IR: return_val

