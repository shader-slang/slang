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

// Capture a DebugCompilationUnit together with the source it wraps. We deliberately bind the
// source from the compilation unit itself rather than from a filename OpString: a precompiled
// module contributes more than one DebugSource for its file (a filename-only record carried in the
// blob plus the full-path record), and only the one the CU wraps is the function's own scope, so
// keying off the filename would bind the wrong record.
// CHECK-DAG: [[CU_MODULE:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SRC_MODULE:%[0-9]+]] %uint_{{[0-9]+}}

// The separately-compiled function's DebugFunction must reference that same source (operand 3) AND
// be scoped to that same compilation unit (the trailing scope operand). Because the source and CU
// are captured as a matched pair, this asserts the function is scoped to the compilation unit of
// its own source — i.e. the module's, not the consumer entry point's — proving the #11983 scope
// operand survived serialization into the .slang-module and back.
// CHECK-DAG: [[FN_MODULE:%[0-9]+]] = OpString "getSeparatelyCompiledDebugValue"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_MODULE]] %{{[0-9]+}} [[SRC_MODULE]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_MODULE]]
