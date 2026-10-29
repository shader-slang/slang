// Large vectors and matrices are emitted as arrays on every target: `vector<float, 6>` as an
// array of 6 floats, `matrix<float, 6, 6>` as an array of 6 such arrays, and `matrix<float, 6, 3>`
// as an array of 6 native 3-vectors. Native sizes are left alone.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute

// SPIRV-DAG: %[[F6:[A-Za-z0-9_]+]] = OpTypeArray %float %int_6
// SPIRV-DAG: OpTypeArray %[[F6]] %int_6
// SPIRV-DAG: OpTypeArray %v3float %int_6
// SPIRV-DAG: OpTypeMatrix %v4float 4
// SPIRV: %rowSums = OpFunction %[[F6]]

// HLSL: void rowSums_0(float {{.*}}m_0[int(6)][int(6)], out float {{.*}}[int(6)])
// HLSL: float3 firstRow_0(float3 {{.*}}m_1[int(6)])
// HLSL: float4x4 native_0

// GLSL: void rowSums_0(float {{.*}}m_0[6][6], out float {{.*}}[6])
// GLSL: vec3 firstRow_0(vec3 {{.*}}m_1[6])
// GLSL: mat4x4 native_0

// METAL: array<float, int(6)> rowSums_0(const array<array<float, int(6)>, int(6)> thread* m_0)
// METAL: float3 firstRow_0(const array<float3, int(6)> thread* m_1)
// METAL: matrix<float,int(4),int(4)> {{.*}}native_0

// WGSL: fn rowSums_0( m_0 : array<array<f32, i32(6)>, i32(6)>
// WGSL: fn firstRow_0( m_1 : array<vec3<f32>, i32(6)>) -> vec3<f32>
// WGSL: var native_0 : mat4x4<f32>

// CUDA: rowSums_0(FixedArray<FixedArray<float, 6> , 6> {{.*}}m_0, FixedArray<float, 6> {{.*}})
// CUDA: firstRow_0(FixedArray<float3 , 6> {{.*}}m_1)
// CUDA: Matrix<float, 4, 4> {{.*}}native_0

// CPP: rowSums_0(FixedArray<FixedArray<float, 6> , 6> {{.*}}m_0, FixedArray<float, 6> {{.*}})
// CPP: firstRow_0(FixedArray<Vector<float, 3> , 6> {{.*}}m_1)
// CPP: Matrix<float, 4, 4> {{.*}}native_0

RWStructuredBuffer<float> outputBuffer;

[noinline]
vector<float, 6> rowSums(matrix<float, 6, 6> m)
{
    vector<float, 6> result;
    for (int i = 0; i < 6; i++)
        result[i] = dot(m[i], vector<float, 6>(1.0));
    return result;
}

[noinline]
float3 firstRow(matrix<float, 6, 3> m)
{
    return m[0];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    matrix<float, 6, 6> m = matrix<float, 6, 6>(outputBuffer[tid.x]);
    matrix<float, 6, 3> t = matrix<float, 6, 3>(outputBuffer[1]);
    float4x4 native = float4x4(outputBuffer[2]);
    outputBuffer[0] = rowSums(m)[tid.x] + firstRow(t).y + native[1][2];
}
