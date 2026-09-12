// enum-case-attribute-wrong-target.slang

// `EnumCase` is intentionally distinct from `Var`: a `Var`-only attribute is not
// applicable on an enum case.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl

[__AttributeUsage(_AttributeTargets.Var)]
struct VarOnlyAttribute
{
    int value;
}

enum LogLevel
{
    [VarOnly(1)] Verbose,
    Info,
}
//CHECK: invalid attribute placement
//CHECK: attribute 'VarOnly' is not valid here

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    LogLevel level = LogLevel.Info;
}
