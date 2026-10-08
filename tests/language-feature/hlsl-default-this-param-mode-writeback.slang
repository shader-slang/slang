//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=OUT): -cpu -source-language hlsl -output-using-type
//TEST:SIMPLE_EX(filecheck=IR): -lang hlsl tests/language-feature/hlsl-default-this-param-mode-writeback.slang -no-codegen -dump-ir

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct Counter
{
    int value;

    void increment()
    {
        value++;
    }
};

struct GetterSetterAndRefStorage
{
    Counter propertyGetterValue;
    Counter subscriptGetterValue;
    Counter propertyRefValue;
    Counter subscriptRefValue;
    int propertySetterValue;
    int subscriptSetterValue;

    property Counter item
    {
        get { return propertyGetterValue; }
        set { propertySetterValue = newValue.value; }
        [mutating]
        ref { return propertyRefValue; }
    }

    __subscript(uint index) -> Counter
    {
        get { return subscriptGetterValue; }
        set { subscriptSetterValue = newValue.value; }
        [mutating]
        ref { return subscriptRefValue; }
    }
};

struct GetterAndRefStorage
{
    Counter propertyGetterValue;
    Counter subscriptGetterValue;
    Counter propertyRefValue;
    Counter subscriptRefValue;

    property Counter item
    {
        get { return propertyGetterValue; }
        [mutating]
        ref { return propertyRefValue; }
    }

    __subscript(uint index) -> Counter
    {
        get { return subscriptGetterValue; }
        [mutating]
        ref { return subscriptRefValue; }
    }
};

void mutateGetterAndRefStorage(inout GetterAndRefStorage storage)
{
    storage.item.increment();
    storage[0].increment();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Counter counter = {40};
    counter.increment();
    counter.increment();

    // A whole property or subscript used as the receiver of a mutating method uses its getter and
    // setter when a setter is available. The competing `ref` targets therefore remain unchanged.
    GetterSetterAndRefStorage storage = {};
    storage.propertyGetterValue.value = 10;
    storage.subscriptGetterValue.value = 20;
    storage.item.increment();
    storage[0].increment();

    outputBuffer[0] = counter.value;
    outputBuffer[1] = storage.propertySetterValue;
    outputBuffer[2] = storage.subscriptSetterValue;
    outputBuffer[3] = storage.propertyRefValue.value;
    outputBuffer[4] = storage.subscriptRefValue.value;
}

// OUT: type: int32_t
// OUT-NEXT: 42
// OUT-NEXT: 11
// OUT-NEXT: 21
// OUT-NEXT: 0
// OUT-NEXT: 0

// The HLSL-default receiver for `Counter.increment` is writable. When no setter exists, the
// receiver path must therefore use the `ref` accessor instead of the getter. These checks prove
// that both the property and subscript calls receive pointers returned by their `ref` accessors.
// IR-LABEL: [nameHint("Counter.increment")]
// IR-NEXT: func [[INCREMENT:%[A-Za-z0-9_]+]] : Func(Void, BorrowInOutParam(%Counter))
// IR-LABEL: [nameHint("GetterAndRefStorage.item.ref")]
// IR: func [[PROPERTY_REF:%[A-Za-z0-9_]+]] :
// IR-SAME: Func(Ptr(%Counter), BorrowInOutParam(%GetterAndRefStorage))
// IR-LABEL: [nameHint("GetterAndRefStorage.operator[].ref")]
// IR: func [[SUBSCRIPT_REF:%[A-Za-z0-9_]+]] :
// IR-SAME: Func(Ptr(%Counter), BorrowInOutParam(%GetterAndRefStorage), UInt)
// IR-LABEL: [nameHint("mutateGetterAndRefStorage")]
// IR: let [[PROPERTY_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(%Counter) = call [[PROPERTY_REF]]({{.*}})
// IR: call [[INCREMENT]]([[PROPERTY_ADDRESS]])
// IR: let [[SUBSCRIPT_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(%Counter) =
// IR-SAME: call [[SUBSCRIPT_REF]]({{.*}}, 0 : UInt)
// IR: call [[INCREMENT]]([[SUBSCRIPT_ADDRESS]])
