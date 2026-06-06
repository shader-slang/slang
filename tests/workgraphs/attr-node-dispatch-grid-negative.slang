//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_FIXED_GENERIC): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DFIXED_GENERIC
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MAX_GENERIC): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMAX_GENERIC
import experimental.workgraph;

struct GridData
{
    uint3 grid : SV_DispatchGrid;
};

__generic<let N : int>
[shader("node")]
[NodeLaunch("broadcasting")]
#if defined(FIXED_GENERIC)
[NodeDispatchGrid(N, 1, 1)]
//CHECK_FIXED_GENERIC: could not extract value from integer constant
#elif defined(MAX_GENERIC)
[NodeMaxDispatchGrid(N, 1, 1)]
//CHECK_MAX_GENERIC: could not extract value from integer constant
#endif
[numthreads(1, 1, 1)]
void main(DispatchNodeInputRecord<GridData> input)
{
}
