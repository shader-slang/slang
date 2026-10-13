//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -target spirv -entry computeMain -stage compute -no-codegen -dump-ir

// Semantic checking inserts an interface upcast while resolving an inherited requirement through
// a derived-interface existential. Lowering must keep the original existential as the write-back
// destination so that the inherited setter updates the payload and repacks it with the original
// derived-interface witness.

interface IBase
{
    property int value
    {
        get;
        set;
    }
}

interface IDerived : IBase
{}

struct Impl : IDerived
{
    int storage;

    property int value
    {
        get { return storage; }
        set { storage = newValue; }
    }
}

void mutate(inout IDerived value)
{
    value.value = 7;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IDerived value = Impl(1);
    mutate(value);
    outputBuffer[0] = value.value;
}

// CHECK: type: int32_t
// CHECK-NEXT: 7

// The inherited setter operates on the one opened payload. The updated payload must then be
// repacked with the original `IDerived` witness before the existential is stored back.
// IR-LABEL: [nameHint("IBase.value.set")]
// IR: [[SETTER_KEY:%[A-Za-z0-9_]+]] : _ = key
// IR-LABEL: [nameHint("mutate")]
// IR: [[SOURCE:%[A-Za-z0-9_]+]] : %IDerived = load([[SOURCE_ADDRESS:%[A-Za-z0-9_]+]])
// IR-NOT: load([[SOURCE_ADDRESS]])
// IR: [[DERIVED_WITNESS:%[A-Za-z0-9_]+]] : {{.*}} = extractExistentialWitnessTable([[SOURCE]])
// IR: [[PAYLOAD_TYPE:%[A-Za-z0-9_]+]] : Type = extractExistentialType([[SOURCE]])
// IR: [[PAYLOAD:%[A-Za-z0-9_]+]] : [[PAYLOAD_TYPE]] = extractExistentialValue([[SOURCE]])
// IR: [[PAYLOAD_ADDRESS:%[A-Za-z0-9_]+]] : Ptr([[PAYLOAD_TYPE]]) = var
// IR: store([[PAYLOAD_ADDRESS]], [[PAYLOAD]])
// IR: [[BASE_WITNESS:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness([[DERIVED_WITNESS]]
// IR: [[SETTER:%[A-Za-z0-9_]+]] : {{.*}} = lookupWitness([[BASE_WITNESS]], [[SETTER_KEY]])
// IR: call [[SETTER]]([[PAYLOAD_ADDRESS]], 7 : Int)
// IR: [[UPDATED_PAYLOAD:%[A-Za-z0-9_]+]] : [[PAYLOAD_TYPE]] = load([[PAYLOAD_ADDRESS]])
// IR: [[UPDATED_EXISTENTIAL:%[A-Za-z0-9_]+]] : %IDerived = makeExistential([[UPDATED_PAYLOAD]], [[DERIVED_WITNESS]])
// IR-NOT: load([[SOURCE_ADDRESS]])
// IR: store([[SOURCE_ADDRESS]], [[UPDATED_EXISTENTIAL]])
// IR: return_val
