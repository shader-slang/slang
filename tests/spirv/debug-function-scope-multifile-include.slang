// Regression guard for shader-slang/slang#12150 with more than one non-included source file in the
// translation unit. This file holds the entry point; debug-function-scope-multifile-other.slang is
// the file that does the #include. Both are compiled as top-level (non-included) files, so each gets
// its own DebugCompilationUnit.
//
// That separation is what gives this test power, and it is not incidental. When a DebugFunction's
// owning compilation unit cannot be resolved, SPIR-V emission falls back to a module-global scope
// pinned to the *entry point's* compilation unit. If the entry point lived in the same file that does
// the #include, the includer's compilation unit would BE the entry point's, the broken fallback would
// coincide with the correct answer, and this test would pass while the feature is broken — which is
// exactly what happened before this file was restructured. Keeping the entry point here and the
// #include in the other file makes includer-CU and entry-CU distinct, so only real resolution can
// satisfy the final assertion below.

//TEST:SIMPLE(filecheck=CHECK): tests/spirv/debug-function-scope-multifile-other.slang -target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly

// Defined in debug-function-scope-multifile-other.slang, which #includes the helper.
float multifileOtherFunc(float x);

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(multifileOtherFunc(0.25));
}

// This file holds the entry point, so its compilation unit is also the one the module-global
// fallback is pinned to.
// CHECK-DAG: [[ENTRY_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-multifile-include.slang"
// CHECK-DAG: [[SOURCE_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[ENTRY_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_ENTRY]] %uint_{{[0-9]+}}

// The other top-level file gets its own, distinct compilation unit. It is the file that #includes
// the helper, so it is the compilation unit that owns the helper.
// CHECK-DAG: [[INCLUDER_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-multifile-other.slang"
// CHECK-DAG: [[SOURCE_INCLUDER:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[INCLUDER_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_INCLUDER:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_INCLUDER]] %uint_{{[0-9]+}}

// The #include'd helper's own source, which has no compilation unit of its own.
// CHECK-DAG: [[INC_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-multifile-inc.slang"
// CHECK-DAG: [[SOURCE_INC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[INC_FILE]] %{{[0-9]+}}

// The entry point stays on its own compilation unit.
// CHECK-DAG: [[FN_MAIN:%[0-9]+]] = OpString "main"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_MAIN]] %{{[0-9]+}} [[SOURCE_ENTRY]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_ENTRY]]

// The discriminating assertion: the included helper must be scoped to [[CU_INCLUDER]], the
// compilation unit of the file that included it — not to [[CU_ENTRY]], which is where the
// module-global fallback puts it when resolution fails.
// CHECK-DAG: [[FN_HELPER:%[0-9]+]] = OpString "multifileHelperFromInclude"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_HELPER]] %{{[0-9]+}} [[SOURCE_INC]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_INCLUDER]]
