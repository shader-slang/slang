//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// The test intentionally uses concrete struct inheritance to exercise an inherited-subobject
// projection, so the unstable-inheritance warning is not relevant to the result.
#pragma warning(disable:30816)

struct BaseValue
{
    int baseField;
}

struct DerivedValue : BaseValue
{
    int derivedField;
}

struct ProjectionStorage
{
    DerivedValue readValue;
    DerivedValue writtenValue;

    property DerivedValue item
    {
        get { return readValue; }
        set { writtenValue = newValue; }
    }
}

void writeDirectField(inout ProjectionStorage storage)
{
    // Lowering reads the complete property value, changes a field declared directly on that value,
    // and passes the result to the setter.
    storage.item.derivedField = 13;
}

void writeInheritedField(inout ProjectionStorage storage)
{
    // Member lookup inserts the concrete `BaseValue` subobject before selecting `baseField`.
    // Lowering preserves both projections while it reads, updates, and writes the property value.
    storage.item.baseField = 23;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    ProjectionStorage directStorage = {};
    directStorage.readValue.baseField = 11;
    directStorage.readValue.derivedField = 12;
    writeDirectField(directStorage);
    outputBuffer[0] = directStorage.writtenValue.baseField;
    outputBuffer[1] = directStorage.writtenValue.derivedField;

    ProjectionStorage inheritedStorage = {};
    inheritedStorage.readValue.baseField = 21;
    inheritedStorage.readValue.derivedField = 22;
    writeInheritedField(inheritedStorage);
    outputBuffer[2] = inheritedStorage.writtenValue.baseField;
    outputBuffer[3] = inheritedStorage.writtenValue.derivedField;
}

// CHECK: type: int32_t
// CHECK-NEXT: 11
// CHECK-NEXT: 13
// CHECK-NEXT: 23
// CHECK-NEXT: 22

// Both projection shapes must finish by writing the complete updated value through the setter.
// IR-LABEL: [nameHint("ProjectionStorage.item.set")]
// IR: func [[SETTER:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("writeDirectField")]
// IR: call [[SETTER]]
// IR-LABEL: [nameHint("writeInheritedField")]
// IR: call [[SETTER]]
