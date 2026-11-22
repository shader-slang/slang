//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0

struct Vertex
{
    float4 position : SV_Position;
};

[ForceInline]
void emitVertices(Vertex input[3], inout TriangleStream<Vertex> stream)
{
    for (uint index = 0; index < 3; ++index)
    {
        Vertex vertex = input[index];
        stream.Append(vertex);
    }
    stream.RestartStrip();
}

[shader("geometry")]
[maxvertexcount(3)]
void main(
    triangle Vertex vertices[3],
    uint primitiveIndex : SV_PrimitiveID,
    inout TriangleStream<Vertex> stream)
{
    if ((primitiveIndex & 1) != 0)
    {
        emitVertices(vertices, stream);
    }
    else
    {
        emitVertices(vertices, stream);
    }
}

// Each inline instance keeps the array parameter in the callee function scope.
// The call sites belong to distinct caller blocks; the loop/body hierarchy is shared.
// clang-format off
// CHECK-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[HELPER_NAME:%[0-9]+]] = OpString "emitVertices"
// CHECK-DAG: [[INPUT_NAME:%[0-9]+]] = OpString "input"
// CHECK-DAG: [[VERTEX_NAME:%[0-9]+]] = OpString "vertex"
// CHECK-DAG: [[MAIN:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]]
// CHECK-DAG: [[HELPER:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[HELPER_NAME]]
// CHECK-DAG: [[CALLER_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE:%[A-Za-z_0-9]+]] %uint_28 %uint_9 [[MAIN]]
// CHECK-DAG: [[CALLER_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_32 %uint_9 [[MAIN]]
// CHECK-DAG: [[AT_A:%[A-Za-z_0-9]+]] = {{.*}} DebugInlinedAt %uint_28 [[CALLER_A]]{{$}}
// CHECK-DAG: [[AT_B:%[A-Za-z_0-9]+]] = {{.*}} DebugInlinedAt %uint_32 [[CALLER_B]]{{$}}
// CHECK-DAG: [[LOOP:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_11 %uint_5 [[HELPER]]
// CHECK-DAG: [[BODY:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_13 %uint_9 [[LOOP]]
// CHECK-DAG: [[INPUT_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[INPUT_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_9 %uint_6 [[HELPER]] %uint_0 %uint_1
// CHECK-DAG: [[INPUT_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[INPUT_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_9 %uint_6 [[HELPER]] %uint_0 %uint_1
// CHECK-DAG: [[LOCAL_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[VERTEX_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_13 %uint_16 [[BODY]] %uint_0
// CHECK-DAG: [[LOCAL_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[VERTEX_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_13 %uint_16 [[BODY]] %uint_0
// CHECK: DebugScope [[HELPER]] [[AT_A]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[INPUT_A]]
// CHECK: OpLoopMerge [[EXIT_A:%[A-Za-z_0-9]+]]
// CHECK: DebugScope [[BODY]] [[AT_A]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[LOCAL_A]]
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpEmitVertex
// CHECK: [[EXIT_A]] = OpLabel
// CHECK: DebugScope [[LOOP]] [[AT_A]]{{$}}
// CHECK: DebugScope [[HELPER]] [[AT_A]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpEndPrimitive
// CHECK: DebugScope [[CALLER_A]]{{$}}
// CHECK: DebugScope [[HELPER]] [[AT_B]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[INPUT_B]]
// CHECK: OpLoopMerge [[EXIT_B:%[A-Za-z_0-9]+]]
// CHECK: DebugScope [[BODY]] [[AT_B]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[LOCAL_B]]
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpEmitVertex
// CHECK: [[EXIT_B]] = OpLabel
// CHECK: DebugScope [[LOOP]] [[AT_B]]{{$}}
// CHECK: DebugScope [[HELPER]] [[AT_B]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpEndPrimitive
// CHECK: DebugScope [[CALLER_B]]{{$}}
// CHECK: DebugScope [[MAIN]]{{$}}
// clang-format on
