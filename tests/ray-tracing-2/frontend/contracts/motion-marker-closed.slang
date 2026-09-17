//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target metal -no-codegen

import slang.raytracing;

struct CustomMotion : rt::IRayMotion
//CHECK:                  ^^^^^^^^^^ cannot inherit from type 'rt.IRayMotion' marked 'sealed' in module 'raytracing'
{}

struct CustomEnabledMotion : rt::IEnabledRayMotion
//CHECK:                         ^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.IEnabledRayMotion' marked 'sealed' in module 'raytracing'
{}
