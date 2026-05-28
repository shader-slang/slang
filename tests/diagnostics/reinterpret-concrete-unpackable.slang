// Verify that `lowerReinterpret` still diagnoses concrete string types
// that cannot be packed into AnyValue. The unresolved-generic skip in that
// pass must not hide user-facing errors for live concrete reinterprets.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    String value = "hello";
    outBuffer[0] = reinterpret<int>(value);
//CHECK:                           ^ type cannot be packed for dynamic dispatch
//CHECK:                           ^ type 'string' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
}
