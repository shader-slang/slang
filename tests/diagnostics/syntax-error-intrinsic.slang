// syntax-error-intrinsic.slang

//DIAGNOSTIC_TEST:SIMPLE_LINE:-entry computeMain -target spirv 
//DIAGNOSTIC_TEST:SIMPLE_LINE:-entry computeMain -target dxil -profile cs_6_0 
//DIAGNOSTIC_TEST:SIMPLE_LINE:-entry computeMain -target dxbc 
//DIAGNOSTIC_TEST:SIMPLE_LINE:-entry computeMain -target dll 
//DIAGNOSTIC_TEST:SIMPLE_LINE:-entry computeMain -target ptx 

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain()
{
    // Will output what downstream compilers will output as a syntax 
    // error.
    __SyntaxError();
}