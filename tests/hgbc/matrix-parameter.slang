//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -matrix-layout-row-major

public struct Transform
{
    public float3x4 matrix;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void applyTransform(uniform Transform transform, RWStructuredBuffer<float4> output)
{
    output[0] = mul(transform.matrix, float4(1.0, 2.0, 3.0, 1.0)).xyzz;
}

[host]
public void main(Transform transform, RWStructuredBuffer<float4> output)
{
    dispatch(applyTransform, uint3(1, 1, 1))(transform, output);
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[FLOAT:%[A-Za-z0-9_]+]] = OpFloat32Type
// CHECK-DAG: [[MATRIX:%[A-Za-z0-9_]+]] = OpMatrixType 3 4 1 [[FLOAT]]
// CHECK-DAG: [[TRANSFORM:%[A-Za-z0-9_]+]] = OpStructType {{%[A-Za-z0-9_]+}} [[MATRIX]]
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType {{%[A-Za-z0-9_]+}}

// CHECK: Section("FUNC")
// CHECK: {{%[A-Za-z0-9_]+}} : [[TRANSFORM]] = OpParam
// CHECK: {{%[A-Za-z0-9_]+}} : [[BUFFER]] = OpParam
// CHECK: OpCall
