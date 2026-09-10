// entry-point-scope-empty.slang
//
// An entry point with no parameters at all has an empty scope. That scope is
// still modeled as a (zero-field) struct, so it is reported as `kind: "none"`
// with an explicit empty `"parameters": []` — the key is present, not omitted.
// This pins the empty-scope output shape so it stays deterministic.

//TEST:REFLECTION(filecheck=CHECK):-target hlsl -profile sm_5_0 -entry main -stage compute -no-codegen

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: "version": "1.1"

//CHECK: "stage": "compute"
//CHECK: "scope": {
//CHECK-NEXT: "kind": "none"
//CHECK-NEXT: "parameters": [
//CHECK-EMPTY:
//CHECK-NEXT: ]
