//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NEGATIVE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DNEGATIVE
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_TOO_LARGE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DTOO_LARGE
import experimental.workgraph;

struct Record
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(
    [MaxRecords(1)]
#if defined(NEGATIVE)
    [NodeArraySize(-1)]
//CHECK_NEGATIVE: expected an expression of type 'uint', got 'int'
#elif defined(TOO_LARGE)
    [NodeArraySize(4294967296)]
//CHECK_TOO_LARGE: expected an expression of type 'uint', got 'int64_t'
#endif
    NodeOutputArray<Record> output)
{
}
