//DIAGNOSTIC_TEST:SIMPLE:

// Regression test: direction qualifiers in func syntax crash semantic checker
// Issue: https://github.com/shader-slang/slang/issues/10239

func modify(x: inout int) { x = x + 1; }
