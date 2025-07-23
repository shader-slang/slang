//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

enum A
{
    V1,
    V2,
    V3
}

enum B
{
    V1,
    V2,
    V3
}

struct Foo<T, A access = A::V1, B addrSpace = B::V1>
{
    int a;
    __init(int b) { a = b; }
    __init<U, A accessOther>(Foo<U, accessOther, addrSpace> ptr)
    {
    }

    // internally Slang is throwing an error, we just don't drain the error
    // from our sink
    __init(uint64_t val) {}
    __init(int64_t val) {}
}

extension int64_t
{
    __init<T, A access, B addrSpace>(Foo<T, access, addrSpace> t) {}
}
extension uint64_t
{
    __init<T, A access, B addrSpace>(Foo<T, access, addrSpace> t) {}
}

RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    Foo<int, A.V1, B.V1> v1 = Foo<int, A.V1, B.V1>(1);

    // CHECK: ([[# @LINE+1]]): error 30080
    Foo<float, A.V1, B.V2> v2 = Foo<float, A.V1, B.V2>(v1);

    output[0] = v1.a;
}
