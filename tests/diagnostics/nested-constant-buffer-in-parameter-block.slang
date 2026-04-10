//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal

struct T
{
    RWStructuredBuffer<int> buf;
}

struct U
{
    ConstantBuffer<T> t;
}

struct S
{
    ConstantBuffer<RWStructuredBuffer<int>> cb;
}

ParameterBlock<S> s1;
//CHECK:          ^^ ConstantBuffer with resource types in ParameterBlock not supported on Metal
//CHECK:          ^^ nesting a 'ConstantBuffer' containing resource types inside a 'ParameterBlock' is not supported on Metal, please use 'ParameterBlock' instead.

ParameterBlock<U> s2;
//CHECK:          ^^ ConstantBuffer with resource types in ParameterBlock not supported on Metal
//CHECK:          ^^ nesting a 'ConstantBuffer' containing resource types inside a 'ParameterBlock' is not supported on Metal, please use 'ParameterBlock' instead.

ParameterBlock<ConstantBuffer<RWStructuredBuffer<int>>> s3;
//CHECK:                                                ^^ ConstantBuffer with resource types in ParameterBlock not supported on Metal
//CHECK:                                                ^^ nesting a 'ConstantBuffer' containing resource types inside a 'ParameterBlock' is not supported on Metal, please use 'ParameterBlock' instead.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void kernelMain()
{
    outputBuffer[0] = s1.cb[0] + s2.t.buf[0] + s3[0];
}
