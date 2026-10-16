// global-scope-cbuffer.slang
//
// The motivating case for #12307: loose global uniforms are gathered into an
// automatically-introduced `$Globals` constant buffer whose own binding is
// invisible in the flat `parameters` list. The new `globalScope` object must
// describe that constant buffer's binding and then its contained parameters.

//TEST:REFLECTION(filecheck=CHECK):-target hlsl -profile sm_5_0 -no-codegen

float4       gTint;
float        gScale;
Texture2D    gTex;
SamplerState gSamp;

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
}

//CHECK: "version": "1.1"

// The global scope is wrapped in the automatically-introduced constant buffer,
// and that constant buffer has its own binding at register b0. The loose globals
// `gTint`/`gScale` and the resources appear as its parameters.
//CHECK: "globalScope": {
//CHECK: "kind": "constantBuffer"
//CHECK: "binding": {"kind": "constantBuffer", "index": 0}
//CHECK: "parameters": [
//CHECK: "name": "gTint"
//CHECK: "name": "gScale"
//CHECK: "name": "gTex"
//CHECK: "name": "gSamp"
