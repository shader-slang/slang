//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// #warning support

#define FIRST 1

#if FIRST

#warning You wouldn't like me when I'm angry...
/*diag:
 ^^^^^^^ preprocessor warning
 ^^^^^^^ #warning: You wouldn't like me when I'm angry...
*/

#else

#warning Not this one!

#endif
