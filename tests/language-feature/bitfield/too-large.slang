//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    int foo : 33;
//CHECK:^^^ bit-field size exceeds type width
//CHECK:^^^ bit-field size (33) exceeds the width of its type int (32)
    int64_t bar : 999;
//CHECK:    ^^^ bit-field size exceeds type width
//CHECK:    ^^^ bit-field size (999) exceeds the width of its type int64_t (64)
};

[numthreads(1, 1, 1)]
void computeMain()
{
}
