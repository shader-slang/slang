//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry computeMain -stage compute -target spirv

// CHECK: error 38041: cannot cast non-integer specialization
// CHECK-NEXT: const float X

[[SpecializationConstant]]
const float X = 10.0;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain() : SV_Target
{
    float arr[int(X)];
    float a = arr[0];
}
