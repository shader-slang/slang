//TEST:SIMPLE(filecheck=NEGATIVE_CHECK): -target spirv -stage compute -entry computeMain

// NEGATIVE_CHECK tests that Associated type fails to compile as expected with Type Coercion Constraints
// due to non-conforming type

// TEST_INPUT:ubuffer(data=[1 2 3 0], stride=4)name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

// TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface MaybeCastInt
{
    int getData();
}

struct MaybeCastIntImpl1 : MaybeCastInt
{
    int data;
    int getData()
    {
        return data;
    }
}
struct MaybeCastIntImpl2 : MaybeCastInt
{
    int data;
    int getData()
    {
        return data;
    }
}

interface CustomStruct<U>
{
    associatedtype T
        where T : MaybeCastInt
        where T(U);
}

struct CustomStructImpl1<U> : CustomStruct<U>
{
    // NEGATIVE_CHECK-DAG: error[E38105]:
    // NEGATIVE_CHECK-DAG: {{.*}}:[[# @LINE+1]]:
    typealias T = MaybeCastIntImpl1;
}
struct CustomStructImpl2<U> : CustomStruct<U>
{
    typealias T = MaybeCastIntImpl2;
}

struct UncastableTarget
{
    int data1;
    double data2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = CustomStructImpl1<UncastableTarget>.T(inputBuffer[1]).getData();
}