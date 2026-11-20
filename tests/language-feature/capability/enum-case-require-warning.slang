//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target hlsl -entry main -stage compute

enum TargetValue
{
    [require(hlsl)]
    hlslOnly = 0,
    [require(spirv)]
    spirvOnly = 1,
}

TargetValue fromTag<let tag : int>()
{
    return TargetValue(tag);
//CHECK: enum integer conversion may require case capabilities
//CHECK: integer conversion to capability-restricted enum 'TargetValue' may introduce a case capability requirement; use a named case when possible
}

enum Ordinary { zero }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = int(TargetValue.hlslOnly);
    // Reusing a specialization does not warn again; the integer conversion is in fromTag.
    outputBuffer[4] = int(fromTag<0>()) + int(fromTag<0>());
    // Ordinary enum casts do not introduce case requirements.
    outputBuffer[5] = int(Ordinary(0));
    outputBuffer[1] = int(TargetValue(0));
//CHECK: enum integer conversion may require case capabilities
//CHECK: integer conversion to capability-restricted enum 'TargetValue' may introduce a case capability requirement; use a named case when possible
    static const int tag = 0;
    outputBuffer[2] = int((TargetValue)tag);
//CHECK: enum integer conversion may require case capabilities
//CHECK: integer conversion to capability-restricted enum 'TargetValue' may introduce a case capability requirement; use a named case when possible
    // The inner conversion warns once; the outer identity conversion must not warn again.
    outputBuffer[3] = int(TargetValue(TargetValue(0)));
//CHECK: enum integer conversion may require case capabilities
//CHECK: integer conversion to capability-restricted enum 'TargetValue' may introduce a case capability requirement; use a named case when possible
}
