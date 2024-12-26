//TEST:SIMPLE(filecheck=CHECK): -target spirv -fvk-use-entrypoint-name -emit-spirv-directly

// CHECK-DAG: OpEntryPoint Vertex
// CHECK-DAG: OpEntryPoint Fragment

// we should only have 1 'BuiltIn InstanceIndex' since the `Output` and `Input` semantic
// for `InstanceIndex` should be converted to a non-builtin
// CHECK-DAG: OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
// CHECK-NOT: OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex

// We require 1 `Flat` for the fragment input `uint`
// CHECK-DAG: OpDecorate %{{[1-9][0-9]*}} Flat
// CHECK-NOT: OpDecorate %{{[1-9][0-9]*}} Flat

struct VIn
{
    uint instanceID : SV_InstanceID;
}

struct VSOutput
{
    uint instanceID : SV_InstanceID;
	float4 color : COLOR;
};

[shader("vertex")]
VSOutput vmain(VIn vin) {
    VSOutput t;
    t.instanceID = vin.instanceID;
    t.color = float4(0, 0, 0, 0);
    return t;
}

[shader("pixel")]
float4 pmain(VSOutput input) : SV_TARGET
{
	return float4(float(input.instanceID), input.color.xyz);
}
