//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type

// Empty declarations introduce no conformance requirements.
interface IEmpty { ; }
struct Empty : IEmpty {}

int useEmpty<T : IEmpty>(T value)
{
    return 7;
}

interface IProperty
{
    property float3 X { get; };
}

struct Property : IProperty
{
    float3 storage;
    property float3 X { get { return storage; } }
}

interface IDerived : IProperty { ;; }
struct Derived : IDerived
{
    float3 storage;
    property float3 X { get { return storage; } }
}

float3 readProperty<T : IProperty>(T value)
{
    return value.X;
}

interface IGeneric<T>
{
    ;
    property T value { get; };
}

struct Generic : IGeneric<int>
{
    int storage;
    property int value { get { return storage; } }
}

int readGeneric<T : IGeneric<int>>(T value)
{
    return value.value;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Empty empty;
    outputBuffer[0] = useEmpty(empty);

    // Access through interface constraints to exercise the conformance witnesses.
    Property property = { float3(11, 12, 13) };
    outputBuffer[1] = dot(readProperty(property), float3(1, 1, 1));

    Derived derived = { float3(21, 22, 23) };
    outputBuffer[2] = dot(readProperty(derived), float3(1, 1, 1));

    Generic generic = { 31 };
    outputBuffer[3] = readGeneric(generic);
}

// CHECK: 7.0
// CHECK-NEXT: 36.0
// CHECK-NEXT: 66.0
// CHECK-NEXT: 31.0
