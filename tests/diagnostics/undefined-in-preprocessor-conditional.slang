//DIAGNOSTIC_TEST(smoke):SIMPLE(diag=diag):

// Use an undefined identifier in a preprocessor conditional

#define FOO  1
#define BORT 1

#if FOO && BART
//diag:    ^^^^ undefined identifier in preprocessor
//diag:    ^^^^ undefined identifier 'BART' in preprocessor expression will evaluate to zero
#error Should not get here
#endif
