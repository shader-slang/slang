//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_FAIL): -target glsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_PASS): -target spirv -entry computeMain -stage compute

// Test that __getAddress correctly reports capabilities.

uniform int* outputBuffer;
uniform int* buffer;

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error

void computeMain()
/*CHECK_FAIL:
     ^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^ entrypoint 'computeMain' uses features that are not available in 'compute' stage for 'glsl' compilation target.
*/
{
    outputBuffer[0] = *(__getAddress(buffer[0]));
//CHECK_FAIL:           ^^^^^^^^^^^^ see using of 'expression'
}