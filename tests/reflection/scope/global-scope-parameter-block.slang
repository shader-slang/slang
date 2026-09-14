// global-scope-parameter-block.slang
//
// A `ParameterBlock<T>` global is a single parameter of the global scope (which
// itself binds directly, so `kind: "none"`); the parameter block introduces a
// register space, reported on the parameter's own binding. This confirms an
// explicit parameter block is NOT confused with the scope-level wrapper (which
// only appears when the whole scope is gathered into a container).

//TEST:REFLECTION(filecheck=CHECK):-target hlsl -profile sm_5_1 -entry main -stage compute -no-codegen

struct Material
{
    float4    albedo;
    Texture2D tex;
}

ParameterBlock<Material> gMaterial;

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: "version": "1.1"

// The global scope binds directly (no scope-level container), with the parameter
// block appearing as one of its parameters.
//CHECK: "globalScope": {
//CHECK: "kind": "none"
//CHECK: "parameters": [
//CHECK: "name": "gMaterial"
//CHECK: "kind": "subElementRegisterSpace"
//CHECK: "kind": "parameterBlock"
