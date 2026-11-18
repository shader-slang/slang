//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -E

// The preprocessor must suppress diagnostics before the lexer reads a disabled line.
#if 0
"disabled
#endif

// After taking a branch, the preprocessor suppresses diagnostics before reading an
// alternative introduced by `#else` or `#elif`.
#if 1
#else
"disabled_after_else
#endif

#if 1
#elif 1
"disabled_after_elif
#endif

// The preprocessor must enable diagnostics before reading a branch selected by `#else` or `#elif`.
#if 0
#else
"enabled_after_else
/*CHECK:
                   ^ newline in literal
*/
#endif

#if 0
#elif 1
"enabled_after_elif
/*CHECK:
                   ^ newline in literal
*/
#endif

// Under a disabled outer branch, the preprocessor must keep diagnostics suppressed
// after processing an inner `#elif`, `#else`, or `#endif`. It must restore diagnostics
// after processing the outer `#endif`, before reading the following line.
#if 0
#if 0
#elif 1
"disabled_after_inner_elif
#else
"disabled_after_inner_else
#endif
"disabled_after_inner_endif
#endif
"enabled
/*CHECK:
        ^ newline in literal
*/
