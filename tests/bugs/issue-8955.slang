//TEST:SIMPLE: -o tests/bugs/issue-8955.slang-module
//TEST:SIMPLE(filecheck=CHK): -target spirv-asm tests/bugs/issue-8955.slang-module

// Test for issue #8955: Using link-time struct as vertex input crashes if compiled from serialized module
// This test verifies that default export types (export struct X = Y) properly preserve their aliasedType
// during serialization/deserialization.

//CHK:OpEntryPoint Vertex %vertexMain

struct VertexOutput {
    float4 position : SV_Position;
};

interface IVertex {
    float3 pos();
}

struct TestVertex : IVertex {
    [[vk::location(0)]] float3 a_pos;

    float3 pos() {
        return a_pos;
    }
}

export struct Vertex : IVertex = TestVertex;

[[shader("vertex")]]
VertexOutput vertexMain(Vertex input) {
    return VertexOutput(float4(input.pos(), 1.0));
}
