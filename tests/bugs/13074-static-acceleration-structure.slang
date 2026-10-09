// Resource-global replacement would create a function-local acceleration-structure variable.
// Khronos and WGSL targets reject that storage, so the front end rejects the declaration written
// by the programmer on every target rather than making front-end legality depend on the selected
// backend.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage raygeneration
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage raygeneration

RaytracingAccelerationStructure inputScene;
static RaytracingAccelerationStructure cachedScene = inputScene;
//CHECK:                               ^^^^^^^^^^^ opaque type is not supported for mutable static storage
//CHECK:                               ^^^^^^^^^^^ mutable file- or namespace-scope `static` variable 'cachedScene' has the opaque type 'RaytracingAccelerationStructure', which cannot be represented by mutable storage

[shader("raygeneration")]
void main()
{
}
