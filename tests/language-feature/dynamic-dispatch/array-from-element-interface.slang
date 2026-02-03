//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test MakeArrayFromElement with interface types
// 

public interface ITerm 
{
    float evaluate(float u);
}

public struct Multiply : ITerm
{
    float factor;
    public __init(float factor) { this.factor = factor; }
    public float evaluate(float u) { return u * factor; }
}

public struct Add : ITerm
{
    float offset;
    public __init(float offset) { this.offset = offset; }
    public float evaluate(float u) { return u + offset; }
}

float sumArray<int N>(Array<ITerm, N> terms, float u)
{
    float v = 0.f;
    for (var i = 0; i < N; i++)
        v += terms[i].evaluate(u);
    return v;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID) {

    // Test 1: Create array from single interface-typed element
    ITerm term1 = Multiply(3.f);
    Array<ITerm, 4> array1 = makeArrayFromElement<ITerm, 4>(term1);
    
    // 3 * 0.5 * 4 elements = 6.0
    outputBuffer[0] = sumArray(array1, 0.5); // CHECK: 6

    // Test 2: Create array from another interface implementation
    ITerm term2 = Add(2.f);
    Array<ITerm, 3> array2 = makeArrayFromElement<ITerm, 3>(term2);
    
    // (0.5 + 2) * 3 elements = 7.5
    outputBuffer[1] = sumArray(array2, 0.5); // CHECK: 7.5

    // Test 3: Verify different element counts work
    Array<ITerm, 5> array3 = makeArrayFromElement<ITerm, 5>(Multiply(2.f));
    
    // 2 * 0.5 * 5 elements = 5.0
    outputBuffer[2] = sumArray(array3, 0.5); // CHECK: 5
}
