//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl -stage vertex -entry vertexMain

// CHECK: OpDecorate {{.*}} InstanceIndex

// Per-vertex attributes to be assembled from bound vertex buffers.
struct VSin
{
  [[vk::location(0)]] float3 position : POSITION;
  [[vk::location(1)]] float2 uv : TEXCOORD;
};

// Output of the vertex shader, and input to the fragment shader.
struct PSin
{
  nointerpolation uint id : DRAW_ID;
  float2 uv : TEXCOORD;
};

// Output of the vertex shader
struct VSout
{
  PSin stage;
  float4 sv_position : SV_Position;
};

// Output of the fragment shader
struct PSout
{
  float4 color : SV_Target;
};

// Vertex  Shader
[shader("vertex")]
VSout vertexMain(VSin input)
{
  uint particleID = InstanceIndex();
  VSout output;
  output.sv_position = float4(0);
  output.stage.uv = input.uv;
  output.stage.id = InstanceIndex();

  return output;
}



// Fragment Shader
[shader("pixel")]
PSout fragmentMain(PSin stage)
{
  uint particleID = stage.id;
  
  PSout output;
  output.color = float4(particleID);
  
  return output;
}
