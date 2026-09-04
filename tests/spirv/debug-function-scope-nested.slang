//TEST:SIMPLE(filecheck=CHECK): tests/spirv/debug-function-scope-nested-owner.slang -target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly -I tests/spirv

// A header can be reached by several include chains that diverge and then reconverge on one outer
// file: here debug-function-scope-nested-owner.slang includes two intermediate files, and both of
// those include the same leaf header. The leaf therefore has two immediate includers but only one
// non-included file at the end of every chain, so its owning compilation unit is determined and must
// be resolved rather than abandoned as ambiguous.
//
// This file holds only the entry point, so the owner's compilation unit and the module-global
// fallback are different instructions. Moving the entry point into the owner would make them the same
// and this test could then pass without resolving anything.

float nestedOwnerFn(float x);

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(nestedOwnerFn(0.5));
}

// This file holds the entry point, so its compilation unit is what the module-global fallback
// resolves to.
// CHECK-DAG: [[ENTRY_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-nested.slang"
// CHECK-DAG: [[SOURCE_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[ENTRY_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_ENTRY]] %uint_{{[0-9]+}}

// The outer file every chain terminates in, and the compilation unit that owns the leaf's functions.
// CHECK-DAG: [[OWNER_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-nested-owner.slang"
// CHECK-DAG: [[SOURCE_OWNER:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[OWNER_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_OWNER:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_OWNER]] %uint_{{[0-9]+}}

// The leaf header's own source, which has no compilation unit of its own.
// CHECK-DAG: [[LEAF_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-nested-leaf.slang"
// CHECK-DAG: [[SOURCE_LEAF:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[LEAF_FILE]] %{{[0-9]+}}

// The discriminating assertion: the leaf's function is scoped to [[CU_OWNER]], the single file all
// chains terminate in, and not to [[CU_ENTRY]], which is where it lands if divergence at the
// intermediate level is mistaken for an undetermined owner.
// CHECK-DAG: [[FN_LEAF:%[0-9]+]] = OpString "nestedLeafFn"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_LEAF]] %{{[0-9]+}} [[SOURCE_LEAF]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_OWNER]]
