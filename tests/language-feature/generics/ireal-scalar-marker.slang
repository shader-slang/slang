// Verifies the scalar-only markers `IScalarReal` and `IScalarFloatingPoint` reject a vector argument
// — their whole purpose is to accept scalar floats but not `vector`/`matrix`. A generic constrained
// to either marker, instantiated at a vector, must fail conformance (E38029), guarding against a
// regression where `vector` accidentally conforms. Non-exhaustive: we assert only the conformance
// errors, not the cascading "see declaration" notes.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cpp -stage compute -entry computeMain

T onlyScalarReal<T : IScalarReal>(T x)
{
    return x.sqrt();
}

T onlyScalarFP<T : IScalarFloatingPoint>(T x)
{
    return x.rcp();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    //CHECK: type argument 'vector<float,3>' does not conform to the required interface 'IScalarReal'
    outputBuffer[0] = onlyScalarReal(float3(1, 2, 3)).x;
    //CHECK: type argument 'vector<float,3>' does not conform to the required interface 'IScalarFloatingPoint'
    outputBuffer[1] = onlyScalarFP(float3(1, 2, 3)).x;
}
