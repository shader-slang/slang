//DIAGNOSTIC_TEST:SIMPLE(diag=diag,exit-code=0):-E
//TEST:SIMPLE(filecheck=CHECK):-E

__LINE__

#define __LINE__ "Hello"
//diag: ^^^^^^^^ builtin macro redefined
//diag: ^^^^^^^^ Redefinition of builtin macro '__LINE__'

__LINE__

#undef __LINE__

__LINE__

// CHECK: 6
// CHECK: "Hello"
// CHECK: 16
