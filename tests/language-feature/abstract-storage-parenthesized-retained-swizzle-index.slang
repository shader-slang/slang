//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// Semantic checking tracks the same projected storage through parentheses. Indexing
// `(storage.item.xy)` therefore uses the property's getter/setter path just like indexing the
// unparenthesized swizzle: a read calls only the getter, while an `inout` argument reads, modifies,
// and writes back once.

static int getterCount;
static int setterCount;
static int4 lastSetValue;

struct ParenthesizedStorage
{
    property int4 item
    {
        [nonmutating]
        get
        {
            getterCount++;
            return int4(11, 12, 13, 14);
        }

        [nonmutating]
        set
        {
            setterCount++;
            lastSetValue = newValue;
        }
    }
}

int readBorrowedScalar(__constref int value)
{
    return value;
}

void increment(inout int value)
{
    value++;
}

int readParenthesizedProjection(ParenthesizedStorage storage)
{
    return readBorrowedScalar((storage.item.xy)[1]);
}

void modifyParenthesizedProjection(ParenthesizedStorage storage)
{
    increment((storage.item.xy)[1]);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    ParenthesizedStorage storage = {};

    getterCount = 0;
    setterCount = 0;
    outputBuffer[0] = readParenthesizedProjection(storage);
    outputBuffer[1] = getterCount;
    outputBuffer[2] = setterCount;

    getterCount = 0;
    setterCount = 0;
    lastSetValue = int4(0);
    modifyParenthesizedProjection(storage);
    outputBuffer[3] = getterCount;
    outputBuffer[4] = setterCount;
    outputBuffer[5] = lastSetValue.y;
}

// CHECK: type: int32_t
// CHECK-NEXT: 12
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 13

// IR-LABEL: [nameHint("ParenthesizedStorage.item.get")]
// IR: func [[GET_ITEM:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("readParenthesizedProjection")]
// IR: call [[GET_ITEM]]
// IR-NOT: call {{.*ParenthesizedStoragex5Fitemx5Fset}}
// IR: return_val
// IR-LABEL: [nameHint("modifyParenthesizedProjection")]
// IR: call [[GET_ITEM]]
// IR-NOT: call [[GET_ITEM]]
// IR: call {{.*ParenthesizedStoragex5Fitemx5Fset}}
// IR-NOT: call [[GET_ITEM]]
// IR: return_val
