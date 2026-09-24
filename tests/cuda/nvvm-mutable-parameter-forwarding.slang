//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[3 7], stride=4):name=inputBuffer
StructuredBuffer<int> inputBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[noinline]
void reset(out int value, int seed)
{
    value = seed;
}

[noinline]
void update(inout int value)
{
    // A mutable borrow can be forwarded to an output parameter without changing its storage.
    reset(value, value + 2);
}

struct Payload
{
    int values[2];
    double weight;

    [mutating]
    [noinline]
    void setLayer(int seed)
    {
        update(values[0]);
        values[1] += seed;
        weight += double(values[0]);
    }
};

[noinline]
void initialize(out Payload result, int seed)
{
    result.values[0] = seed;
    result.values[1] = 10;
    result.weight = 1.0;
    // This is the material's out-result to mutating-this call boundary.
    result.setLayer(seed);
    result.setLayer(1);
}

[noinline]
void replace(inout Payload result, int seed)
{
    // BorrowInOut -> Out, then Out -> BorrowInOut in initialize.
    initialize(result, seed);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Payload value;
    initialize(value, inputBuffer[0]);
    outputBuffer[0] = value.values[0]; // 3 + 2 + 2
    outputBuffer[1] = value.values[1]; // 10 + 3 + 1
    outputBuffer[2] = int(value.weight); // 1 + 5 + 7
    replace(value, inputBuffer[1]);
    outputBuffer[3] = value.values[0] + value.values[1] + int(value.weight); // 11 + 18 + 21
}

// CHECK: 7
// CHECK-NEXT: 14
// CHECK-NEXT: 13
// CHECK-NEXT: 50
