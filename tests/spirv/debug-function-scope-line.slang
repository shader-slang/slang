//TEST:SIMPLE(filecheck=CHECK): tests/spirv/debug-function-scope-line-other.slang -target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly

// A `#line` directive remaps locations to a presumed path that is not one of the translation unit's
// source files, so the DebugSource emitted for that path has no DebugCompilationUnit. The function is
// still physically defined in a real file that does have one, so its DebugFunction must be scoped to
// that file's compilation unit rather than falling back to the module-global scope.
//
// This file holds only the entry point; debug-function-scope-line-other.slang holds the
// `#line`-remapped function. The separation is load-bearing: the module-global fallback is pinned to
// the *entry point's* compilation unit, so with the remapped function in this same file the fallback
// would coincide with the correct answer and the test could pass while the feature is broken.

float remappedHelper(float x);

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(remappedHelper(0.5));
}

// This file holds the entry point, so its compilation unit is the one the fallback is pinned to.
// CHECK-DAG: [[ENTRY_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-line.slang"
// CHECK-DAG: [[SOURCE_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[ENTRY_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_ENTRY]] %uint_{{[0-9]+}}

// The other top-level file physically holds the remapped function, and has its own compilation unit.
// CHECK-DAG: [[PHYS_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-line-other.slang"
// CHECK-DAG: [[SOURCE_PHYS:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[PHYS_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_PHYS:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_PHYS]] %uint_{{[0-9]+}}

// The `#line` presumed path gets its own DebugSource, identified here structurally rather than by
// filename. Two reasons the obvious `OpString "…-presumed.slang"` match does not work: the presumed
// file does not exist on disk, so the record carries no text operand (matching a trailing operand
// fails on arity); and slang-test rewrites embedded-source OpStrings to a placeholder, which clobbers
// the presumed filename string itself. What remains stable is the shape — it is the only DebugSource
// in this module with a single operand, every real file's record having both a filename and text.
// CHECK-DAG: [[SOURCE_PRESUMED:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource %{{[0-9]+$}}

// The discriminating assertion: the remapped function reports the presumed path as its source — that
// is what `#line` is for — but must be scoped to [[CU_PHYS]], the compilation unit of the file
// physically holding it, and not to [[CU_ENTRY]], where the fallback puts it when resolution fails.
// CHECK-DAG: [[FN_REMAPPED:%[0-9]+]] = OpString "remappedHelper"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_REMAPPED]] %{{[0-9]+}} [[SOURCE_PRESUMED]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_PHYS]]
