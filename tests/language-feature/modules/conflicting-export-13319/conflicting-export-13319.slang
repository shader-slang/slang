// Regression test for https://github.com/shader-slang/slang/issues/13319.
//
// When two modules both `export` a definition of one link-time symbol and no
// target-specialization rule prefers either, the order of the modules in the
// link decides which definition the linker selects. The linker reports that
// with warning E45002, naming the module whose definition it selected and each
// other module.

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
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/renderer-c.slang -o tests/language-feature/modules/conflicting-export-13319/renderer-c.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/extension-owner.slang -o tests/language-feature/modules/conflicting-export-13319/extension-owner.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/extension-other.slang -o tests/language-feature/modules/conflicting-export-13319/extension-other.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/generic-contract.slang -o tests/language-feature/modules/conflicting-export-13319/generic-contract.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/generic-a.slang -o tests/language-feature/modules/conflicting-export-13319/generic-a.slang-module
//TEST:COMPILE: tests/language-feature/modules/conflicting-export-13319/generic-b.slang -o tests/language-feature/modules/conflicting-export-13319/generic-b.slang-module

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

// The contract module can come anywhere in the link; here the renderer
// modules come first, and the earlier of them is selected.
//TEST:SIMPLE_EX(filecheck=LAST): tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-b.slang-module tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain -target spirv-asm
// LAST-DAG: warning[E45002]: 'Renderer' is exported by more than one module; using the definition from module 'renderer-a'
// LAST-DAG: note: another definition of 'Renderer' is exported by module 'renderer-b'
// LAST: OpConstant %uint 11

// With three exporting modules, the warning has a note for each module that
// was not selected.
//TEST:SIMPLE_EX(filecheck=THREE): tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-b.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-c.slang-module -target spirv-asm
// THREE: warning[E45002]: 'kCount' is exported by more than one module; using the definition from module 'renderer-c'
// THREE-DAG: note: another definition of 'kCount' is exported by module 'renderer-a'
// THREE-DAG: note: another definition of 'kCount' is exported by module 'renderer-b'
// THREE: OpConstant %uint 303

// E45002 is an ordinary warning, so `-warnings-as-errors` makes the link fail,
// and `-warnings-disable` silences it together with its notes.
//TEST:SIMPLE_EX(filecheck=WERROR): tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-b.slang-module -target spirv-asm -warnings-as-errors 45002
// WERROR: error[E45002]: 'Renderer' is exported by more than one module
//TEST:SIMPLE_EX(filecheck=WDISABLE): tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-b.slang-module -target spirv-asm -warnings-disable 45002
// WDISABLE-NOT: E45002
// WDISABLE-NOT: another definition of
// WDISABLE: OpConstant %uint 22

// Controls with no conflict: one export overriding the extern default, and the
// same module listed twice.
//TEST:SIMPLE_EX(filecheck=CONTROL): tests/language-feature/modules/conflicting-export-13319/contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/renderer-a.slang-module tests/language-feature/modules/conflicting-export-13319/override-default.slang-module -target spirv-asm
// CONTROL-NOT: E45002
// CONTROL: OpConstant %uint 11
// CONTROL: OpConstant %uint 101
// CONTROL: OpConstant %uint 111
// CONTROL: OpConstant %uint 7

// A module that was not selected but is listed twice gets one note.
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
// methods are linked separately from the type, which the warning also says, so
// the test does not check which module's methods are used.
//TEST:SIMPLE_EX(filecheck=WITNESS): tests/language-feature/modules/conflicting-export-13319/witness-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/witness-a.slang-module tests/language-feature/modules/conflicting-export-13319/witness-b.slang-module -target spirv-asm
// WITNESS-DAG: warning[E45002]: 'Thing : IValue' is exported by more than one module; using the definition from module 'witness-a'
// WITNESS-DAG: note: another definition of 'Thing : IValue' is exported by module 'witness-b'
// WITNESS-DAG: warning[E45002]: 'Renderer' is exported by more than one module
// WITNESS-DAG: note: another definition of 'Renderer' is exported by module
// WITNESS-DAG: note: the methods of 'Renderer' are linked separately
// WITNESS: OpConstant %uint 11
//TEST:SIMPLE_EX(filecheck=WITNESS-ONCE): tests/language-feature/modules/conflicting-export-13319/witness-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/witness-a.slang-module tests/language-feature/modules/conflicting-export-13319/witness-b.slang-module -target spirv-asm
// WITNESS-ONCE-NOT: Renderer : IRenderer

// A conformance exported both by the module that exports its type and by
// another module is reported in either link order, even though the type itself
// has one definition.
//TEST:SIMPLE_EX(filecheck=OWNER-FIRST): tests/language-feature/modules/conflicting-export-13319/extension-owner.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/extension-other.slang-module -target spirv-asm
// OWNER-FIRST: warning[E45002]: 'Thing : IValue' is exported by more than one module; using the definition from module 'extension-owner'
// OWNER-FIRST: note: another definition of 'Thing : IValue' is exported by module 'extension-other'
// OWNER-FIRST: OpConstant %uint 11
//TEST:SIMPLE_EX(filecheck=OWNER-LAST): tests/language-feature/modules/conflicting-export-13319/extension-other.slang-module tests/language-feature/modules/conflicting-export-13319/extension-owner.slang-module -entry computeMain -target spirv-asm
// OWNER-LAST: warning[E45002]: 'Thing : IValue' is exported by more than one module; using the definition from module 'extension-other'
// OWNER-LAST: note: another definition of 'Thing : IValue' is exported by module 'extension-owner'
// OWNER-LAST: OpConstant %uint 22

// Conformances of a builtin type and of a generic type are reported the same way.
//TEST:SIMPLE_EX(filecheck=BUILTIN): tests/language-feature/modules/conflicting-export-13319/builtin-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/builtin-a.slang-module tests/language-feature/modules/conflicting-export-13319/builtin-b.slang-module -target spirv-asm
// BUILTIN-DAG: warning[E45002]: 'float : IValue' is exported by more than one module; using the definition from module 'builtin-a'
// BUILTIN-DAG: note: another definition of 'float : IValue' is exported by module 'builtin-b'
// BUILTIN-DAG: warning[E45002]: 'Box : IValue' is exported by more than one module; using the definition from module 'builtin-a'
// BUILTIN-DAG: note: another definition of 'Box : IValue' is exported by module 'builtin-b'

// A generic link-time type and a generic type defined by two modules are
// reported by name, and the generic type is reported once, as the type.
//TEST:SIMPLE_EX(filecheck=GENERIC): tests/language-feature/modules/conflicting-export-13319/generic-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/generic-a.slang-module tests/language-feature/modules/conflicting-export-13319/generic-b.slang-module -target spirv-asm
// GENERIC-DAG: warning[E45002]: 'GenericRenderer' is exported by more than one module; using the definition from module 'generic-b'
// GENERIC-DAG: note: another definition of 'GenericRenderer' is exported by module 'generic-a'
// GENERIC-DAG: warning[E45002]: 'Box' is exported by more than one module; using the definition from module 'generic-a'
// GENERIC-DAG: note: another definition of 'Box' is exported by module 'generic-b'
// GENERIC-DAG: note: the methods of 'Box' are linked separately
//TEST:SIMPLE_EX(filecheck=GENERIC-ONCE): tests/language-feature/modules/conflicting-export-13319/generic-contract.slang-module -entry computeMain tests/language-feature/modules/conflicting-export-13319/generic-a.slang-module tests/language-feature/modules/conflicting-export-13319/generic-b.slang-module -target spirv-asm
// GENERIC-ONCE-NOT: Box : IValue
// GENERIC-ONCE-NOT: '_ST
// GENERIC-ONCE-NOT: the methods of 'GenericRenderer'
