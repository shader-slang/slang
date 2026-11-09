// Debugger support is temporarily non-functional while debug-scope tracking is redesigned.
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -g2 -validate-ir

// Keep this coverage compiler-only. The included test also has a dispatcher test, but this test is
// specifically responsible for validating debug scopes through lambda lowering and autodiff.
#include "differentiable-lambda.slang"

// CHECK: OpStringLit {{[0-9]+}} "evaluateBackward"
// CHECK: Section("META")
// CHECK: OpDebugLexicalBlock
// CHECK: OpSlangEntryPoint {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}}
