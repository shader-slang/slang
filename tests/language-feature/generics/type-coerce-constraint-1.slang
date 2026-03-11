//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1,non-exhaustive): -target spirv -stage compute -entry computeMain -DNEGATIVE_CHECK_1
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2,non-exhaustive): -target spirv -stage compute -entry computeMain -DNEGATIVE_CHECK_2

RWStructuredBuffer<int> outputBuffer;

// CHECK1 tests that the compile fails since `ToType` (MyStruct1) does not have an
// implicit conversion into `FromType` (MyStruct2).
//
// CHECK2 tests that the compile fails since `ToType` (MyStruct1) does not have a
// conversion into `FromType` (MyStruct2).

#ifdef NEGATIVE_CHECK_1
ToType castableConstraint1<ToType, FromType>(FromType input) where ToType(FromType) implicit
#else
ToType castableConstraint1<ToType, FromType>(FromType input) where ToType(FromType)
#endif
{
    return {};
}

struct CustomStruct1
{
    int data;
    __init(int other)
    {
        data = other;
    }
}

struct CustomStruct2
{
    int data;

#ifdef NEGATIVE_CHECK_1
    __init(CustomStruct1 input)
    {
        data = input.data;
    }
#endif

    __init(int other)
    {
        data = other;
    }
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;
    outputBuffer[0] = (true && castableConstraint1<CustomStruct2, CustomStruct1>(CustomStruct1(1)).data == 1) ? 1 : 0;
//CHECK1: implicit conversion constraint not satisfied
//CHECK2: type coerce constraint not satisfied
}