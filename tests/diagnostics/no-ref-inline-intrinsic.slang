// The companion test `no-ref-inline-not-intrinsic.slang` covers the rejection
// of `[noRefInline]` on ordinary functions. This one guards the accepted
// shape: the attribute on an intrinsic declaration, whose `__target_switch` +
// `__intrinsic_asm` body never emits a target function definition, must stay
// legal so the core module's mesh-shading intrinsics (SetMeshOutputCounts,
// DispatchMesh) keep compiling.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> buf;

[noRefInline]
void intrinsicBump(__ref int x)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "InterlockedAdd($0, 1)";
    case spirv:
        return spirv_asm
        {
            OpCapability Shader;
        };
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int v = buf[0];
    intrinsicBump(v);
    buf[2] = v;
}

// CHECK: InterlockedAdd({{.*}}, 1);
