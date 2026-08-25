//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target metal

import slang.raytracing;

struct CustomMotion : rt::IRayMotion
//CHECK:                  ^^^^^^^^^^ cannot inherit from type 'rt.IRayMotion' marked 'sealed' in module 'raytracing'
{}

struct CustomEnabledMotion : rt::IEnabledRayMotion
//CHECK:                         ^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.IEnabledRayMotion' marked 'sealed' in module 'raytracing'
{}
