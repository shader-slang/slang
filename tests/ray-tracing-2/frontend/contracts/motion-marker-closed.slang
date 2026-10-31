//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target metal -no-codegen

import slang.raytracing.portable;

struct CustomMotion : rt::IRayMotion
//CHECK:                  ^^^^^^^^^^ cannot inherit from type 'rt.IRayMotion' marked 'sealed' in module 'portable'
{}

struct CustomEnabledMotion : rt::IEnabledRayMotion
//CHECK:                         ^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.IEnabledRayMotion' marked 'sealed' in module 'portable'
{}
