//TEST:SIMPLE(filecheck=NEGATIVE_CHECK_1): -target spirv -stage compute -entry computeMain -DNEGATIVE_CHECK_1

// NEGATIVE_CHECK_1 tests that the compile fails since `FromType` cannot be converted to `ToType` given the
// type information the user provided to the function. More `where` constraints would be needed to get
// this code to compile: super-type constraints or type-coerce-constraint.

struct CustomStruct1
{
    int data = 1;
    int getData()
    {
        return data;
    }
}
struct CustomStruct2
{
    int data = 1;
    __init(CustomStruct1 other)
    {
        data = 2;
    }
    int getData()
    {
        return data;
    }
}

RWStructuredBuffer<int> outputBuffer;

// NEGATIVE_CHECK_1-DAG: note:
// NEGATIVE_CHECK_1-DAG: {{.*}}:[[# @LINE+1]]:
ToType castableConstraint<ToType, FromType>() where ToType(FromType)
{
    return ToType();
}

ToType logic<ToType, FromType>()
{
    // NEGATIVE_CHECK_1-DAG: error[E38043]:
    // NEGATIVE_CHECK_1-DAG-NEXT: {{.*}}:[[# @LINE+1]]:
    return castableConstraint<ToType, FromType>();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 1
    var t = logic<CustomStruct2, CustomStruct1>();
    outputBuffer[0] = t.getData();
}