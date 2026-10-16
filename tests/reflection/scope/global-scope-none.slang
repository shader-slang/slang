// global-scope-none.slang
//
// When the global scope has no loose uniform parameters (only resources, which
// bind directly), no constant buffer is introduced, so the scope is unwrapped:
// `kind: "none"`, with the resources listed as parameters.

//TEST:REFLECTION(filecheck=CHECK):-target hlsl -profile sm_5_0 -no-codegen

Texture2D    gTex;
SamplerState gSamp;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: "version": "1.1"

//CHECK: "globalScope": {
//CHECK: "kind": "none"
//CHECK: "parameters": [
//CHECK: "name": "gTex"
//CHECK: "name": "gSamp"
