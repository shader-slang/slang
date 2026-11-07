// A module-scope varying declared in an `__include`d file is at the top level of the module, so
// E31401 does not apply. `__include` makes the module Slang 2025, so the varying is rejected with
// E30626.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target spirv -entry main -stage fragment -no-codegen

module "global-varying-include";
__include global_varying_include_helper;

// CHECK: error[E30626]: module-scope varying 'gIncluded' is not supported in Slang 2025 and later
// CHECK-NOT: E31401

float4 main() : SV_Target
{
    return gIncluded;
}
