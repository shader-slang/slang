// The companion test `no-ref-inline-not-intrinsic.slang` covers the rejection
// of `[noRefInline]` on ordinary functions. This one guards the accepted
// shapes, all with a `__ref` parameter: the `__target_switch` +
// `__intrinsic_asm` body with a `return spirv_asm` arm is the shape the core
// module's mesh-shading intrinsics (`SetMeshOutputCounts`, `DispatchMesh`)
// rely on; the bare `__intrinsic_asm` body, the `__stage_switch` body, and
// the statement spelling of `spirv_asm` (parentheses allowed) are accepted
// by design. The uncalled definitions below exist to exercise the classifier;
// only `intrinsicBump` is called, so only its asm expansion reaches the
// output, and its `spirv` arm is there to match the accepted two-arm shape,
// not to be exercised by the `-target hlsl` run.

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

[noRefInline]
void plainAsmIntrinsic(__ref int x)
{
    __intrinsic_asm "InterlockedAdd($0, 1)";
}

[noRefInline]
void stageSwitchIntrinsic(__ref int x)
{
    __stage_switch
    {
    case vertex:
        __intrinsic_asm "InterlockedAdd($0, 1)";
    case fragment:
        __intrinsic_asm "InterlockedAdd($0, 1)";
    }
}

[noRefInline]
void spirvStatementIntrinsic(__ref int x)
{
    __target_switch
    {
    case spirv:
        spirv_asm
        {
            OpCapability Shader;
        };
    case metal:
        return (spirv_asm
        {
            OpCapability Shader;
        });
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
