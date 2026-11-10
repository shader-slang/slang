// A module-scope varying declared in an `__include`d file is at the top level of the module, so
// E31401 does not apply. The `module` declaration makes the module Slang 2025, so the varying is
// rejected with E30626. The run is exhaustive, so an E31401 would fail it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage fragment -no-codegen

module "global-varying-include";
__include global_varying_include_helper;

//CHECK: module-scope varying 'gIncluded' is not supported in Slang 2025 and later
//CHECK: module-scope varyings are a GLSL feature; declare 'gIncluded' as an entry-point parameter instead

float4 main() : SV_Target
{
    return gIncluded;
}
