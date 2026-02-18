//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,diag=diag):-E

//
// Check replacing __LINE__
//

#define __LINE__ "Hello"
//diag: ^ builtin macro redefined
//diag: ^ Redefinition of builtin macro '__LINE__'

__LINE__

#undef __LINE__

__LINE__

// CHECK: "Hello"
// CHECK: 14
