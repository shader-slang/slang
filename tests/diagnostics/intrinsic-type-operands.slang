//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -no-codegen

uint runtimeFlags;

// Runtime expressions must be diagnosed before an intrinsic type reaches lowering.
// CHECK: error[E39999]: expression does not evaluate to a compile-time constant
__intrinsic_type(RayQuery, runtimeFlags, 0)
struct InvalidQuery {}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    InvalidQuery query;
}
