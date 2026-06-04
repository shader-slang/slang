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
#if defined(NEGATIVE)
void main([MaxRecords(-1)] NodeOutput<Record> output)
//CHECK_NEGATIVE: expected an expression of type 'uint', got 'int'
#elif defined(TOO_LARGE)
void main([MaxRecords(4294967296)] NodeOutput<Record> output)
//CHECK_TOO_LARGE: expected an expression of type 'uint', got 'int64_t'
#endif
{
}
