//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly -entry vertexMain -stage vertex

[[vk::offset(16)]] struct S1
/*CHECK:
      ^^^^^^ invalid attribute placement
      ^^^^^^ attribute 'vk_offset' is not valid here
*/
{
    [[vk::offset(32)]]
    float2 a;

    float3 b;

    [[vk::offset(16)]]
    float4 c;
  }

[[vk::push_constant]]
S1 pc;

struct VertexOutput
{
    float3 position : SV_Position;
//CHECK:   ^^^^^^^^ type 'vector<float,3>' is not valid for system value semantic 'SV_Position'; expected 'vector<float,4>'
}

[shader("vertex")]
VertexOutput vertexMain()
{
    [[vk::offset(8)]] VertexOutput output;
//CHECK:  ^^^^^^ invalid attribute placement
//CHECK:  ^^^^^^ attribute 'vk_offset' is not valid here
    output.position = float3(pc.a.x, pc.b.y, pc.c.z);
    return output;
}
