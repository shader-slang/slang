// Regression guard for shader-slang/slang#11983, separate-compilation path.
//
// The DebugFunction scope operand (issue #11983) is an IR operand, so it must survive
// serialization: a module precompiled to a .slang-module ships as an IR blob whose IRDebugFunction
// is deserialized into the consumer. This test precompiles a module with -g2, then consumes it via
// -r and checks that the separately-compiled function's DebugFunction is scoped to the *module's*
// own DebugCompilationUnit — not the consumer entry point's — proving the operand round-trips.

//TEST:COMPILE: tests/spirv/debug-function-scope-separate-compilation-module.slang -g2 -o tests/spirv/debug-function-scope-separate-compilation-module.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/spirv/debug-function-scope-separate-compilation-module.slang-module -target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly

import "debug-function-scope-separate-compilation-module";

[shader("fragment")]
float4 main() : SV_Target
{
    return getSeparatelyCompiledDebugValue(0.5);
}

// Capture the separately-compiled function's own source and parent scope directly from its
// DebugFunction record. Anchoring on the function record (rather than on a filename OpString) is
// unambiguous: a precompiled module contributes more than one DebugSource for its file (a
// filename-only record carried in the blob plus a full-path record), and only the record the
// function actually references is the one that has a compilation unit.
// CHECK-DAG: [[FN_MODULE:%[0-9]+]] = OpString "getSeparatelyCompiledDebugValue"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_MODULE]] %{{[0-9]+}} [[SRC_MODULE:%[0-9]+]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_MODULE:%[0-9]+]]

// That parent scope must be a DebugCompilationUnit that wraps the function's own source — i.e. the
// module's compilation unit, reconstructed from the serialized .slang-module. This is the core
// round-trip assertion: the #11983 parentScope operand survived serialization and still points at
// the module's own compilation unit.
// CHECK-DAG: [[CU_MODULE]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SRC_MODULE]] %uint_{{[0-9]+}}

// The consumer's entry point is scoped to a *different* source and compilation unit, confirming the
// module function's scope is genuinely the module's and was not reassigned to the consumer's CU.
// Capturing both from their own DebugFunction records forces [[SRC_MODULE]]/[[SRC_CONSUMER]] (and
// hence [[CU_MODULE]]/[[CU_CONSUMER]]) to distinct records.
// CHECK-DAG: [[FN_MAIN:%[0-9]+]] = OpString "main"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_MAIN]] %{{[0-9]+}} [[SRC_CONSUMER:%[0-9]+]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_CONSUMER:%[0-9]+]]
// CHECK-DAG: [[CU_CONSUMER]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SRC_CONSUMER]] %uint_{{[0-9]+}}
