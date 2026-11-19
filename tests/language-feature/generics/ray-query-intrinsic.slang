//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -profile sm_6_9

// A user-written intrinsic declaration already describes the native numeric operands.
// Only the standard-library RayQuery projects a feature type into native flags.
__intrinsic_type(RayQuery)
struct NativeQuery<let rayFlags : uint, let queryFlags : uint>
{
    [mutating]
    bool proceed()
    {
        __target_switch
        {
        case hlsl: __intrinsic_asm ".Proceed";
        }
    }
}

RWStructuredBuffer<uint> output;

// CHECK: RayQuery<2{{[uU]? *, *}}1{{[uU]? *}}>
// CHECK: .Proceed(
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    NativeQuery<2, 1> query;
    output[0] = query.proceed();
}
