//TEST:SIMPLE(filecheck=NEGATIVE_CHECK_1): -target spirv -stage compute -entry computeMain -DNEGATIVE_CHECK_1
//TEST:SIMPLE(filecheck=NEGATIVE_CHECK_2): -target spirv -stage compute -entry computeMain -DNEGATIVE_CHECK_2

RWStructuredBuffer<int> outputBuffer;

// NEGATIVE_CHECK_1 tests that the compile fails since `ToType` (MyStruct1) does not have an 
// implicit conversion into `FromType` (MyStruct2).
//
// NEGATIVE_CHECK_2 tests that the compile fails since `ToType` (MyStruct1) does not have a
// conversion into `FromType` (MyStruct2).

#ifdef NEGATIVE_CHECK_1
// NEGATIVE_CHECK_1-DAG: ([[# @LINE+1]]): note:
ToType castableConstraint1<ToType, FromType>(FromType input) where ToType(FromType) implicit
#else
// NEGATIVE_CHECK_2-DAG: ([[# @LINE+1]]): note:
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
    // NEGATIVE_CHECK_1-DAG: ([[# @LINE+1]]): note:
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
    outputBuffer[0] = (true
                 // NEGATIVE_CHECK_1-DAG: ([[# @LINE+2]]): error 38042: 'CustomStruct1' is not implicitly convertible to 'CustomStruct2', not satisfying the type coerce constraint 'CustomStruct2(CustomStruct1)'
                 // NEGATIVE_CHECK_2-DAG: ([[# @LINE+1]]): error 38043: 'CustomStruct1' is not convertible to 'CustomStruct2', not satisfying the type coerce constraint 'CustomStruct2(CustomStruct1)'
                 && castableConstraint1<CustomStruct2, CustomStruct1>(CustomStruct1(1)).data == 1
                ) ? 1 : 0;
}