//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
module mymodule;

// This tests the issue described here: https://github.com/shader-slang/slang/issues/5995

//CHECK: result code = 0
//CHECK: implementing directive contains file extension in module name 'mymodule.slang'
__include "implementing-with-ext";

int moduleFunction() { return 0; }
