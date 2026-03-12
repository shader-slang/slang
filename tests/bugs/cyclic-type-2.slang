//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -emit-spirv-directly

interface IFoo {}
struct TA<T:IFoo> : IFoo
/*CHECK:
       ^^ type contains cyclic reference
       ^^ type 'TA' contains cyclic reference to itself.
*/
{
    TB<T> tb;
}

struct TB<T:IFoo>
{
    TA<T> ta[2];
}

struct TC : IFoo {}

[numthreads(1,1,1)]
void main()
{
    TA<TC> ta;
    ta.tb.ta[0] = ta;
//CHECK:        ^ use of uninitialized variable
//CHECK:        ^ use of uninitialized variable 'ta'
}