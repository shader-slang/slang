//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly

// Test for debug variable handling with generic parameters and structs containing resources
// This test ensures that structs containing StructuredBuffer fields don't get invalid DebugVar instructions

StructuredBuffer<uint4> gData;

interface IGeometryReader
{
    float4 read(uint attributeIndex);
}

struct PositionReader : IGeometryReader
{
    float4 read(uint vertexIndex)
    {
        return m_geometryBuffer[vertexIndex];
    }

    __init(StructuredBuffer<uint4> geometryBuffer)
    {
        m_geometryBuffer = geometryBuffer;
    }

    StructuredBuffer<uint4> m_geometryBuffer;
}

float4 test<Reader: IGeometryReader>(Reader reader)
{
    float4 pos = reader.read(0);
    return pos;
}

RWStructuredBuffer<float4> result;

[numthreads(1,1,1)]
void main()
{
    let reader = PositionReader(gData);
    float4 pos = test(reader);
    
    result[0] = pos;
}

// Verify that debug info is generated but no invalid DebugVar for struct with StructuredBuffer
// CHECK: OpExtInst %void {{.*}} DebugExpression

// Ensure we have the expected legitimate debug variables
// CHECK: pos{{.*}}DebugLocalVariable{{.*}}
// CHECK: vertexIndex{{.*}}DebugLocalVariable{{.*}}

// After legitimate debug vars, ensure we don't create debug vars for non-debuggable types
// Specifically check that we don't create debug vars for variables containing StructuredBuffer:
// - No debug var for 'reader' (PositionReader struct instance)
// - No debug var for 'm_geometryBuffer' (StructuredBuffer field)
// The absence of these variables in the output proves the fix is working
// CHECK-NOT: reader{{.*}}DebugLocalVariable{{.*}}
// CHECK-NOT: m_geometryBuffer{{.*}}DebugLocalVariable{{.*}}
// CHECK-NOT: geometryBuffer{{.*}}DebugLocalVariable{{.*}}

// Verify the function ends properly
// CHECK: OpFunctionEnd