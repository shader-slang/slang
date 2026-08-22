// Disabled: an `__include`'d file's source view carries no initiating location, so the include chain
// that resolves an owning compilation unit does not exist for it. Enabling this test requires
// `__include` to record that location the way `#include` does — see shader-slang/slang#12339.
//DISABLE_TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly -I tests/spirv

// A function defined in an __include'd file has no compilation unit of its own — an __include is
// textually part of the module that included it — so its DebugFunction must be scoped to the
// compilation unit of that module.
//
// This file holds only the entry point; debug-function-scope-include-mod.slang is the module that
// performs the __include. They are separate top-level files, so each has its own
// DebugCompilationUnit. That separation is load-bearing rather than incidental: when the owning
// compilation unit cannot be resolved, emission falls back to a module-global scope pinned to the
// *entry point's* compilation unit. If the __include lived in this file, that fallback would
// coincide with the correct answer and this test could pass while the feature is broken.

import debug_function_scope_include_mod;

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(helperFromIncludedFile(0.25));
}

// This file holds the entry point, so its compilation unit is the one the fallback is pinned to.
// CHECK-DAG: [[ENTRY_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-include.slang"
// CHECK-DAG: [[SOURCE_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[ENTRY_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_ENTRY]] %uint_{{[0-9]+}}

// The module that performs the __include has its own, distinct compilation unit, and it is the one
// that owns the __include'd function.
// CHECK-DAG: [[MOD_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-include-mod.slang"
// CHECK-DAG: [[SOURCE_MOD:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[MOD_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_MOD:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_MOD]] %uint_{{[0-9]+}}

// The __include'd file has its own DebugSource but no compilation unit of its own.
// CHECK-DAG: [[INCL_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-included.slang"
// CHECK-DAG: [[SOURCE_INCL:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[INCL_FILE]] %{{[0-9]+}}

// The discriminating assertion: the __include'd function reports the __include'd file as its source,
// but must be scoped to [[CU_MOD]] — the compilation unit of the module that included it — and not to
// [[CU_ENTRY]], where the module-global fallback puts it when resolution fails.
// CHECK-DAG: [[FN_HELPER:%[0-9]+]] = OpString "helperFromIncludedFile"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_HELPER]] %{{[0-9]+}} [[SOURCE_INCL]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_MOD]]
