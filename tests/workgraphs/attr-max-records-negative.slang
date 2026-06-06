//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NEGATIVE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DNEGATIVE
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_TOO_LARGE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DTOO_LARGE
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NEGATIVE_CONST,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DNEGATIVE_CONST
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_GENERIC): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DGENERIC
import experimental.workgraph;

struct Record
{
    int value;
};

static const int kNegativeMaxRecords = -1;

#if defined(GENERIC)
__generic<let N : int>
#endif
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
#elif defined(NEGATIVE_CONST)
void main([MaxRecords(kNegativeMaxRecords)] NodeOutput<Record> output)
//CHECK_NEGATIVE_CONST: expected an expression of type 'uint', got 'int'
#elif defined(GENERIC)
void main([MaxRecords(N)] NodeOutput<Record> output)
//CHECK_GENERIC: could not extract value from integer constant
#endif
{
}
