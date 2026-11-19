//DIAGNOSTIC_TEST:SIMPLE(diag=diag,exit-code=0):-E
//TEST:SIMPLE(filecheck=CHECK):-E

//
// Check replacing __FILE__
//

#define __FILE__(x) "Is" x
//diag: ^^^^^^^^ builtin macro redefined
//diag: ^^^^^^^^ Redefinition of builtin macro '__FILE__'

__FILE__("Anybody")
__FILE__(There)

#undef __FILE__

__FILE__

// The last item is `__FILE__` expanding to this file's invocation-relative
// path, as the test harness invokes the compiler from the repository root.
// CHECK: "Is" "Anybody" "Is" There "tests/preprocessor/special-macro-redefine-function.slang"
