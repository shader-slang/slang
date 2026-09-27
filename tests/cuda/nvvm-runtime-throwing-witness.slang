// clang-format off
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// CHECK: {{^110$}}
// CHECK-NEXT: {{^4$}}
// CHECK-NEXT: {{^B$}}
// CHECK-NEXT: {{^110$}}

//TEST_INPUT:ubuffer(data=[-1 0 7 -3], stride=4):name=inputBuffer
StructuredBuffer<int> inputBuffer;
//TEST_INPUT:ubuffer(data=[0x12345678 0x12345679 0x1234567A 0x1234567B], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Keep manual formatting below: clang-format does not understand Slang throws clauses.

enum WitnessError
{
    Fail = 0x10
};

interface IFoo
{
    [mutating] int dosomething<T>() throws WitnessError;
};

struct MyFoo : IFoo
{
    int value;

    // Calling this nonmutating implementation through IFoo requires a mutating witness adapter.
    [noinline]
    int dosomething<T>() throws WitnessError
    {
        if (value < 0)
            throw WitnessError.Fail;
        return value + int(sizeof(T));
    }
};

// The catch offset makes the result depend on the error tag as well as its payload.
int helper<T : IFoo>(T receiver)
{
    do
    {
        return try receiver.dosomething<int>();
    }
    catch (error : WitnessError)
    {
        return 0x100 + reinterpret<int>(error);
    }
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint lane = dispatchThreadID.x;
    MyFoo receiver;
    receiver.value = inputBuffer[lane];
    outputBuffer[lane] = helper(receiver);
}
// clang-format on
