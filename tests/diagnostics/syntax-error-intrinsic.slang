// syntax-error-intrinsic.slang

// NOTE! That although this is a 'diagnostic' like test, it tests using downstream compiler
// the downstream compiler being present is a requirement, so we mark as a 'TEST' so that
// those tests are made.

//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target spirv -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target dxil -profile cs_6_0
//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target dxbc
//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target shader-dll
//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target ptx

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain()
{
    // Will output what downstream compilers will output as a syntax
    // error.
    //CHK:([[#@LINE+1]]): error
    __SyntaxError();
}
