//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc

[shader("vertex")]
float4 vertexMain(uint vertexID : SV_VertexID) : SV_Position { return float4(0); }
[shader("fragment")]
float4 fragmentMain() : SV_Target { return float4(1); }

void wrongDescriptor()
{
    __makeGraphicsPipeline(vertexMain, fragmentMain, 1);
}

// CHECK: expected a '__GraphicsPipelineDesc' value, got 'int'
