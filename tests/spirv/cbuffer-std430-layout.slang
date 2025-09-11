//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -stage fragment -emit-spirv-directly

// Test that layout(std430) cbuffer uses correct ArrayStride (8 for int2 array)
// instead of std140 ArrayStride (16 for int2 array)

layout(std430) cbuffer CBlock {
    int2 bb[2];
};

struct B {
    int2 a[2];
};

ConstantBuffer<B, Std430DataLayout> c;

//SPIRV-DAG: OpDecorate %_arr_v2int_int_2 ArrayStride 8

float4 main() : SV_Target
{
    return float4(0.3, 0.7, 0.55, 1.0
    + bb[1].y
    + c.a[0].x
    );
}