//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[noinline]
float4 vectorOps(float4 left, float4 right)
{
    float4 value = sin(left + right) * right / right + ceil(left) + floor(left) + exp(left) +
        exp2(left) + log(right) + log2(right);
    return value.wzyx;
}

[noinline]
bool4 vectorCompare(float4 left, float4 right)
{
    return !(left < right);
}

[noinline]
float2x2 matrixOps(float2x2 left, float2x2 right)
{
    float2x2 value = left + right;
    return sqrt(value) + ceil(value) + floor(value) + exp(value) + exp2(value) + log(value) +
        log2(value);
}

[noinline]
bool2x2 matrixCompare(float2x2 left, float2x2 right)
{
    return !(left < right);
}

[host]
void main()
{
    print(vectorOps(float4(0.0), float4(1.0))[0]);
    print(vectorCompare(float4(0.0), float4(1.0))[0]);
    print(matrixOps(float2x2(1.0), float2x2(2.0))[0][0]);
    print(matrixCompare(float2x2(1.0), float2x2(2.0))[0][0]);
}

// CHECK-DAG: [[FLOAT:%[A-Za-z0-9_]+]] = OpFloat32Type
// CHECK-DAG: [[FLOAT4:%[A-Za-z0-9_]+]] = OpVectorType 4 [[FLOAT]]
// CHECK-DAG: [[BOOL4:%[A-Za-z0-9_]+]] = OpVectorType 4 {{%[A-Za-z0-9_]+}}
// CHECK-DAG: [[FLOAT2X2:%[A-Za-z0-9_]+]] = OpMatrixType 2 2 {{[0-9]+}} [[FLOAT]]
// CHECK-DAG: [[BOOL2X2:%[A-Za-z0-9_]+]] = OpMatrixType 2 2 {{[0-9]+}} {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpAdd [[FLOAT4]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpMul [[FLOAT4]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpFDiv [[FLOAT4]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpSin
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpCeil
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpFloor
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpExp
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpExp2
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpLog
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT4]] = OpLog2
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[BOOL4]] = OpLess [[FLOAT4]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[BOOL4]] = OpNot
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT2X2]] = OpAdd [[FLOAT2X2]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT2X2]] = OpSqrt
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT2X2]] = OpCeil
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT2X2]] = OpFloor
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT2X2]] = OpExp
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT2X2]] = OpExp2
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT2X2]] = OpLog
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[FLOAT2X2]] = OpLog2
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[BOOL2X2]] = OpLess [[FLOAT2X2]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[BOOL2X2]] = OpNot
// CHECK-DAG: OpMakeVector
// CHECK-DAG: OpMakeMatrix
