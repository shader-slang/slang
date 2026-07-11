//TEST:SIMPLE(filecheck=CHECK):-target spirv-assembly -entry main -stage fragment -emit-spirv-directly

// Verify that the integral subgroup input builtins used by WaveGetLaneIndex and
// WaveGetLaneCount receive the Flat decoration required for fragment shaders.

// CHECK-DAG: OpDecorate %SubgroupLocalInvocationId Flat
// CHECK-DAG: OpDecorate %SubgroupLocalInvocationId BuiltIn SubgroupLocalInvocationId
// CHECK-DAG: OpDecorate %SubgroupSize Flat
// CHECK-DAG: OpDecorate %SubgroupSize BuiltIn SubgroupSize

float4 main() : SV_Target
{
    uint laneId = WaveGetLaneIndex();
    uint laneCount = WaveGetLaneCount();
    return float4(float(laneId), float(laneCount), 0, 1);
}
