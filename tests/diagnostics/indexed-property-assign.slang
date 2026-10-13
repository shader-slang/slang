//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain

// Regression test for https://github.com/shader-slang/slang/issues/7160
// Assigning to an indexed element of an array-returning property requires the property's direct
// address. A property without a `ref` accessor must be rejected during semantic checking instead
// of reaching unsupported lowering.

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
/*CHECK:
      ^ property or subscript does not provide an address
      ^ cannot take a direct reference to this property because it has no `ref` accessor
*/
    output[0] = c.b[0];
}
