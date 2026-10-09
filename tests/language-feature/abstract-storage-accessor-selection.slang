//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// The test intentionally uses concrete struct inheritance to exercise an inherited-subobject
// projection, so the unstable-inheritance warning is not relevant to the result.
#pragma warning(disable:30816)

// The accessor used for a property depends on the operation performed on the property result.
// The runtime cases use distinct getter values and setter records to reveal the selected accessor.
// The IR cases verify the address-producing `ref` path and the materialized `__constref` path
// directly.

// `IndexedPropertyStorage` stores different values for its `get` and `ref` accessors and also
// defines a setter.
struct IndexedPropertyStorage
{
    int2 getterValue;
    int2 refValue;

    property int2 item
    {
        get { return getterValue; }
        set {}

        [mutating]
        ref { return refValue; }
    }
}

void increment(inout int value)
{
    value++;
}

int readX(__constref int2 value)
{
    return value.x;
}

// Forms an indexed `inout` argument from a property that has all three accessor kinds.
void exerciseIndexedInOut()
{
    IndexedPropertyStorage storage;
    storage.getterValue = int2(31, 32);
    storage.refValue = int2(41, 42);

    // An `inout` argument requires the address of the indexed element. A getter/setter pair can
    // write back a whole property value, but it cannot provide the required element address, so
    // lowering must call `ref` for this operation.
    increment(storage.item[0]);
}

int exerciseWholePropertyBorrowIn(IndexedPropertyStorage storage)
{
    // A copyable property can satisfy a `__constref` parameter by materializing the value returned
    // by `get`. The argument must not select `ref` merely because `__constref` eventually passes an
    // address to the callee.
    return readX(storage.item);
}

Ref<int2> getWholePropertyReference(inout IndexedPropertyStorage storage)
{
    // A writable explicit reference requires the address supplied by the property's `ref`
    // accessor. The getter/setter path can replace a complete value but cannot supply that address.
    return storage.item;
}

int readWholePropertyThroughReadOnlyReference(inout IndexedPropertyStorage storage)
{
    // A read-only explicit reference borrows a materialized property value. It must observe the
    // getter result rather than request the distinct storage exposed by `ref`.
    Ref<int2, Access.Read> reference = storage.item;
    return reference.x;
}

// `BaseValue` and `DerivedValue` form the concrete inherited subobject projected below.
struct BaseValue
{
    int baseField;
}

struct DerivedValue : BaseValue
{
    int derivedField;
}

// The `DerivedPropertyStorage.item` setter records the complete value after a projected write.
struct DerivedPropertyStorage
{
    DerivedValue getterValue;
    DerivedValue lastSetValue;
    int setterCallCount;

    property DerivedValue item
    {
        get { return getterValue; }

        set
        {
            lastSetValue = newValue;
            setterCallCount++;
        }
    }
}

// The `VectorPropertyStorage.item` setter records the complete vector after a swizzled write.
struct VectorPropertyStorage
{
    int4 getterValue;
    int4 lastSetValue;
    int setterCallCount;

    property int4 item
    {
        get { return getterValue; }

        set
        {
            lastSetValue = newValue;
            setterCallCount++;
        }
    }
}

// The `MatrixPropertyStorage.item` setter records the complete matrix after a projected write.
struct MatrixPropertyStorage
{
    int2x2 getterValue;
    int2x2 lastSetValue;
    int setterCallCount;

    property int2x2 item
    {
        get { return getterValue; }

        set
        {
            lastSetValue = newValue;
            setterCallCount++;
        }
    }
}

// A whole-property `out` argument should use the setter without evaluating the getter or `ref`
// accessor. Each accessor records its invocation so the test distinguishes all three choices.
struct OutPropertyStorage
{
    int2 getterValue;
    int2 refValue;
    int2 lastSetValue;
    int getterCallCount;
    int setterCallCount;
    int refCallCount;

    property int2 item
    {
        [mutating]
        get
        {
            getterCallCount++;
            return getterValue;
        }

        [mutating]
        set
        {
            lastSetValue = newValue;
            setterCallCount++;
        }

        [mutating]
        ref
        {
            refCallCount++;
            return refValue;
        }
    }
}

// A swizzled assignment through a `ref`-only property must evaluate its accessor exactly once.
struct RefOnlyVectorValue
{
    int4 elements;
}

static RefOnlyVectorValue refOnlyVectorValue = {};

struct RefOnlyVectorPropertyStorage
{
    property RefOnlyVectorValue item
    {
        [mutating]
        ref
        {
            return refOnlyVectorValue;
        }
    }
}

void exerciseRefOnlySwizzledAssignment()
{
    RefOnlyVectorPropertyStorage storage;

    // Address formation refines the property to a pointer-backed field and swizzle. Assignment
    // must retain that partially refined value instead of calling the `ref` accessor again.
    storage.item.elements.xy = int2(135, 136);
}

void overwrite(out int2 value)
{
    value = int2(121, 122);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IndexedPropertyStorage readStorage;
    readStorage.getterValue = int2(11, 12);
    readStorage.refValue = int2(21, 22);

    // Reading an indexed element first reads the property's value. The read must therefore call
    // `get`, even though a `ref` accessor could provide an address for the same element.
    outputBuffer[0] = readStorage.item[0];
    outputBuffer[1] = readStorage.refValue[0];

    DerivedPropertyStorage derivedStorage = {};
    derivedStorage.getterValue.baseField = 51;
    derivedStorage.getterValue.derivedField = 61;
    derivedStorage.setterCallCount = 0;

    // Member lookup represents the inherited field access by inserting the concrete `BaseValue`
    // subobject between the property and `baseField`. The subobject is part of the property value,
    // so lowering reads the complete value, updates the base subobject, and passes the resulting
    // `DerivedValue` to the setter.
    derivedStorage.item.baseField = 53;
    outputBuffer[2] = derivedStorage.lastSetValue.baseField;
    outputBuffer[3] = derivedStorage.lastSetValue.derivedField;
    outputBuffer[4] = derivedStorage.setterCallCount;

    VectorPropertyStorage vectorStorage = {};
    vectorStorage.getterValue = int4(71, 72, 73, 74);
    vectorStorage.setterCallCount = 0;

    // The literal index selects one component of the writable `.xy` swizzle. Lowering can preserve
    // that update by reading the complete property value and passing the modified vector to `set`.
    vectorStorage.item.xy[0] = 75;
    outputBuffer[5] = vectorStorage.lastSetValue.x;
    outputBuffer[6] = vectorStorage.lastSetValue.y;
    outputBuffer[7] = vectorStorage.lastSetValue.z;
    outputBuffer[8] = vectorStorage.setterCallCount;

    MatrixPropertyStorage matrixStorage = {};
    matrixStorage.getterValue._m00 = 81;
    matrixStorage.getterValue._m01 = 82;
    matrixStorage.getterValue._m10 = 83;
    matrixStorage.getterValue._m11 = 84;
    matrixStorage.setterCallCount = 0;

    // A matrix-element projection participates in whole-value getter/setter writeback.
    matrixStorage.item._m00 = 85;
    outputBuffer[9] = matrixStorage.lastSetValue._m00;
    outputBuffer[10] = matrixStorage.lastSetValue._m01;
    outputBuffer[11] = matrixStorage.lastSetValue._m10;
    outputBuffer[12] = matrixStorage.lastSetValue._m11;
    outputBuffer[13] = matrixStorage.setterCallCount;

    OutPropertyStorage outStorage;
    outStorage.getterValue = int2(111, 112);
    outStorage.refValue = int2(113, 114);
    outStorage.getterCallCount = 0;
    outStorage.setterCallCount = 0;
    outStorage.refCallCount = 0;

    // A pure `out` operation produces a complete replacement value, so it calls only `set`.
    overwrite(outStorage.item);
    outputBuffer[14] = outStorage.lastSetValue.x;
    outputBuffer[15] = outStorage.lastSetValue.y;
    outputBuffer[16] = outStorage.getterCallCount;
    outputBuffer[17] = outStorage.setterCallCount;
    outputBuffer[18] = outStorage.refCallCount;

    // The whole-property `__constref` case must observe the getter value, 11, rather than the
    // value available through `ref`, 21.
    outputBuffer[19] = exerciseWholePropertyBorrowIn(readStorage);

    // A read-only explicit reference follows the same value-access rule.
    outputBuffer[20] = readWholePropertyThroughReadOnlyReference(readStorage);

}

// CHECK: type: int32_t
// CHECK-NEXT: 11
// CHECK-NEXT: 21
// CHECK-NEXT: 53
// CHECK-NEXT: 61
// CHECK-NEXT: 1
// CHECK-NEXT: 75
// CHECK-NEXT: 72
// CHECK-NEXT: 73
// CHECK-NEXT: 1
// CHECK-NEXT: 85
// CHECK-NEXT: 82
// CHECK-NEXT: 83
// CHECK-NEXT: 84
// CHECK-NEXT: 1
// CHECK-NEXT: 121
// CHECK-NEXT: 122
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 11
// CHECK-NEXT: 11

// IR-LABEL: [nameHint("IndexedPropertyStorage.item.get")]
// IR: func [[ITEM_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readX")]
// IR: func [[READ_X:%[A-Za-z0-9_]+]] :

// A whole-property `__constref` argument materializes the value returned by `get` and passes the
// resulting temporary to `readX`. It does not use the property's `ref` accessor.
// IR-LABEL: [nameHint("exerciseWholePropertyBorrowIn")]
// IR-NOT: call %IndexedPropertyStoragex5Fitemx5Fref
// IR: call [[ITEM_GET]]
// IR-NOT: call %IndexedPropertyStoragex5Fitemx5Fref
// IR: call [[READ_X]]
// IR-NOT: call %IndexedPropertyStoragex5Fitemx5Fref
// IR: return_val

// A read-only explicit reference materializes the property's getter result and borrows the
// resulting temporary. Forming this reference must not request the storage exposed by `ref`.
// IR-LABEL: [nameHint("readWholePropertyThroughReadOnlyReference")]
// IR-NOT: call %IndexedPropertyStoragex5Fitemx5Fref
// IR: call [[ITEM_GET]]
// IR-NOT: call %IndexedPropertyStoragex5Fitemx5Fref
// IR: return_val

// IR-LABEL: [nameHint("IndexedPropertyStorage.item.set")]
// IR: func [[ITEM_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("IndexedPropertyStorage.item.ref")]
// IR: func [[ITEM_REF:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("increment")]
// IR: func [[INCREMENT:%[A-Za-z0-9_]+]] :

// The indexed `inout` argument needs an address, so the lowered call obtains the property's `ref`
// result instead of using the getter/setter writeback path. It then calculates the address of
// element zero.
// IR-LABEL: [nameHint("exerciseIndexedInOut")]
// IR-NOT: call [[ITEM_GET]]
// IR-NOT: call [[ITEM_SET]]
// IR: let [[PROPERTY_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(Vec(Int, 2 : Int)) = call [[ITEM_REF]]
// IR-NOT: call [[ITEM_GET]]
// IR-NOT: call [[ITEM_SET]]
// IR: let [[ELEMENT_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(Int{{.*}}) =
// IR-SAME: getElementPtr([[PROPERTY_ADDRESS]], 0 : Int)
// IR-NOT: call [[ITEM_GET]]
// IR-NOT: call [[ITEM_SET]]
// IR: call [[INCREMENT]]([[ELEMENT_ADDRESS]])
// IR-NOT: call [[ITEM_GET]]
// IR-NOT: call [[ITEM_SET]]
// IR: return_val

// Forming an explicit writable reference requests the whole property's address from `ref`.
// IR-LABEL: [nameHint("getWholePropertyReference")]
// IR-NOT: call [[ITEM_GET]]
// IR-NOT: call [[ITEM_SET]]
// IR: call [[ITEM_REF]]
// IR-NOT: call [[ITEM_GET]]
// IR-NOT: call [[ITEM_SET]]
// IR: return_val

// A swizzled assignment through a `ref`-only property calls the accessor once, retains the
// resulting address through the field projection, and uses it as the base of the swizzled store.
// IR-LABEL: [nameHint("RefOnlyVectorPropertyStorage.item.ref")]
// IR: func [[REF_ONLY_ITEM_REF:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("exerciseRefOnlySwizzledAssignment")]
// IR-NOT: call [[REF_ONLY_ITEM_REF]]
// IR: let [[REF_ONLY_STORAGE:%[A-Za-z0-9_]+]] : Ptr({{.*}}) = call [[REF_ONLY_ITEM_REF]]
// IR-NOT: call [[REF_ONLY_ITEM_REF]]
// IR: swizzledStore
// IR-NOT: call [[REF_ONLY_ITEM_REF]]
// IR: return_val
