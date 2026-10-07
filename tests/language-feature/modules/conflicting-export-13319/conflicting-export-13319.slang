// Regression test for https://github.com/shader-slang/slang/issues/13319.
//
// When two modules both `export` a definition of one link-time symbol and no
// target-specialization rule prefers either, the linker keeps the definition
// it sees first, so the module order decides the program. The linker reports
// that with warning E45002, naming the module whose definition it selected
// and each other module.

//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/contract.slang -o tests/language-feature/modules/conflicting-export-13319/contract.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/renderer-a.slang -o tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/renderer-b.slang -o tests/language-feature/modules/conflicting-export-13319/renderer-b.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/override-default.slang -o tests/language-feature/modules/conflicting-export-13319/override-default.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/specialized-contract.slang -o tests/language-feature/modules/conflicting-export-13319/specialized-contract.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/specialized-generic.slang -o tests/language-feature/modules/conflicting-export-13319/specialized-generic.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/specialized-spirv.slang -o tests/language-feature/modules/conflicting-export-13319/specialized-spirv.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/specialized-hlsl-a.slang -o tests/language-feature/modules/conflicting-export-13319/specialized-hlsl-a.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/specialized-hlsl-b.slang -o tests/language-feature/modules/conflicting-export-13319/specialized-hlsl-b.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/witness-contract.slang -o tests/language-feature/modules/conflicting-export-13319/witness-contract.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/witness-a.slang -o tests/language-feature/modules/conflicting-export-13319/witness-a.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/witness-b.slang -o tests/language-feature/modules/conflicting-export-13319/witness-b.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/self-contained.slang -o tests/language-feature/modules/conflicting-export-13319/self-contained.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/builtin-contract.slang -o tests/language-feature/modules/conflicting-export-13319/builtin-contract.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/builtin-a.slang -o tests/language-feature/modules/conflicting-export-13319/builtin-a.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/builtin-b.slang -o tests/language-feature/modules/conflicting-export-13319/builtin-b.slang-module

// A struct alias, a `static const` and a function are each exported by both
// renderer modules. Overriding the extern default `kDefault` is not a conflict.
//TEST:SIMPLE_EX(filecheck=AB): tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-b.slang-module tests/language-feature/modules/conflicting-export-13319/override-default.slang-module -target spirv-asm
//TEST:SIMPLE_EX(filecheck=BA): tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/renderer-b.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/override-default.slang-module -target spirv-asm

// AB-NOT: kDefault
// AB-DAG: warning[E45002]: 'Renderer' is exported by more than one module; using the definition from module 'renderer-b'
// AB-DAG: note: another definition of 'Renderer' is exported by module 'renderer-a'
// AB-DAG: warning[E45002]: 'kCount' is exported by more than one module; using the definition from module 'renderer-b'
// AB-DAG: note: another definition of 'kCount' is exported by module 'renderer-a'
// AB-DAG: warning[E45002]: 'pick' is exported by more than one module; using the definition from module 'renderer-b'
// AB-DAG: note: another definition of 'pick' is exported by module 'renderer-a'
// AB-NOT: kDefault
// AB: OpConstant %uint 22
// AB: OpConstant %uint 202
// AB: OpConstant %uint 222
// AB: OpConstant %uint 7

// BA-DAG: warning[E45002]: 'Renderer' is exported by more than one module; using the definition from module 'renderer-a'
// BA-DAG: note: another definition of 'Renderer' is exported by module 'renderer-b'
// BA-DAG: warning[E45002]: 'kCount' is exported by more than one module; using the definition from module 'renderer-a'
// BA-DAG: note: another definition of 'kCount' is exported by module 'renderer-b'
// BA-DAG: warning[E45002]: 'pick' is exported by more than one module; using the definition from module 'renderer-a'
// BA-DAG: note: another definition of 'pick' is exported by module 'renderer-b'
// BA: OpConstant %uint 11
// BA: OpConstant %uint 101
// BA: OpConstant %uint 111
// BA: OpConstant %uint 7

// E45002 is an ordinary warning, so `-warnings-as-errors` makes the link fail.
//TEST:SIMPLE_EX(filecheck=WERROR): tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-b.slang-module -target spirv-asm -warnings-as-errors 45002
// WERROR: error[E45002]: 'Renderer' is exported by more than one module

// Controls with no conflict: one export overriding the extern default, and the
// same module listed twice. A losing module listed twice still gets one note.
//TEST:SIMPLE_EX(filecheck=CONTROL): tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/override-default.slang-module -target spirv-asm
// CONTROL-NOT: E45002
// CONTROL: OpConstant %uint 11
// CONTROL: OpConstant %uint 101
// CONTROL: OpConstant %uint 111
// CONTROL: OpConstant %uint 7
//TEST:SIMPLE_EX(filecheck=REPEAT): tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-b.slang-module -target spirv-asm
// REPEAT: warning[E45002]: 'Renderer' is exported by more than one module; using the definition from module 'renderer-b'
// REPEAT-NEXT: note: another definition of 'Renderer' is exported by module 'renderer-a'
// REPEAT-NOT: note: another definition of 'Renderer'

// A module given both as source and as its own precompiled form reaches the
// linker as two IR modules with one module name; that is not a conflict.
//TEST:SIMPLE_EX(filecheck=SELF): tests/language-feature/modules/conflicting-export-13319/self-contained.slang -target spirv-asm tests/language-feature/modules/conflicting-export-13319/self-contained.slang-module
// SELF-NOT: E45002
// SELF: OpConstant %uint 11

// A definition specialized for the requested target is preferred over a
// generic one, and a definition specialized for another target is never a
// candidate, so none of these links is ambiguous.
//TEST:SIMPLE_EX(filecheck=SPEC-SPIRV): tests/language-feature/modules/conflicting-export-13319/specialized-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/specialized-generic.slang-module tests/language-feature/modules/conflicting-export-13319/specialized-spirv.slang-module -target spirv-asm
// SPEC-SPIRV-NOT: E45002
// SPEC-SPIRV: OpConstant %uint 2
//TEST:SIMPLE_EX(filecheck=SPEC-HLSL): tests/language-feature/modules/conflicting-export-13319/specialized-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/specialized-generic.slang-module tests/language-feature/modules/conflicting-export-13319/specialized-spirv.slang-module -target hlsl
// SPEC-HLSL-NOT: E45002
// SPEC-HLSL: return 1
//TEST:SIMPLE_EX(filecheck=SPEC-OTHER): tests/language-feature/modules/conflicting-export-13319/specialized-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/specialized-generic.slang-module tests/language-feature/modules/conflicting-export-13319/specialized-hlsl-a.slang-module tests/language-feature/modules/conflicting-export-13319/specialized-hlsl-b.slang-module -target spirv-asm
// SPEC-OTHER-NOT: E45002
// SPEC-OTHER: OpConstant %uint 1

// Two definitions specialized for the same target do conflict on that target.
//TEST:SIMPLE_EX(filecheck=SPEC-SAME): tests/language-feature/modules/conflicting-export-13319/specialized-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/specialized-hlsl-a.slang-module tests/language-feature/modules/conflicting-export-13319/specialized-hlsl-b.slang-module -target hlsl
// SPEC-SAME: warning[E45002]: 'pickForTarget' is exported by more than one module; using the definition from module 'specialized-hlsl-a'
// SPEC-SAME: note: another definition of 'pickForTarget' is exported by module 'specialized-hlsl-b'
// SPEC-SAME: return 3

// A conformance exported by two modules for a type that neither defines is
// reported as the conformance. A type defined directly by two modules is
// reported once, as the type, and not again for its conformance witness. Its
// members are linked separately from the type, which the warning also says, so
// the test does not check which module's members are used.
//TEST:SIMPLE_EX(filecheck=WITNESS): tests/language-feature/modules/conflicting-export-13319/witness-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/witness-a.slang-module tests/language-feature/modules/conflicting-export-13319/witness-b.slang-module -target spirv-asm
// WITNESS-NOT: Renderer : IRenderer
// WITNESS-DAG: warning[E45002]: 'Thing : IValue' is exported by more than one module; using the definition from module 'witness-a'
// WITNESS-DAG: note: another definition of 'Thing : IValue' is exported by module 'witness-b'
// WITNESS-DAG: warning[E45002]: 'Renderer' is exported by more than one module
// WITNESS-DAG: note: another definition of 'Renderer' is exported by module
// WITNESS-DAG: note: the methods and fields of 'Renderer' are linked separately
// WITNESS-NOT: Renderer : IRenderer
// WITNESS: OpConstant %uint 11

// Conformances of a builtin type and of a generic type are reported the same way.
//TEST:SIMPLE_EX(filecheck=BUILTIN): tests/language-feature/modules/conflicting-export-13319/builtin-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/builtin-a.slang-module tests/language-feature/modules/conflicting-export-13319/builtin-b.slang-module -target spirv-asm
// BUILTIN-DAG: warning[E45002]: 'float : IValue' is exported by more than one module; using the definition from module 'builtin-a'
// BUILTIN-DAG: note: another definition of 'float : IValue' is exported by module 'builtin-b'
// BUILTIN-DAG: warning[E45002]: 'Box : IValue' is exported by more than one module; using the definition from module 'builtin-a'
// BUILTIN-DAG: note: another definition of 'Box : IValue' is exported by module 'builtin-b'
