//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_INVALID,non-exhaustive): -experimental-feature -DINVALID
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_DUPLICATE,non-exhaustive): -experimental-feature -DDUPLICATE
import experimental.workgraph;

// Test that invalid or duplicate NodeLaunch attributes produce diagnostics.

[shader("node")]
#if defined(INVALID)
[NodeLaunch("invalid")]
//CHECK_INVALID: E31113
#elif defined(DUPLICATE)
[NodeLaunch("broadcasting")]
[NodeLaunch("thread")]
//CHECK_DUPLICATE: duplicate modifier
#endif
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}
