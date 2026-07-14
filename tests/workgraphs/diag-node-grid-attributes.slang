//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_CONFLICT,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DCONFLICT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_CONFLICT_REVERSE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DCONFLICT_REVERSE
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_CONFLICT_DOUBLE_MAX,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DCONFLICT_DOUBLE_MAX
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_THREAD,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DTHREAD
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_THREAD_MAX,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DTHREAD_MAX
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COALESCING,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DCOALESCING
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COALESCING_MAX,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DCOALESCING_MAX
import experimental.workgraph;

[shader("node")]
#if defined(CONFLICT)
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NodeMaxDispatchGrid(2, 2, 2)]
//CHECK_CONFLICT: modifier 'NodeMaxDispatchGrid' is redundant or conflicting with existing modifier 'NodeDispatchGrid'
[numthreads(1, 1, 1)]
#elif defined(CONFLICT_REVERSE)
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(2, 2, 2)]
[NodeDispatchGrid(1, 1, 1)]
//CHECK_CONFLICT_REVERSE: modifier 'NodeDispatchGrid' is redundant or conflicting with existing modifier 'NodeMaxDispatchGrid'
[numthreads(1, 1, 1)]
#elif defined(CONFLICT_DOUBLE_MAX)
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, 1, 1)]
[NodeMaxDispatchGrid(2, 2, 2)]
//CHECK_CONFLICT_DOUBLE_MAX: modifier 'NodeMaxDispatchGrid' is redundant or conflicting with existing modifier 'NodeMaxDispatchGrid'
[numthreads(1, 1, 1)]
#elif defined(THREAD)
[NodeLaunch("thread")]
[NodeDispatchGrid(1, 1, 1)]
//CHECK_THREAD: E31115
#elif defined(THREAD_MAX)
[NodeLaunch("thread")]
[NodeMaxDispatchGrid(1, 1, 1)]
//CHECK_THREAD_MAX: E31115
#elif defined(COALESCING)
[NodeLaunch("coalescing")]
[NodeDispatchGrid(1, 1, 1)]
//CHECK_COALESCING: E31115
#elif defined(COALESCING_MAX)
[NodeLaunch("coalescing")]
[NodeMaxDispatchGrid(1, 1, 1)]
//CHECK_COALESCING_MAX: E31115
#endif
void main()
{
}
