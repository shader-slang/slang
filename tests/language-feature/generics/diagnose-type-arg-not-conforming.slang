//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Passing a type argument that does not conform to the required interface
// constraint produces error 38029.

interface ITransformable
{
    float transform(float x);
}

struct PlainData
{
    float value;
}

float applyTransform<T : ITransformable>(T obj, float x)
{
    return obj.transform(x);
}

float test()
{
    PlainData p = { 1.0 };
    return applyTransform<PlainData>(p, 1.0);
//CHECK:   ^^^^^^^^^^^^^^ type argument doesn't conform to interface
//CHECK:   ^^^^^^^^^^^^^^ type argument 'PlainData' does not conform to the required interface 'ITransformable'
}
