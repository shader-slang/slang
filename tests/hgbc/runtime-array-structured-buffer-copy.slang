//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

struct Camera
{
    row_major float4x4 view;
    row_major float4x4 projection;
}

struct CameraData
{
    Camera camera;
    float2 screenCenter;
}

struct ScalarRecord
{
    float values[3];
}

[host]
void main()
{
    RuntimeArray<CameraData> source = RuntimeArray<CameraData>::alloc(1);
    RWStructuredBuffer<CameraData> destination = RWStructuredBuffer<CameraData>::alloc(1);
    copy(destination, 0, source, 0, 1);

    StructuredBuffer<ScalarRecord, ScalarDataLayout> scalarBuffer =
        StructuredBuffer<ScalarRecord, ScalarDataLayout>::alloc(1);
}

// CHECK-DAG: [[CAMERA_DATA:%CameraData]] = OpStructType
// CHECK-DAG: [[RUNTIME_ARRAY:%[A-Za-z0-9_]+]] = OpRuntimeArrayType [[CAMERA_DATA]]
// CHECK-DAG: [[DEVICE_BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[CAMERA_DATA]]
// ScalarDataLayout remains available for packed scalar records such as binary PLY vertices.
// CHECK-DAG: [[SCALAR_RECORD:%ScalarRecord]] = OpStructType
// CHECK-DAG: OpStructuredBufferType [[SCALAR_RECORD]]
// CHECK: OpCopyArrayRange [[DEVICE_BUFFER]] [[RUNTIME_ARRAY]] [[CONVERTER:%[A-Za-z0-9_]+]]
// CHECK: Section("EMBD")
// CHECK-DAG: [[HOST_LAYOUT:%[A-Za-z0-9_]+]] = OpTypeLayout [[CAMERA_DATA]] Host Value
// CHECK-DAG: [[DEVICE_LAYOUT:%[A-Za-z0-9_]+]] = OpTypeLayout [[CAMERA_DATA]] DeviceMemory StructuredBufferElement
// CHECK-DAG: [[CONVERTER]] = OpLayoutConverter [[HOST_LAYOUT]] [[DEVICE_LAYOUT]]
