//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl
// non-exhaustive: overload resolution also emits candidate notes.

enum TargetValue
{
    [require(hlsl)]
    hlslOnly = 0,
    [require(spirv)]
    spirvOnly = 1,
}

TargetValue castRuntime(int value)
{
    return TargetValue(value);
//CHECK: enum integer conversion may require case capabilities
//CHECK: enum cast requires a compile-time constant case
}

TargetValue castUnknownConstant()
{
    return (TargetValue)42;
//CHECK: enum integer conversion may require case capabilities
//CHECK: enum cast has no matching case
}

T fromInteger<T : ILogical>(int value)
{
    return T(value);
}

TargetValue castGeneric(int value)
{
    return fromInteger<TargetValue>(value);
//CHECK: type argument 'TargetValue' does not conform to the required interface 'ILogical'
}

T fromTag<T : __EnumType>(T.__Tag value)
{
    return T(value);
}

TargetValue castGenericTag(int value)
{
    return fromTag<TargetValue>(value);
//CHECK: type argument 'TargetValue' does not conform to the required interface '__EnumType'
}

// Ordinary enums retain both direct and generic integer construction.
enum Ordinary { zero, one }

Ordinary ordinaryCasts(int value)
{
    Ordinary direct = Ordinary(value);
    Ordinary logical = fromInteger<Ordinary>(value);
    Ordinary tagged = fromTag<Ordinary>(value);
    return direct | logical | tagged;
}

// The runtime restriction also applies when the target supports every declared case.
enum HlslValue
{
    [require(hlsl)]
    supported = 0,
}

HlslValue castRuntimeOnSupportedTarget(int value)
{
    return HlslValue(value);
//CHECK: enum integer conversion may require case capabilities
//CHECK: enum cast requires a compile-time constant case
}
