//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

T readAt<T, TArray : IArray<T>>(TArray values, int index)
{
    return values[index];
}

RWBuffer<float> values;

float test()
{
    return readAt<float, RWBuffer<float>>(values, 0);
//CHECK:   ^^^^^^ type argument doesn't conform to interface
//CHECK:   ^^^^^^ type argument 'RWBuffer<float>' does not conform to the required interface 'IArray<float>'
}
