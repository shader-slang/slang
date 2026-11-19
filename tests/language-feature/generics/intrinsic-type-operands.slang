//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -profile sm_6_9

interface IQueryFlags
{
    static const uint flags;
}

struct QueryFlags : IQueryFlags
{
    static const uint flags = 1;
}

// Explicit operands use the checked values, including associated constants and
// arithmetic on generic parameters. The feature type and its witness must not
// become operands of the intrinsic IR type.
__intrinsic_type(RayQuery, inputFlags + 1u, Features.flags)
struct ExplicitQuery<let inputFlags : uint, Features : IQueryFlags>
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
    ExplicitQuery<1, QueryFlags> query;
    output[0] = query.proceed();
}
