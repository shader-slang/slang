//TEST:SIMPLE(filecheck=CHECK):-target spirv-assembly -entry main -stage fragment -emit-spirv-directly

// Verify that SubgroupLocalInvocationId gets Flat decoration in fragment shaders.

// CHECK-DAG: OpDecorate %SubgroupLocalInvocationId Flat
// CHECK-DAG: OpDecorate %SubgroupLocalInvocationId BuiltIn SubgroupLocalInvocationId

float4 main() : SV_Target
{
    uint laneId = WaveGetLaneIndex();
    return float4(float(laneId), 0, 0, 1);
}
