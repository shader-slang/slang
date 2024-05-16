//TEST:REFLECTION(filecheck=REFLECT): -stage compute -entry computeMain  -target metal

// REFLECT: "name": "sampler"
// REFLECT: {"kind": "shaderResource", "index": 0}
// REFLECT: {"kind": "samplerState", "index": 0}
Sampler2D sampler;

// REFLECT: "name": "followingTexture"
// REFLECT: "binding": {"kind": "shaderResource", "index": 1}
Texture2D followingTexture;

// REFLECT: "name": "followingSamplerState"
// REFLECT: "binding": {"kind": "samplerState", "index": 1}
SamplerState followingSamplerState;

// REFLECT: "name": "explicitBindingSampler"
// REFLECT: {"kind": "shaderResource", "index": 4}
// REFLECT: {"kind": "samplerState", "index": 3}
Sampler2D explicitBindingSampler : register(t4): register(s3);

float4 computeMain() : SV_Target
{
    return sampler.Sample(float2(0, 0)) + explicitBindingSampler.Sample(float2(0, 0)) + followingTexture.Sample(followingSamplerState, float2(0,0));
}
