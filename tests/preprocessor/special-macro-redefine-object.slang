//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-E
// TODO(#14354): This line is disabled, because the preprocessor does not restore a builtin macro
// after `#undef`. The last `__LINE__` is emitted as it is, and `CHECK: 14` fails.
//DISABLE_TEST:SIMPLE(filecheck=CHECK):-E

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
