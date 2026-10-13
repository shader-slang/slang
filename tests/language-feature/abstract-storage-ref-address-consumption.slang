//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RUNTIME): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// While forming a writable `Ref<T>` from a user-defined subscript, lowering must preserve the
// address returned by its `ref` accessor. In particular, the deferred subscript arguments must not
// make lowering copy the selected value into a temporary and return the temporary's address.

struct RefSubscriptStorage
{
    int values[2];

    __subscript(int index) -> int
    {
        [mutating]
        ref { return values[index]; }
    }
}

struct NestedRefStorage
{
    RefSubscriptStorage storage;

    property int first
    {
        [mutating]
        ref { return storage[0]; }
    }
}

Ref<int> getSubscriptReference(inout RefSubscriptStorage storage, int index)
{
    return storage[index];
}

Ref<int> getNestedReference(inout NestedRefStorage storage)
{
    return storage.first;
}

void overwrite(inout int value, int replacement)
{
    value = replacement;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    RefSubscriptStorage directStorage;
    directStorage.values[1] = 20;
    overwrite(getSubscriptReference(directStorage, 1), 41);

    NestedRefStorage nestedStorage;
    nestedStorage.storage.values[0] = 10;
    overwrite(getNestedReference(nestedStorage), 31);

    outputBuffer[0] = directStorage.values[1];
    outputBuffer[1] = nestedStorage.storage.values[0];
}

// RUNTIME: type: int32_t
// RUNTIME-NEXT: 41
// RUNTIME-NEXT: 31

// IR-LABEL: [nameHint("RefSubscriptStorage.operator[].ref")]
// IR: let [[SUBSCRIPT_ELEMENT_ADDRESS:%[A-Za-z0-9_]+]] : {{.*}} = getElementPtr
// IR-NOT: load([[SUBSCRIPT_ELEMENT_ADDRESS]])
// IR: return_val([[SUBSCRIPT_ELEMENT_ADDRESS]])

// IR-LABEL: [nameHint("getSubscriptReference")]
// IR: let [[DIRECT_REFERENCE:%[A-Za-z0-9_]+]] : Ptr(Int) = call
// IR-NOT: load([[DIRECT_REFERENCE]])
// IR-NOT: var
// IR-NOT: store
// IR: return_val([[DIRECT_REFERENCE]])

// IR-LABEL: [nameHint("NestedRefStorage.first.ref")]
// IR: let [[NESTED_SUBSCRIPT_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(Int) = call
// IR-NOT: load([[NESTED_SUBSCRIPT_ADDRESS]])
// IR-NOT: var
// IR-NOT: store
// IR: return_val([[NESTED_SUBSCRIPT_ADDRESS]])

// IR-LABEL: [nameHint("getNestedReference")]
// IR: let [[NESTED_REFERENCE:%[A-Za-z0-9_]+]] : Ptr(Int) = call
// IR-NOT: load([[NESTED_REFERENCE]])
// IR-NOT: var
// IR-NOT: store
// IR: return_val([[NESTED_REFERENCE]])
