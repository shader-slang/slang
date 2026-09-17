//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target metal -no-codegen

import slang.raytracing;

// Structural lowering only understands the standard acceleration-structure topologies declared by
// `slang.raytracing`, so applications cannot add another topology by implementing the marker.
struct CustomAccelerationStructure : rt::IAccelerationStructure
//CHECK:                                 ^^^^^^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.IAccelerationStructure' marked 'sealed' in module 'raytracing'
{}
