//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// Semantic checking tracks a constant index into a swizzle as a projection of the original
// l-value. Passing that projection to `__constref` reads the property once; it does not modify the
// property and must not require or call a setter. The getter-only case also confirms that forming
// the read-only projection does not require a writable accessor.

static int getterSetterGetterCount;
static int getterSetterSetterCount;
static int getterOnlyGetterCount;

struct GetterSetterStorage
{
    property int4 item
    {
        [nonmutating]
        get
        {
            getterSetterGetterCount++;
            return int4(11, 12, 13, 14);
        }

        [nonmutating]
        set
        {
            getterSetterSetterCount++;
        }
    }
}

struct GetterOnlyStorage
{
    property int4 item
    {
        [nonmutating]
        get
        {
            getterOnlyGetterCount++;
            return int4(21, 22, 23, 24);
        }
    }
}

int readBorrowedScalar(__constref int value)
{
    return value;
}

int readGetterSetterProjection(GetterSetterStorage storage)
{
    return readBorrowedScalar(storage.item.xy[0]);
}

int readGetterOnlyProjection(GetterOnlyStorage storage)
{
    return readBorrowedScalar(storage.item.xy[0]);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    getterSetterGetterCount = 0;
    getterSetterSetterCount = 0;
    getterOnlyGetterCount = 0;

    GetterSetterStorage getterSetterStorage = {};
    outputBuffer[0] = readGetterSetterProjection(getterSetterStorage);
    outputBuffer[1] = getterSetterGetterCount;
    outputBuffer[2] = getterSetterSetterCount;

    GetterOnlyStorage getterOnlyStorage = {};
    outputBuffer[3] = readGetterOnlyProjection(getterOnlyStorage);
    outputBuffer[4] = getterOnlyGetterCount;
}

// CHECK: type: int32_t
// CHECK-NEXT: 11
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 21
// CHECK-NEXT: 1

// IR-LABEL: [nameHint("GetterSetterStorage.item.get")]
// IR: func [[GET_GETTER_SETTER:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readGetterSetterProjection")]
// IR: call [[GET_GETTER_SETTER]]
// IR-NOT: call [[GET_GETTER_SETTER]]
// IR-NOT: call {{.*GetterSetterStoragex5Fitemx5Fset}}
// IR: return_val
// IR-LABEL: [nameHint("GetterOnlyStorage.item.get")]
// IR: func [[GET_GETTER_ONLY:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readGetterOnlyProjection")]
// IR: call [[GET_GETTER_ONLY]]
// IR-NOT: call [[GET_GETTER_ONLY]]
// IR: return_val
