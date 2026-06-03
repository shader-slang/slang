//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_CONFLICT,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DCONFLICT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_THREAD,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DTHREAD
import experimental.workgraph;

[shader("node")]
#if defined(CONFLICT)
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NodeMaxDispatchGrid(2, 2, 2)]
//CHECK_CONFLICT: E31114
[numthreads(1, 1, 1)]
#elif defined(THREAD)
[NodeLaunch("thread")]
[NodeDispatchGrid(1, 1, 1)]
//CHECK_THREAD: E31115
#endif
void main()
{
}
