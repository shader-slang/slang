// global-scope-metal.slang
//
// The scope's container binding is target-dependent. This checks the Metal
// target: loose global uniforms are still gathered into a constant buffer whose
// own binding is reported in `globalScope`.

//TEST:REFLECTION(filecheck=CHECK):-target metal -entry main -stage compute -no-codegen

float4    gTint;
Texture2D gTex;

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: "version": "1.1"

//CHECK: "globalScope": {
//CHECK: "kind": "constantBuffer"
//CHECK: "binding":
//CHECK: "parameters": [
//CHECK: "name": "gTint"
//CHECK: "name": "gTex"
