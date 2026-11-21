//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl
// non-exhaustive: conformance failures include requirement and declaration notes.

// Explicitly requesting a constructor-bearing interface must not synthesize a runtime cast.
enum LogicalValue : ILogical
//CHECK: type 'LogicalValue' does not provide required interface member '$init'
{
    [require(hlsl)]
    hlslOnly = 0,
}

enum RawValue : __EnumType
//CHECK: type 'RawValue' does not provide required interface member '$init'
{
    [require(hlsl)]
    hlslOnly = 0,
}
