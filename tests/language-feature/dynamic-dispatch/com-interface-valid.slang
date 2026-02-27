// Valid COM interface patterns: basic declaration, COM-to-COM inheritance,
// and multiple method signatures. These compile without errors, confirming
// that the COM validation rejects only invalid usage.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

[COM("00000000-0000-0000-0000-000000000001")]
interface ICOMBase
{
    float getValue();
    void setValue(float v);
}

[COM("00000000-0000-0000-0000-000000000002")]
interface ICOMDerived : ICOMBase
{
    float compute(float x);
}

// CHECK-NOT: error 31122
// CHECK-NOT: error 31123
// CHECK-NOT: error 31124
// CHECK-NOT: error

[numthreads(1, 1, 1)]
void computeMain()
{
}
