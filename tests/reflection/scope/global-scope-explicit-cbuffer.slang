// global-scope-explicit-cbuffer.slang
//
// An explicit `ConstantBuffer<T>` global (as opposed to loose uniforms gathered
// into `$Globals`) appears as a top-level parameter, and the global scope has no
// loose uniforms of its own, so `globalScope` is `kind: "none"`.

//TEST:REFLECTION(filecheck=CHECK):-target hlsl -profile sm_5_0 -no-codegen

struct Params
{
    float4 tint;
    float  scale;
}

ConstantBuffer<Params> gParams;
Texture2D              gTex;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: "version": "1.1"

//CHECK: "globalScope": {
//CHECK: "kind": "none"
//CHECK: "parameters": [
//CHECK: "name": "gParams"
//CHECK: "name": "gTex"
