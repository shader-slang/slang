//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

// Test that interface variables work correctly when embedded in structs

interface IProcessor
{
    float compute(float x);
}

struct AddProcessor : IProcessor
{
    float addValue;
    float compute(float x) { return x + addValue; }
}

struct MulProcessor : IProcessor
{
    float mulValue;
    float compute(float x) { return x * mulValue; }
}

// Test 1: Struct containing single interface member
struct ProcessorContainer
{
    IProcessor processor;
    float baseValue;
}

// Test 2: Struct containing multiple interface members
struct DualProcessorContainer
{
    IProcessor first;
    IProcessor second;
    float multiplier;
}

// Test 3: Nested struct with interface member
struct NestedContainer
{
    ProcessorContainer inner;
    int id;
}

ProcessorContainer createContainer(uint id, float baseVal)
{
    ProcessorContainer c;
    c.baseValue = baseVal;
    if (id == 0)
        c.processor = AddProcessor(10.0);
    else
        c.processor = MulProcessor(2.0);
    return c;
}

DualProcessorContainer createDualContainer(uint id1, uint id2, float mult)
{
    DualProcessorContainer c;
    c.multiplier = mult;
    if (id1 == 0)
        c.first = AddProcessor(5.0);
    else
        c.first = MulProcessor(3.0);
    if (id2 == 0)
        c.second = AddProcessor(2.0);
    else
        c.second = MulProcessor(0.5);
    return c;
}

NestedContainer createNestedContainer(uint id, float baseVal, int containerId)
{
    NestedContainer n;
    n.inner = createContainer(id, baseVal);
    n.id = containerId;
    return n;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint idx = 0;

    // Test 1: Single interface in struct
    {
        ProcessorContainer c0 = createContainer(0, 100.0);
        outputBuffer[idx++] = c0.processor.compute(5.0);  // AddProcessor: 5 + 10 = 15
        // CHECK: 15.0

        ProcessorContainer c1 = createContainer(1, 200.0);
        outputBuffer[idx++] = c1.processor.compute(5.0);  // MulProcessor: 5 * 2 = 10
        // CHECK: 10.0

        // Test reassignment of struct with interface member
        c0 = createContainer(1, 300.0);
        outputBuffer[idx++] = c0.processor.compute(3.0);  // MulProcessor: 3 * 2 = 6
        // CHECK: 6.0

        // Test using baseValue from struct
        outputBuffer[idx++] = c0.baseValue;
        // CHECK: 300.0
    }

    // Test 2: Multiple interface members in struct
    {
        DualProcessorContainer dc = createDualContainer(0, 1, 2.0);
        float result1 = dc.first.compute(10.0);   // AddProcessor: 10 + 5 = 15
        float result2 = dc.second.compute(10.0);  // MulProcessor: 10 * 0.5 = 5
        outputBuffer[idx++] = result1 + result2;  // 15 + 5 = 20
        // CHECK: 20.0

        DualProcessorContainer dc2 = createDualContainer(1, 0, 3.0);
        result1 = dc2.first.compute(4.0);   // MulProcessor: 4 * 3 = 12
        result2 = dc2.second.compute(4.0);  // AddProcessor: 4 + 2 = 6
        outputBuffer[idx++] = result1 + result2;  // 12 + 6 = 18
        // CHECK: 18.0
    }

    // Test 3: Nested struct with interface member
    {
        NestedContainer nc = createNestedContainer(0, 50.0, 123);
        outputBuffer[idx++] = nc.inner.processor.compute(7.0);  // AddProcessor: 7 + 10 = 17
        // CHECK: 17.0

        outputBuffer[idx++] = nc.inner.baseValue;
        // CHECK: 50.0

        outputBuffer[idx++] = float(nc.id);
        // CHECK: 123.0

        NestedContainer nc2 = createNestedContainer(1, 75.0, 456);
        outputBuffer[idx++] = nc2.inner.processor.compute(8.0);  // MulProcessor: 8 * 2 = 16
        // CHECK: 16.0
    }

    // Test 4: Copy struct with interface member
    {
        ProcessorContainer c = createContainer(0, 25.0);
        ProcessorContainer c2 = c;  // Copy struct containing interface
        float result = c2.processor.compute(5.0) + c2.baseValue;  // (5 + 10) + 25 = 40
        outputBuffer[idx++] = result;
        // CHECK: 40.0
    }

    // Test 5: Struct containing interface in array
    {
        ProcessorContainer containers[2];
        containers[0] = createContainer(0, 1.0);
        containers[1] = createContainer(1, 2.0);

        float sum = containers[0].processor.compute(3.0);  // 3 + 10 = 13
        sum += containers[1].processor.compute(3.0);       // 3 * 2 = 6
        outputBuffer[idx++] = sum;  // 13 + 6 = 19
        // CHECK: 19.0
    }
}
