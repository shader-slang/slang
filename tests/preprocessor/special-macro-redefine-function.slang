//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-E
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

// CHECK: "Is" "Anybody" "Is" There __FILE__
