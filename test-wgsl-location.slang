struct VertexInput
{
    float2 position : POSITION;
    float3 color : COLOR;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain( VertexInput input ) 
{
    VertexOutput output;

    const float aspectRatio = 640.0f / 480.0f;                
    const float2 offset = float2( -0.6875f, -0.463f );        

    output.position = float4( input.position.x + offset.x, (input.position.y + offset.y) * aspectRatio, 0.0f, 1.0f );
    output.color = input.color;		

    return output;
}

[shader("fragment")]
float4 fragmentMain( VertexOutput input ) : SV_Target
{	
    return float4( input.color, 1.0f );		
}
