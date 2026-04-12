//TEST:SIMPLE(filecheck=NEGATIVE_CHECK): -target spirv -stage compute -entry computeMain

// NEGATIVE_CHECK tests that we emit an error when a user uses a concrete type as a ToType for a type coercion constraint
// and this ToType does not have its declaration in the immediate parent generic argument list

RWStructuredBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

struct CustomStruct1
{
    int data = 1;
    __init(int other)
    {
        data = other;
    }
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
        data = other.getData()+inputBuffer[1];
    }
    int getData()
    {
        return data;
    }
}

CustomStruct2 logic<FromType>(FromType dataIn)
    where CustomStruct2(FromType)
{
    // NEGATIVE_CHECK-DAG: error[E38044]:
    // NEGATIVE_CHECK-DAG: {{.*}}:[[# @LINE+1]]:
    return CustomStruct2(dataIn);
}

struct CustomStruct3<T>
{
    // NEGATIVE_CHECK-DAG: error[E38044]:
    // NEGATIVE_CHECK-DAG: {{.*}}:[[# @LINE+1]]:
    void convert<>() where CustomStruct1(int)
    {

    }

    // NEGATIVE_CHECK-DAG: error[E38044]:
    // NEGATIVE_CHECK-DAG: {{.*}}:[[# @LINE+1]]:
    void convert<>() where T(int)
    {

    }
}

interface IAssociatedTypeStruct<T>
{
    associatedtype U;
    // NEGATIVE_CHECK-DAG: error[E38044]:
    // NEGATIVE_CHECK-DAG: {{.*}}:[[# @LINE+1]]:
    associatedtype T where U(float);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int data = inputBuffer[0];
    CustomStruct1 dataIn = CustomStruct1(data);
    var t = logic(dataIn);
    outputBuffer[0] = t.getData();
}