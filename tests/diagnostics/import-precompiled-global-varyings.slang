// A precompiled module keeps the marker for its module-scope varyings, so importing it is rejected
// as importing its source is. `import` prefers the `.slang-module` that the COMPILE step writes, and
// a binary module is not checked again: if the source were checked instead, its W30627 would fail
// this exhaustive run.

//TEST:COMPILE: tests/diagnostics/import-precompiled-global-varyings-helper.slang -o tests/diagnostics/import-precompiled-global-varyings-helper.slang-module -warnings-disable 30627
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage fragment -no-codegen

import "import-precompiled-global-varyings-helper";
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ cannot import a module that declares module-scope varyings
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ module 'import-precompiled-global-varyings-helper' declares module-scope varyings
*/

float4 main() : SV_Target
{
    return precompiledHelper();
}
