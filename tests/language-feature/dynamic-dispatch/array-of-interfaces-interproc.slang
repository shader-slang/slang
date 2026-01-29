//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test inter-procedural analysis with arrays of interface types
// 

public interface IOperation 
{
    float apply(float x);
}

public struct Scale : IOperation
{
    float factor;
    public __init(float factor) { this.factor = factor; }
    public float apply(float x) { return x * factor; }
}

public struct Offset : IOperation
{
    float delta;
    public __init(float delta) { this.delta = delta; }
    public float apply(float x) { return x + delta; }
}

public struct Negate : IOperation
{
    public __init() {}
    public float apply(float x) { return -x; }
}

// Test 1: Function taking array of interfaces as input parameter
float applyOperations<int N>(Array<IOperation, N> ops, float value)
{
    float result = value;
    for (int i = 0; i < N; i++)
        result = ops[i].apply(result);
    return result;
}

// Test 2: Function taking array of interfaces as out parameter
void createOperations(out Array<IOperation, 3> ops)
{
    ops[0] = Scale(2.0);
    ops[1] = Offset(5.0);
    ops[2] = Negate();
}

// Test 3: Function taking array of interfaces as inout parameter
void modifyOperations(inout Array<IOperation, 2> ops)
{
    // Replace second operation
    ops[1] = Scale(10.0);
}

// Test 4: Function returning array of interfaces indirectly via out parameter
void buildPipeline(float scaleFactor, out Array<IOperation, 4> pipeline)
{
    pipeline[0] = Scale(scaleFactor);
    pipeline[1] = Offset(1.0);
    pipeline[2] = Scale(0.5);
    pipeline[3] = Offset(-2.0);
}

// Test 5: Nested function calls with array of interfaces
float processWithPipeline<int N>(Array<IOperation, N> pipeline, float input)
{
    return applyOperations(pipeline, input);
}

// Test 6: Function that modifies and returns result via out parameter
void transformArray(Array<IOperation, 2> input, out float result, float value)
{
    result = applyOperations(input, value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID) {

    // Test 1: Pass array to function
    Array<IOperation, 3> ops1 = { Scale(2.0), Offset(3.0), Scale(0.5) };
    // (1.0 * 2.0 + 3.0) * 0.5 = 2.5
    outputBuffer[0] = applyOperations(ops1, 1.0); // CHECK: 2.5

    // Test 2: Out parameter
    Array<IOperation, 3> ops2;
    createOperations(ops2);
    // (1.0 * 2.0 + 5.0) * -1 = -7.0
    outputBuffer[1] = applyOperations(ops2, 1.0); // CHECK: -7

    // Test 3: Inout parameter
    Array<IOperation, 2> ops3 = { Scale(3.0), Offset(5.0) };
    modifyOperations(ops3);
    // 2.0 * 3.0 * 10.0 = 60.0
    outputBuffer[2] = applyOperations(ops3, 2.0); // CHECK: 60

    // Test 4: Build pipeline via out parameter
    Array<IOperation, 4> pipeline;
    buildPipeline(4.0, pipeline);
    // ((10.0 * 4.0 + 1.0) * 0.5 - 2.0) = ((40 + 1) * 0.5 - 2.0) = 18.5
    outputBuffer[3] = applyOperations(pipeline, 10.0); // CHECK: 18.5

    // Test 5: Nested function calls
    Array<IOperation, 2> ops5 = { Offset(10.0), Scale(2.0) };
    // (5.0 + 10.0) * 2.0 = 30.0
    outputBuffer[4] = processWithPipeline(ops5, 5.0); // CHECK: 30

    // Test 6: Transform with out parameter
    Array<IOperation, 2> ops6 = { Scale(5.0), Offset(-3.0) };
    float result;
    transformArray(ops6, result, 2.0);
    // 2.0 * 5.0 - 3.0 = 7.0
    outputBuffer[5] = result; // CHECK: 7

    // Test 7: Multiple calls with different array contents
    Array<IOperation, 2> opsA = { Scale(2.0), Offset(1.0) };
    Array<IOperation, 2> opsB = { Offset(1.0), Scale(2.0) };
    // (3.0 * 2.0 + 1.0) = 7.0
    outputBuffer[6] = applyOperations(opsA, 3.0); // CHECK: 7
    // (3.0 + 1.0) * 2.0 = 8.0
    outputBuffer[7] = applyOperations(opsB, 3.0); // CHECK: 8

    // Test 8: Array modification through inout and reuse
    Array<IOperation, 2> ops8 = { Scale(1.0), Scale(1.0) };
    modifyOperations(ops8);
    // 1.0 * 1.0 * 10.0 = 10.0
    outputBuffer[8] = applyOperations(ops8, 1.0); // CHECK: 10

    // Test 9: Chained out parameter calls
    Array<IOperation, 4> pipeline2;
    buildPipeline(3.0, pipeline2);
    // ((2.0 * 3.0 + 1.0) * 0.5 - 2.0) = ((6 + 1) * 0.5 - 2.0) = 1.5
    outputBuffer[9] = processWithPipeline(pipeline2, 2.0); // CHECK: 1.5
}
