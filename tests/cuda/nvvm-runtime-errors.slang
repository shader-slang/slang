// clang-format off
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// CHECK: {{^5$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^2$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^2$}}
// CHECK-NEXT: {{^13$}}
// CHECK-NEXT: {{^14$}}
// CHECK-NEXT: {{^4$}}

//TEST_INPUT:ubuffer(data=[2.5 3.5 1.0 4.0], stride=4):name=floatInput
StructuredBuffer<float> floatInput;
//TEST_INPUT:ubuffer(data=[1 3 4 2], stride=4):name=intInput
StructuredBuffer<int> intInput;
//TEST_INPUT:ubuffer(data=[0x12345678 0x12345679 0x1234567A 0x1234567B 0x1234567C 0x1234567D 0x1234567E 0x1234567F], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Keep manual formatting below: clang-format does not understand Slang throws clauses.

enum GenericError
{
    Fail = 1
};

// Runtime-loaded values exercise both the return path and the rethrown enum payload.
[noinline]
T genericRethrow<T : __BuiltinFloatingPointType>(T value) throws GenericError
{
    do
    {
        if (value >= T(3))
            throw GenericError.Fail;
        return value * T(2);
    }
    catch (error : GenericError)
    {
        throw error;
    }
}

struct AggregateError
{
    int code = 0;
    int param = 0;
};

// Both fields of the aggregate error must survive the noinline call.
[noinline]
int aggregateThrow(int value) throws AggregateError
{
    if (value >= 3)
        throw AggregateError(1, value);
    return value * 2;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint lane = dispatchThreadID.x;
    do
    {
        outputBuffer[lane] = int(try genericRethrow(floatInput[lane]));
    }
    catch (error : GenericError)
    {
        outputBuffer[lane] = reinterpret<int>(error);
    }
    do
    {
        outputBuffer[4 + lane] = try aggregateThrow(intInput[lane]);
    }
    catch (error : AggregateError)
    {
        outputBuffer[4 + lane] = error.code * 0x10 + error.param;
    }
}
// clang-format on
