//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=OUT): -cpu -source-language hlsl -output-using-type
//TEST:SIMPLE_EX(filecheck=IR): -lang hlsl tests/language-feature/hlsl-default-this-param-mode-writeback.slang -no-codegen -dump-ir

// A method with HLSL's default writable receiver participates in ordinary property and subscript
// writeback. When both a getter and setter are present, a whole-value call reads through the getter
// and writes the modified value through the setter. When no setter exists, the whole-value call
// instead uses a writable `ref` accessor.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct Counter
{
    int value;

    void increment()
    {
        value++;
    }
};

struct Storage
{
    Counter propertyGetterValue;
    Counter subscriptGetterValue;
    Counter subscriptRefValue;
    int propertySetterValue;

    property Counter item
    {
        get { return propertyGetterValue; }
        set { propertySetterValue = newValue.value; }
    }

    __subscript(uint index) -> Counter
    {
        get { return subscriptGetterValue; }
        [mutating]
        ref { return subscriptRefValue; }
    }
};

void mutateStorage(inout Storage storage)
{
    storage.item.increment();
    storage[0].increment();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Storage storage = {};
    storage.propertyGetterValue.value = 10;
    storage.subscriptGetterValue.value = 30;
    storage.subscriptRefValue.value = 20;
    mutateStorage(storage);

    outputBuffer[0] = storage.propertySetterValue;
    outputBuffer[1] = storage.subscriptRefValue.value;
}

// OUT: type: int32_t
// OUT-NEXT: 11
// OUT-NEXT: 21

// The IR for `mutateStorage` must write the property receiver back through its setter and pass the
// address from the subscript's `ref` accessor to the writable receiver of `Counter.increment`.
// IR-LABEL: [nameHint("Storage.item.get")]
// IR: func [[PROPERTY_GET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("Counter.increment")]
// IR-NEXT: func [[INCREMENT:%[A-Za-z0-9_]+]] : Func(Void, BorrowInOutParam(%Counter))
// IR-LABEL: [nameHint("Storage.item.set")]
// IR: func [[PROPERTY_SET:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("Storage.operator[].ref")]
// IR: func [[SUBSCRIPT_REF:%[A-Za-z0-9_]+]] :
// IR-SAME: Func(Ptr(%Counter), BorrowInOutParam(%Storage), UInt)
// IR-LABEL: [nameHint("mutateStorage")]
// IR: let [[PROPERTY_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(%Counter) = var
// IR: [[PROPERTY_VALUE:%[A-Za-z0-9_]+]] : %Counter = call [[PROPERTY_GET]]({{.*}})
// IR: store([[PROPERTY_ADDRESS]], [[PROPERTY_VALUE]])
// IR: call [[INCREMENT]]([[PROPERTY_ADDRESS]])
// IR: [[UPDATED_PROPERTY_VALUE:%[A-Za-z0-9_]+]] : %Counter = load([[PROPERTY_ADDRESS]])
// IR: call [[PROPERTY_SET]]({{.*}}, [[UPDATED_PROPERTY_VALUE]])
// IR: let [[SUBSCRIPT_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(%Counter) =
// IR-SAME: call [[SUBSCRIPT_REF]]({{.*}}, 0 : UInt)
// IR: call [[INCREMENT]]([[SUBSCRIPT_ADDRESS]])
