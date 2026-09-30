//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry computeMain -layout-rules-version 202z

// `-layout-rules-version` accepts the same version keywords as `-std`, so an unknown keyword gets
// the same diagnostic. The diagnostic points into the command line, which has no source line to
// anchor a caret to, so we match on its code; the test is non-exhaustive because the diagnostic
// also prints a command-line span row that we do not annotate.

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = 1.0;
}

//CHECK: 15207
