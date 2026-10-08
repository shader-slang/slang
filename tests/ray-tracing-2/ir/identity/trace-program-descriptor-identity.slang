//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry main -stage compute -dump-ir -no-codegen

import slang.raytracing;

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

rt::TraceProgramDescriptor<Schema> descriptor;

// The imported serialized module must preserve the compiler-owned operation, including its
// schema type argument and conformance witness.
// CHECK-NOT: TraceProgramDescriptorResources
// CHECK: [nameHint("descriptor")]
// CHECK-NEXT: [export({{.*}})]
// CHECK-NEXT: let %descriptor : TraceProgramDescriptor(%Schema, %{{[0-9]+}}) = global_param
// CHECK-NOT: TraceProgramDescriptorResources

[numthreads(1, 1, 1)]
void main() {}
