//TEST:SIMPLE(filecheck=CHECK): tests/spirv/debug-function-scope-ambiguous-other.slang -target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly -I tests/spirv

// When a header is included by two files of one translation unit, nothing recorded on a declaration
// says which expansion produced it — a declaration carries its source file, and both expansions share
// one source file. The resolution deliberately binds nothing in that case, leaving the pre-existing
// module-global scope, which is pinned to the entry point's compilation unit.
//
// This test asserts that fallback is *preserved*. It is the negative side of the feature: a scope
// naming a real but wrong file misleads a debugger more than a generic fallback does.
//
// Two placements make a wrong binding observable, and both are load-bearing:
//   - Each file defines a different macro before including the header, so each expansion declares a
//     different function with no redefinition error. That is what makes the answer genuinely
//     unrecoverable rather than merely awkward.
//   - This file is the *first* includer, so it is the compilation unit that a resolution taking the
//     first qualifying occurrence would pick, while the entry point — and therefore the fallback —
//     lives in the other file. With both in one file the wrong answer and the fallback would be the
//     same instruction and this test could not fail.

#define AMBIGUOUS_VARIANT_B
#include "debug-function-scope-ambiguous-inc.slang"

float ambiguousWrapB(float x)
{
    return ambiguousFromB(x);
}

// This file is the first includer of the shared header, so its compilation unit is what a
// first-occurrence resolution would wrongly bind.
// CHECK-DAG: [[FIRST_INCLUDER_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-ambiguous.slang"
// CHECK-DAG: [[SOURCE_FIRST:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[FIRST_INCLUDER_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_FIRST:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_FIRST]] %uint_{{[0-9]+}}

// The other file holds the entry point, so its compilation unit is what the module-global fallback
// resolves to.
// CHECK-DAG: [[ENTRY_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-ambiguous-other.slang"
// CHECK-DAG: [[SOURCE_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[ENTRY_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_ENTRY:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_ENTRY]] %uint_{{[0-9]+}}

// The shared header's own source, which has no compilation unit of its own.
// CHECK-DAG: [[INC_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-ambiguous-inc.slang"
// CHECK-DAG: [[SOURCE_INC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[INC_FILE]] %{{[0-9]+}}

// The assertion that pins the fallback: ambiguousFromB was declared by this file's expansion, but with
// the owning unit unrecoverable it keeps the module-global scope [[CU_ENTRY]] rather than [[CU_FIRST]].
// Binding [[CU_FIRST]] is what arbitrary first-occurrence selection produces, so this CHECK is what
// distinguishes the deliberate fallback from a guess.
// CHECK-DAG: [[FN_B:%[0-9]+]] = OpString "ambiguousFromB"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_B]] %{{[0-9]+}} [[SOURCE_INC]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_ENTRY]]
