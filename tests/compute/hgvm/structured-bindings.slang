//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct Sample
{
    bool enabled;
    uint value;
}

struct DispatchArguments
{
    bool enabled;
    float2 weights;
    Array<Sample, 2> samples;
    float2x3 transform;
    RWStructuredBuffer<uint> output;
}

struct ResourceElement
{
    RWStructuredBuffer<uint> output;
    uint value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void inspectArguments(DispatchArguments arguments)
{
    arguments.output[0] = arguments.enabled ? 1 : 0;
    arguments.output[1] = uint(arguments.weights.x * 10.0 + arguments.weights.y);
    arguments.output[2] = arguments.samples[0].enabled ? arguments.samples[0].value : 0;
    arguments.output[3] = arguments.samples[1].enabled ? arguments.samples[1].value : 0;
    arguments.output[4] = uint(arguments.transform[0][2] + arguments.transform[1][0]);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeResourceArray(Array<ResourceElement, 2> elements)
{
    elements[0].output[0] = elements[0].value;
    elements[1].output[0] = elements[1].value;
}

[host]
void main()
{
    RWStructuredBuffer<uint> output = RWStructuredBuffer<uint>::alloc(5);

    Sample first;
    first.enabled = true;
    first.value = 23;

    Sample second;
    second.enabled = false;
    second.value = 99;

    DispatchArguments arguments;
    arguments.enabled = true;
    arguments.weights = float2(4.0, 2.0);
    arguments.samples[0] = first;
    arguments.samples[1] = second;
    arguments.transform = float2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    arguments.output = output;

    dispatch(inspectArguments, uint3(1, 1, 1))(arguments);
    print(readBack(output, 0));
    print(readBack(output, 1));
    print(readBack(output, 2));
    print(readBack(output, 3));
    print(readBack(output, 4));

    RWStructuredBuffer<uint> firstOutput = RWStructuredBuffer<uint>::alloc(1);
    RWStructuredBuffer<uint> secondOutput = RWStructuredBuffer<uint>::alloc(1);

    ResourceElement firstResource;
    firstResource.output = firstOutput;
    firstResource.value = 71;

    ResourceElement secondResource;
    secondResource.output = secondOutput;
    secondResource.value = 83;

    Array<ResourceElement, 2> resourceElements;
    resourceElements[0] = firstResource;
    resourceElements[1] = secondResource;
    dispatch(writeResourceArray, uint3(1, 1, 1))(resourceElements);
    print(readBack(firstOutput, 0));
    print(readBack(secondOutput, 0));
}

// CHECK: 1
// CHECK-NEXT: 42
// CHECK-NEXT: 23
// CHECK-NEXT: 0
// CHECK-NEXT: 7
// CHECK-NEXT: 71
// CHECK-NEXT: 83
