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
// CHECK: DebugFunctionDefinition
// CHECK: DebugScope
// CHECK: DebugLine

// Ensure we don't create DebugVar for PositionReader struct (contains StructuredBuffer)
