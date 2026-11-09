//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -matrix-layout-row-major

[shader("compute")]
[numthreads(1, 1, 1)]
void consumeMatrix(uniform float4x4 matrix, RWStructuredBuffer<float> output)
{
    output[0] = matrix[0][0];
}

float4x4 makeMatrix(uint width, int offset, float angle)
{
    float widthFloat = float(width);
    float offsetFloat = float(offset);
    float value =
        (sin(angle) + cos(angle)) * tan(angle) / sqrt(widthFloat) - pow(0.9, angle) +
        ceil(angle) + floor(angle) + exp(angle) + exp2(angle) + log(widthFloat) +
        log2(widthFloat) + offsetFloat;
    float4 row = float4(value, widthFloat, -angle, 1.0);
    return float4x4(row, row, row, row);
}

[host]
public void main(uint width, int offset, float angle, bool enabled, RWStructuredBuffer<float> output)
{
    print(int(angle));
    print(uint(angle));
    print(bool(width));
    print(int(enabled));
    dispatch(consumeMatrix, uint3(1, 1, 1))(makeMatrix(width, offset, angle), output);
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[INT:%[A-Za-z0-9_]+]] = OpIntType
// CHECK-DAG: [[FLOAT:%[A-Za-z0-9_]+]] = OpFloat32Type
// CHECK-DAG: [[BOOL:%[A-Za-z0-9_]+]] = OpBoolType
// CHECK-DAG: [[MATRIX:%[A-Za-z0-9_]+]] = OpMatrixType 4 4 1 [[FLOAT]]

// CHECK: Section("FUNC")
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpScalarTypeCast [[UINT]] {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpScalarTypeCast [[INT]] {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[INT]] = OpScalarTypeCast [[FLOAT]] {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[UINT]] = OpScalarTypeCast [[FLOAT]] {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[BOOL]] = OpScalarTypeCast [[UINT]] {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[INT]] = OpScalarTypeCast [[BOOL]] {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpSin
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpCos
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpTan
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpSqrt
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpPow [[FLOAT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpCeil
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpFloor
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpExp
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpExp2
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpLog
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpLog2
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpAdd [[FLOAT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpSub [[FLOAT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpMul [[FLOAT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpFDiv [[FLOAT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT]] = OpNeg
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[MATRIX]] = OpMakeMatrix
