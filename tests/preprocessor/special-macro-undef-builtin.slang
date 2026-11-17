//DIAGNOSTIC_TEST:SIMPLE(diag=diag,exit-code=0):-E
//TEST:SIMPLE(filecheck=CHECK):-E

// Undefining a built-in macro with no user-level definition in effect is a
// no-op, and a built-in returns after `#undef` of a user shadow: the cycle
// below shadows and restores `__LINE__` twice, then redefines an existing
// shadow, checking each expansion and each redefinition warning.

#undef __LINE__

__LINE__

#define __LINE__ "Shadow"
//diag: ^^^^^^^^ builtin macro redefined
//diag: ^^^^^^^^ Redefinition of builtin macro '__LINE__'

__LINE__

#undef __LINE__

__LINE__

#define __LINE__ "Again"
//diag: ^^^^^^^^ builtin macro redefined
//diag: ^^^^^^^^ Redefinition of builtin macro '__LINE__'

__LINE__

#undef __LINE__

__LINE__

#ifdef __LINE__
LINE_BUILTIN_VISIBLE
#endif

#undef __FILE__

#if defined(__FILE__)
FILE_BUILTIN_VISIBLE
#endif

#define __LINE__ "One"
//diag: ^^^^^^^^ builtin macro redefined
//diag: ^^^^^^^^ Redefinition of builtin macro '__LINE__'
//diag: ^^^^^^^^ see previous definition of '__LINE__'

#define __LINE__ "Two"
//diag: ^^^^^^^^ macro redefined
//diag: ^^^^^^^^ redefinition of macro '__LINE__'

__LINE__

#undef __LINE__

__LINE__

// CHECK: standard output = {
// CHECK: 11
// CHECK: "Shadow"
// CHECK: 21
// CHECK: "Again"
// CHECK: 31
// CHECK: LINE_BUILTIN_VISIBLE
// CHECK: FILE_BUILTIN_VISIBLE
// CHECK: "Two"
// CHECK: 56
