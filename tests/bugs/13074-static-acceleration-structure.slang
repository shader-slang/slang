// Resource-global replacement would create a function-local acceleration-structure variable.
// Khronos and WGSL targets reject that storage, so the front end rejects the source declaration on
// every target rather than making source legality depend on the selected backend.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage raygeneration
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage raygeneration

RaytracingAccelerationStructure inputScene;
static RaytracingAccelerationStructure cachedScene = inputScene;
//CHECK:                               ^^^^^^^^^^^ global variable cannot have opaque type
//CHECK:                               ^^^^^^^^^^^ global variable cannot have opaque type.

[shader("raygeneration")]
void main()
{
}
