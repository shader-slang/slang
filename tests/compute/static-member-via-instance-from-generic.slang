// A plain `static` member function called through an instance from a
// generic body must not get the instance as an implicit `this` argument,
// unlike `__fwd_diff(obj.method)` on a non-static method.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)

interface IBase
{
    static float helper(float t);
}

struct Foo : IBase
{
    static float helper(float t)
    {
        return 2.0 * t;
    }
}

float runStatic<T : IBase>(T obj, float t)
{
    return obj.helper(t);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;

    // CHECK: 4.0
    outputBuffer[0] = runStatic<Foo>(f, 2.0);

    // CHECK: 6.0
    outputBuffer[1] = runStatic<Foo>(f, 3.0);
}
