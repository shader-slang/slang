// With `-validate-with-downstream-compiler`, a target that has no downstream compiler or validator
// is an error. `slangc` does not skip the validation silently, and `slang-test` does not remove
// the option for such a target.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target hpp -entry computeMain -stage compute -validate-with-downstream-compiler

// CHECK: error[E52015]
// CHECK: '-validate-with-downstream-compiler' cannot be used with target 'hpp'

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = 1.0;
}
