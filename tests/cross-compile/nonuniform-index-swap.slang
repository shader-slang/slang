//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry main -stage fragment -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry main -stage fragment -O1 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment -O1 -validate-ir

// Check that swapping two nonuniform indices and then breaking out of a loop preserves
// both values in GLSL and HLSL. For count = 1 and uv.y > 0.5, the result must be (1, 0, 0, 1).
// The generated assignments must read both old values before overwriting either variable.
// CHECK-DAG: uint [[OLD_B:[A-Za-z_0-9]+]] = b_0;
// CHECK-DAG: uint [[OLD_A:[A-Za-z_0-9]+]] = a_0;
// CHECK: a_0 = {{(nonuniformEXT|NonUniformResourceIndex)}}([[OLD_B]]);
// CHECK: b_0 = {{(nonuniformEXT|NonUniformResourceIndex)}}([[OLD_A]]);

cbuffer Params
{
    uint count;
};
float4 main(float2 uv : TEXCOORD) : SV_Target
{
    uint a = 0, b = 1;
    for (uint k = 0; k < count; ++k)
    {
        uint oldA = a;
        a = b;
        b = oldA;
        if (uv.y > 0.5)
        {
            a = NonUniformResourceIndex(a);
            b = NonUniformResourceIndex(b);
            break;
        }
    }
    return float4(a, b, 0, 1);
}
