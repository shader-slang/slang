// entry-point-scope-uniform.slang
//
// Entry-point `uniform` parameters are gathered into the entry point's scope,
// analogously to the global scope. The new per-entry-point `scope` object must
// describe them.

//TEST:REFLECTION(filecheck=CHECK):-target hlsl -profile sm_5_0 -entry main -stage compute -no-codegen

[numthreads(1, 1, 1)]
void main(uniform float4 uColor, uniform float uGain)
{
}

//CHECK: "version": "1.1"

//CHECK: "entryPoints": [
//CHECK: "name": "main"
//CHECK: "stage": "compute"
// The entry-point uniforms are gathered into a constant buffer whose own binding
// is reported on the scope, exactly like the global scope.
//CHECK: "scope": {
//CHECK: "kind": "constantBuffer"
//CHECK: "binding": {"kind": "constantBuffer", "index": 0}
//CHECK: "parameters": [
//CHECK: "name": "uColor"
//CHECK: "name": "uGain"
