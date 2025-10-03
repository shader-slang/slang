//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -stage fragment -emit-spirv-directly
//TEST:SIMPLE(filecheck=WARN): -target dxil -entry main -stage fragment

// Test that layout(std430) cbuffer uses correct ArrayStride (8 for int2 array)
// instead of std140 ArrayStride (16 for int2 array)

layout(std430) cbuffer CBlock {
    int2 bb[2];
};

//SPIRV-DAG: OpDecorate %_arr_v2int_int_2 ArrayStride 8

//WARN: warning 39012: layout qualifier 'std430' is not supported for target 'dxil'

float4 main() : SV_Target
{
    return float4(0.3, 0.7, 0.55, 1.0 + bb[1].y);
}