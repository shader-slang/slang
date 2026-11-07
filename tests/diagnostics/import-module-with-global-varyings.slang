// An entry point owns the module-scope varyings of its own module only, so importing a module that
// declares module-scope varyings is an error: those varyings would belong to no entry point.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage fragment -no-codegen -warnings-disable 30627

import "import-module-with-global-varyings-helper";
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ cannot import a module that declares module-scope varyings
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ module 'import-module-with-global-varyings-helper' declares module-scope 'in'/'out' variables, which belong only to the entry points declared in that module
*/

float4 main() : SV_Target
{
    return gHelperColor;
}
