//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target hlsl -entry main -stage compute

enum TargetValue
{
    [require(hlsl)]
    hlslOnly = 0,
    [require(spirv)]
    spirvOnly = 1,
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = int(TargetValue.hlslOnly);
    outputBuffer[1] = int(TargetValue(0));
//CHECK: enum constant cast inherits case capability requirements
//CHECK: this conversion selects case 'hlslOnly' and inherits its capability requirements
    static const int tag = 0;
    outputBuffer[2] = int((TargetValue)tag);
//CHECK: enum constant cast inherits case capability requirements
//CHECK: this conversion selects case 'hlslOnly' and inherits its capability requirements
    // The inner conversion warns once; the outer identity conversion must not warn again.
    outputBuffer[3] = int(TargetValue(TargetValue(0)));
//CHECK: enum constant cast inherits case capability requirements
//CHECK: this conversion selects case 'hlslOnly' and inherits its capability requirements
}
