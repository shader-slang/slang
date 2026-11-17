//DIAGNOSTIC_TEST:SIMPLE(diag=diag,exit-code=0):-E
//TEST:SIMPLE(filecheck=CHECK):-E

// Undefining a built-in macro with no user-level definition in effect
// is a no-op: the built-in stays available.

#undef __LINE__

__LINE__

#define __LINE__ "Shadow"
//diag: ^^^^^^^^ builtin macro redefined
//diag: ^^^^^^^^ Redefinition of builtin macro '__LINE__'

__LINE__

#undef __LINE__

__LINE__

// CHECK: 9
// CHECK: "Shadow"
// CHECK: 19
