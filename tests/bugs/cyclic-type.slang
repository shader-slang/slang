//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -emit-spirv-directly

struct TA
{
    TB tb;
}

struct TB
/*CHECK:
       ^^ type contains cyclic reference
       ^^ type 'array<TA,2>' contains cyclic reference to itself.
*/
{
    TA ta[2];
}

[numthreads(1,1,1)]
void main()
{
    TA ta;
    ta.tb.ta[0] = ta;
}