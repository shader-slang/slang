//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Regression test for https://github.com/shader-slang/slang/issues/9200
// When an overload group is used where a type is expected, the diagnostic
// should include the identifier name to help the user locate the problem.

int MyFunc(int x) { return x; }
float MyFunc(float x) { return x; }

void test(MyFunc v) {}
//CHECK:  ^^^^^^ expected a type
//CHECK:  ^^^^^^ expected a type, got a 'overload group' for 'MyFunc'
