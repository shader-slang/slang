//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -g2 -emit-spirv-directly

// A function defined in an __include'd file has no compilation unit of its own (an __include is
// textually part of the including module), so its DebugFunction must be scoped to the compilation
// unit of the file that included it — verified below against the module's own DebugCompilationUnit.

module debug_function_scope_include;

__include "debug-function-scope-included";

[shader("fragment")]
float4 main() : SV_Target
{
    return helperFromIncludedFile(0.25);
}

// The main file is the module's only compilation unit here (the __include'd file has none).
// CHECK-DAG: [[MAIN_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-include.slang"
// CHECK-DAG: [[SOURCE_MAIN:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[MAIN_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_MAIN:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_MAIN]] %uint_{{[0-9]+}}

// The included file has its own DebugSource but NO DebugCompilationUnit of its own.
// CHECK-DAG: [[INCL_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-included.slang"
// CHECK-DAG: [[SOURCE_INCL:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[INCL_FILE]] %{{[0-9]+}}

// The included function references the included file as its source (operand 3) but is scoped to
// the module's compilation unit [[CU_MAIN]] (the trailing scope operand), since the include has
// no compilation unit of its own.
// CHECK-DAG: [[FN_HELPER:%[0-9]+]] = OpString "helperFromIncludedFile"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_HELPER]] %{{[0-9]+}} [[SOURCE_INCL]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_MAIN]]
