// simple-module.slang
// A simple module that exports a function for testing module import

module simple_module;

// Include implementation file (TestFunction is defined here)
__include "simple-module-impl";

// TestFunctionB is defined directly in this file
public int TestFunctionB(int x)
{
    return x + 200;
}
