// sv-clip-distance.slang

//TEST:SIMPLE: -profile sm_5_0 -stage vertex -entry mainVertex -target glsl

// (filecheck=CHECK)
//-CHECK: out float  gl_ClipDistance[2];
//-CHECK: gl_ClipDistance[0] =
//-CHECK: gl_ClipDistance[1] =

struct VertexInput 
{
    float3 position : POSITION; 
};

void mainVertex(VertexInput vi, out float4 outPosition : SV_Position, out float outClips[2] : SV_ClipDistance) 
{
    outPosition = float4(vi.position, 1);
    outClips[0] = vi.position.x / 10;
    outClips[1] = vi.position.z * 2;
} 

