// Negative companion for https://github.com/shader-slang/slang/issues/14059: the
// common-type join gives `nullptr` the type of a pointer arm only. A `nullptr`
// paired with a non-pointer, and two different pointer types, still fail with a
// front-end diagnostic, and a generic whose `T` must conform to an interface
// rejects the joined pointer type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

interface IFoo
{
    int get();
}

struct S
{
    int a;
}

int pickFoo<T : IFoo>(bool c, T a, T b)
{
    return c ? a.get() : b.get();
}

RWStructuredBuffer<int> outputBuffer;
uniform int* ip;
uniform float* fp;

[numthreads(1, 1, 1)]
void computeMain()
{
    bool c = outputBuffer[0] != 0;
    S s = { 1 };

    let a = c ? ip : fp;
    let b = c ? 1 : nullptr;
    let d = select(c, 1.0f, nullptr);
    let e = c ? s : nullptr;
    outputBuffer[1] = pickFoo(c, ip, nullptr);
}

// The errors follow source order.
//CHECK-NOT: E99999
//CHECK: E39999
//CHECK-SAME: no overload for '?:' applicable to arguments of type (bool, Ptr<int,{{.*}}>, Ptr<float,
//CHECK: E30019
//CHECK: got 'NullPtr'
//CHECK: E39999
//CHECK-SAME: no overload for 'select' applicable to arguments of type (bool, float, NullPtr)
//CHECK: E39999
//CHECK-SAME: no overload for '?:' applicable to arguments of type (bool, S, NullPtr)
//CHECK: E38029
//CHECK-SAME: type argument doesn't conform to interface
//CHECK-NOT: E99999
