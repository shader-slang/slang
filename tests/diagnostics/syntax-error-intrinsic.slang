// syntax-error-intrinsic.slang

//DIAGNOSTIC_TEST:SIMPLE:-target spirv -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE:-target dxil -profile cs_6_0 -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE:-target dxbc -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE:-target dll -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE:-target ptx -entry computeMain

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain()
{
    // Will output what downstream compilers will output as a syntax 
    // error.
    __SyntaxError();
}