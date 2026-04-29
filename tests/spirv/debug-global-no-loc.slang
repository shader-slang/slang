//TEST:SIMPLE(filecheck=CHECK):-target spirv-assembly -emit-spirv-directly -g2 -entry main -stage hull

// Verify that compiler-generated tessellation interface variables without
// source locations do not produce DebugGlobalVariable entries.

// CHECK-NOT: DebugGlobalVariable {{.*}} "patch.x"
// CHECK-NOT: DebugGlobalVariable {{.*}} "entryPointParam_main.x"

struct V
{
    float4 p : SV_POSITION;
    float  x : TEXCOORD0;
};

[shader("hull")]
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(1)]
V main(InputPatch<V, 1> patch)
{
    return patch[0];
}
