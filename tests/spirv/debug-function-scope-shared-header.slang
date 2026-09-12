//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly -I tests/spirv

// A header included by two different modules has more than one recorded include occurrence in the
// shared source manager, so identifying it by source file alone is ambiguous: whichever occurrence
// was registered first would win, and a function from the header would be scoped to that module's
// compilation unit even while lowering the other module.
//
// This file holds only the entry point. Both including modules are separate from it, which is what
// makes the test able to fail in both directions: the module-global fallback is pinned to the entry
// point's compilation unit, so if the entry point shared a file with either includer, "fell back to
// the entry point" and "resolved to that includer" would be the same instruction in the output and
// indistinguishable.

import debug_function_scope_shared_header_a;
import debug_function_scope_shared_header_b;

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(wrapInA(0.25) + wrapInB(0.5));
}

// Three distinct compilation units: the entry point's and one per including module.
// CHECK-DAG: [[ENTRY_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-shared-header.slang"
// CHECK-DAG: [[SOURCE_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[ENTRY_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_ENTRY]] %uint_{{[0-9]+}}

// CHECK-DAG: [[A_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-shared-header-a.slang"
// CHECK-DAG: [[SOURCE_A:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[A_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_A:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_A]] %uint_{{[0-9]+}}

// CHECK-DAG: [[B_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-shared-header-b.slang"
// CHECK-DAG: [[SOURCE_B:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[B_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_B:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_B]] %uint_{{[0-9]+}}

// Each module's own function is scoped to that module's compilation unit.
// CHECK-DAG: [[FN_A:%[0-9]+]] = OpString "wrapInA"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_A]] %{{[0-9]+}} [[SOURCE_A]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_A]]
// CHECK-DAG: [[FN_B:%[0-9]+]] = OpString "wrapInB"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_B]] %{{[0-9]+}} [[SOURCE_B]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_B]]

// The discriminating assertions. The shared header contributes one DebugFunction per importing
// module, and each must carry the compilation unit of the module that included it — one on [[CU_A]]
// and one on [[CU_B]]. Resolving by source file alone puts both on whichever occurrence was seen
// first; falling back puts both on [[CU_ENTRY]]. Only per-occurrence resolution satisfies both.
// CHECK-DAG: [[HDR_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-shared-header-inc.slang"
// CHECK-DAG: [[SOURCE_HDR:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[HDR_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[FN_SHARED:%[0-9]+]] = OpString "sharedFromHeader"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_SHARED]] %{{[0-9]+}} [[SOURCE_HDR]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_A]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_SHARED]] %{{[0-9]+}} [[SOURCE_HDR]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_B]]
