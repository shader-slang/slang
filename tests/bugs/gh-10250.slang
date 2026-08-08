//DIAGNOSTIC_TEST:SIMPLE: -E

// Regression test: SIGSEGV in preprocessor on recursive/malformed macro definitions
// Issue: https://github.com/shader-slang/slang/issues/10250
// Found by AFL++ fuzzing

#define mo
#define mo(e mo(
