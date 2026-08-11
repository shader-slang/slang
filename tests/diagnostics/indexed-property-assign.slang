//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain

// Regression test for https://github.com/shader-slang/slang/issues/7160
// Assigning to an indexed element of an array-returning property should
// produce a diagnostic instead of crashing with an internal error.

struct Foo
{
    float a;
    float _b[2];

    property float b[2]
    {
        get { return _b; }
        set
        {
            _b[0] = newValue[0];
            _b[1] = newValue[1];
        }
    }
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Foo c;
    c.b[0] = 1.0;
//CHECK:   ^ assignment target is not supported
//CHECK:   ^ this form of assignment is not currently supported; consider assigning to the whole value instead of an individual element
    output[0] = c.b[0];
}
