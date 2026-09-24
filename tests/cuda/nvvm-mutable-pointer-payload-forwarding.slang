//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[3 7], stride=4):name=inputBuffer
uniform int* inputBuffer;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct IndirectPayload
{
    int* pointer;
    int bias;

    [mutating]
    [noinline]
    void advance()
    {
        pointer = pointer + 1;
        bias += 2;
    }
};

[noinline]
void initializeIndirect(out IndirectPayload result, int* source)
{
    result.pointer = source;
    result.bias = 1;
    result.advance();
}

[noinline]
void replaceIndirect(inout IndirectPayload result, int* source)
{
    initializeIndirect(result, source);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IndirectPayload indirect;
    initializeIndirect(indirect, inputBuffer);
    outputBuffer[0] = *indirect.pointer + indirect.bias; // 7 + 3
    replaceIndirect(indirect, inputBuffer);
    outputBuffer[1] = *indirect.pointer + indirect.bias; // 7 + 3
}

// CHECK: 10
// CHECK-NEXT: 10
