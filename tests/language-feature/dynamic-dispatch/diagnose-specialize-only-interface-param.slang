// When a [Specialize] interface is used as a function parameter and the concrete
// type cannot be determined at the call site, the compiler must reject the program
// because dynamic dispatch would be required for a specialization-only interface.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -gnone

#lang slang 2025

[Specialize]
interface IProcessor
{
    float process(float x);
}

struct DoubleProcessor : IProcessor
{
    float process(float x) { return x * 2; }
}

struct SquareProcessor : IProcessor
{
    float process(float x) { return x * x; }
}

// This function accepts an existential IProcessor parameter.
// When the concrete type is unknown at the call site, the compiler
// would need dynamic dispatch to resolve the method call.
float applyProcessor(IProcessor proc, float val)
{
    return proc.process(val);
//CHECK:               ^ dynamic dispatch on specialize-only type
//CHECK:               ^ type 'IProcessor' is marked for specialization only, but dynamic dispatch is needed for the call.
}

IProcessor selectProcessor(uint id)
{
    if (id == 0)
        return DoubleProcessor();
    return SquareProcessor();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IProcessor proc = selectProcessor(dispatchThreadID.x);
    outputBuffer[0] = applyProcessor(proc, 5.0);
}
