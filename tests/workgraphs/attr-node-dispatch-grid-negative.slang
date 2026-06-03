//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_FIXED_NEGATIVE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DFIXED_NEGATIVE
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_FIXED_TOO_LARGE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DFIXED_TOO_LARGE
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MAX_NEGATIVE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMAX_NEGATIVE
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MAX_TOO_LARGE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMAX_TOO_LARGE
import experimental.workgraph;

struct GridData
{
    uint3 grid : SV_DispatchGrid;
};

[shader("node")]
[NodeLaunch("broadcasting")]
#if defined(FIXED_NEGATIVE)
[NodeDispatchGrid(-1, 1, 1)]
//CHECK_FIXED_NEGATIVE: NodeDispatchGrid x must be in the range 0 to 0xffffffff, got -1
#elif defined(FIXED_TOO_LARGE)
[NodeDispatchGrid(4294967296, 1, 1)]
//CHECK_FIXED_TOO_LARGE: NodeDispatchGrid x must be in the range 0 to 0xffffffff, got 4294967296
#elif defined(MAX_NEGATIVE)
[NodeMaxDispatchGrid(-1, 1, 1)]
//CHECK_MAX_NEGATIVE: NodeMaxDispatchGrid x must be in the range 0 to 0xffffffff, got -1
#elif defined(MAX_TOO_LARGE)
[NodeMaxDispatchGrid(4294967296, 1, 1)]
//CHECK_MAX_TOO_LARGE: NodeMaxDispatchGrid x must be in the range 0 to 0xffffffff, got 4294967296
#endif
[numthreads(1, 1, 1)]
void main(DispatchNodeInputRecord<GridData> input)
{
}
