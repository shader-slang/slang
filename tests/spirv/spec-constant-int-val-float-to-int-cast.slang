//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -entry computeMain -stage compute -target spirv

// CHECK: error[E38041]
// CHECK: const float X

[[SpecializationConstant]]
const float X = 10.0;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain() : SV_Target
{
    float arr[int(X)];
    float a = arr[0];
}
