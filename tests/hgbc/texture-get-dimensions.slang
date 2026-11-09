//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public void main(RWTexture2D<float4> output)
{
    uint width;
    uint height;
    output.GetDimensions(width, height);
    let groupWidth = (width + 7) / 8;
    print(width);
    print(height);
    print(groupWidth);
}

// CHECK: Section("LITR")
// CHECK-DAG: [[ZERO:%[A-Za-z0-9_]+]] : [[UINT:%[A-Za-z0-9_]+]] = OpIntLit 0
// CHECK-DAG: [[ONE:%[A-Za-z0-9_]+]] : [[UINT]] = OpIntLit 1
// CHECK-DAG: [[SEVEN:%[A-Za-z0-9_]+]] : [[UINT]] = OpIntLit 7
// CHECK-DAG: [[EIGHT:%[A-Za-z0-9_]+]] : [[UINT]] = OpIntLit 8

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT]] = OpUIntType
// CHECK-DAG: [[FLOAT:%[A-Za-z0-9_]+]] = OpFloat32Type
// CHECK-DAG: [[FLOAT4:%[A-Za-z0-9_]+]] = OpVectorType 4 [[FLOAT]]
// CHECK-DAG: [[UINT2:%[A-Za-z0-9_]+]] = OpVectorType 2 [[UINT]]
// CHECK-DAG: [[TARGET_TYPE:%[A-Za-z0-9_]+]] = OpTexture2DType [[FLOAT4]] ReadWrite Undefined

// CHECK: Section("FUNC")
// CHECK-DAG: [[OUTPUT:%[A-Za-z0-9_]+]] : [[TARGET_TYPE]] = OpParam
// CHECK-DAG: [[DIMS:%[A-Za-z0-9_]+]] : [[UINT2]] = OpTextureGetDimensions [[OUTPUT]]
// CHECK-DAG: [[WIDTH:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[DIMS]] [[ZERO]]
// CHECK-DAG: [[HEIGHT:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[DIMS]] [[ONE]]
// CHECK-DAG: [[WIDTH_PLUS_SEVEN:%[A-Za-z0-9_]+]] : [[UINT]] = OpAdd [[UINT]] [[WIDTH]] [[SEVEN]]
// CHECK-DAG: [[GROUP_WIDTH:%[A-Za-z0-9_]+]] : [[UINT]] = OpIDiv [[UINT]] [[WIDTH_PLUS_SEVEN]] [[EIGHT]]
// CHECK: OpPrint [[UINT]] [[WIDTH]]
// CHECK: OpPrint [[UINT]] [[HEIGHT]]
// CHECK: OpPrint [[UINT]] [[GROUP_WIDTH]]
