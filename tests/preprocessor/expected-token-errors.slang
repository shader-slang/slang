//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Test preprocessor expected token diagnostics
// This tests the Expect/ExpectRaw helper functions in slang-preprocessor.cpp

// Test 15200: expected token in preprocessor expression (missing closing paren)
#if (1 + 2
/*diag:
          ^ preprocessor parse error
          ^ expected '')'' in preprocessor expression
    ^ opening '('
*/
#endif

// Test 15203: expected token in defined expression (missing closing paren)
#if defined(FOO
/*diag:
               ^ preprocessor parse error
               ^ expected '')'' in 'defined' expression
           ^ opening '('
*/
#endif

// Test 15403: expected token in macro parameters (missing closing paren)
#define BROKEN_MACRO(a,
/*diag:
                       ^ preprocessor parse error
                       ^ expected 'identifier' in macro parameters
*/
