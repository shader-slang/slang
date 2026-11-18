//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -E

// Directive messages accept unmatched quotes without lexer diagnostics, even when the
// message starts immediately after the directive name. Normal lexing resumes on the next line.
#warning"warning message
"after_warning
/*CHECK:
              ^ newline in literal
*/
//CHECK: preprocessor warning
//CHECK: #warning: "warning message

#error"error message
"after_error
/*CHECK:
            ^ newline in literal
*/
//CHECK: preprocessor error
//CHECK: #error: "error message
