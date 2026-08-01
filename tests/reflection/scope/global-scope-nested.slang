// global-scope-nested.slang
//
// A nested `ConstantBuffer<ParameterBlock<T>>` global is a single parameter of
// the global scope, which binds directly (`kind: "none"`). This confirms the
// scope object composes with an arbitrarily-nested container parameter: the
// parameter's own type layout carries the nested constant-buffer / parameter-block
// structure, emitted by the existing parameter machinery the scope routine reuses.

//TEST:REFLECTION(filecheck=CHECK):-target hlsl -profile sm_5_1 -entry main -stage compute -no-codegen

struct Inner
{
    float4 value;
}

ConstantBuffer<ParameterBlock<Inner>> gNested;

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: "version": "1.1"

//CHECK: "globalScope": {
//CHECK: "kind": "none"
//CHECK: "parameters": [
//CHECK: "name": "gNested"
// Pin the nesting on gNested's own type: the parameter block is the constant buffer's
// direct element, so the two `kind` lines must be adjacent through the `elementType` key.
//CHECK: "kind": "constantBuffer"
//CHECK-NEXT: "elementType"
//CHECK-NEXT: "kind": "parameterBlock"
