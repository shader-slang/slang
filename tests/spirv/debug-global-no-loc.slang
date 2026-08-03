//TEST:SIMPLE(filecheck=VARS):-target spirv-assembly -emit-spirv-directly -g2 -entry main -stage hull
//TEST:SIMPLE(filecheck=NODBG):-target spirv-assembly -emit-spirv-directly -g2 -entry main -stage hull

// Scalarizing a `struct`-typed varying gives each field its own interface global.
// Those per-field globals are not variables the user declared, so they must not be
// reported as debug globals. The interface variables themselves are still emitted.

// VARS-DAG: OpName %patch_x "patch.x"
// VARS-DAG: OpName %entryPointParam_main_x "entryPointParam_main.x"

// NODBG-NOT: OpExtInst {{.*}} Debug{{Global}}Variable

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
