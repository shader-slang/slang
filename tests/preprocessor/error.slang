//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// #error support

#define FIRST 0

#if FIRST
#error Not this one!
#else

#error This isn't valid!
/*diag:
 ^ preprocessor error
 ^ #error: This isn't valid!
*/

#endif
