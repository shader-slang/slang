//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly

// A `#line` directive remaps locations to a presumed path that is not one of the translation
// unit's source files, so the DebugSource emitted for that path has no DebugCompilationUnit of its
// own. The function is nevertheless physically defined in this file, which does have one, so its
// DebugFunction must be scoped to this file's compilation unit rather than falling back to the
// module-global scope.

RWStructuredBuffer<float> outputBuffer;

#line 1 "debug-function-scope-line-presumed.slang"
[noinline] float helperInRemappedSource(float value)
{
    return value * 2.0;
}

#line 2000
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = helperInRemappedSource(0.5);
}

// This file is a non-included source, so it gets a DebugSource and a DebugCompilationUnit.
// CHECK-DAG: [[PHYS_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-line.slang"
// CHECK-DAG: [[SOURCE_PHYS:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[PHYS_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_PHYS:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_PHYS]] %uint_{{[0-9]+}}

// The `#line` presumed path gets its own DebugSource, and no compilation unit references it.
// CHECK-DAG: [[PRESUMED_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-line-presumed.slang"
// CHECK-DAG: [[SOURCE_PRESUMED:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[PRESUMED_FILE]] %{{[0-9]+}}

// The remapped function reports the presumed path as its source (operand 3) — that is what `#line`
// is for — but its scope operand must be this file's compilation unit, resolved from the physical
// SourceLoc rather than from the remapped source.
// CHECK-DAG: [[FN_REMAPPED:%[0-9]+]] = OpString "helperInRemappedSource"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_REMAPPED]] %{{[0-9]+}} [[SOURCE_PRESUMED]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_PHYS]]
