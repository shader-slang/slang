//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_G0):-target spirv -g0 -debug-info-include-source -emit-spirv-directly
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_REV):-target spirv -debug-info-include-source -g0 -emit-spirv-directly
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_ALONE):-target spirv -debug-info-include-source -emit-spirv-directly

// `-debug-info-include-source` requires debug information to embed the source into, so it
// conflicts with `-g0` (no debug info). The post-parse validation is order-independent (it reads
// the final option state), so `-g0 -debug-info-include-source` and the reverse both error. And
// because the default debug level (no `-g`) also resolves to None, passing the flag alone is the
// same conflict. All three invocations must error with 57007.

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 1.0f;
}

//CHECK_G0: 57007
//CHECK_REV: 57007
//CHECK_ALONE: 57007
