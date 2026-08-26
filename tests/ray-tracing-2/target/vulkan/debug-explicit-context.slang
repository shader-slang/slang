//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -target spirv -emit-spirv-directly -entry main -stage raygeneration -g2 -enable-experimental-passes

// Structural tracing must keep native ray-tracing globals outside an explicit context, while also
// preserving acceleration-structure debug information.

// CHECK-DAG: %[[AS_TYPE:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR
// CHECK-DAG: %[[AS_POINTER:[A-Za-z0-9_]+]] = OpTypePointer UniformConstant %[[AS_TYPE]]
// CHECK-DAG: %[[AS:[A-Za-z0-9_]+]] = OpVariable %[[AS_POINTER]] UniformConstant
// CHECK-DAG: %[[PAYLOAD:[A-Za-z0-9_]+]] = OpVariable %{{.*}} RayPayloadKHR
// CHECK-DAG: OpString "RaytracingAccelerationStructure"
// CHECK-DAG: DebugTypeComposite
// CHECK: OpLoad %[[AS_TYPE]] %[[AS]]
// CHECK: OpTraceRayKHR {{.*}} %[[PAYLOAD]]

#include "../../runtime/shaders/triangle-hit-miss.slang"
