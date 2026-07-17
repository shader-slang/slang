// Regression guard for shader-slang/slang#11983 with more than one non-included source file in the
// translation unit. This file and debug-function-scope-multifile-other.slang are both compiled as
// top-level (non-included) files, so each gets its own DebugCompilationUnit. This file additionally
// #includes a helper file. The included helper's function must be scoped to THIS file's
// compilation unit (the file that included it), not the other top-level file's — resolving the
// owning compilation unit by include chain rather than by first-emitted compilation unit.

//TEST:SIMPLE(filecheck=CHECK): tests/spirv/debug-function-scope-multifile-other.slang -target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly

#include "debug-function-scope-multifile-inc.slang"

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(multifileHelperFromInclude(0.5) + multifileOtherFunc(0.25));
}

// This file's own source and compilation unit (it is a non-included, top-level file).
// CHECK-DAG: [[THIS_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-multifile-include.slang"
// CHECK-DAG: [[SOURCE_THIS:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[THIS_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_THIS:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_THIS]] %uint_{{[0-9]+}}

// The included helper's source (which has no compilation unit of its own).
// CHECK-DAG: [[INC_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-multifile-inc.slang"
// CHECK-DAG: [[SOURCE_INC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[INC_FILE]] %{{[0-9]+}}

// The included helper's DebugFunction references the include's source but is scoped to THIS file's
// compilation unit [[CU_THIS]] — the file that included it — even though the other top-level file's
// compilation unit is emitted first.
// CHECK-DAG: [[FN_HELPER:%[0-9]+]] = OpString "multifileHelperFromInclude"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_HELPER]] %{{[0-9]+}} [[SOURCE_INC]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_THIS]]
