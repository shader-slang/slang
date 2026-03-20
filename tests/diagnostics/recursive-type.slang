//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

struct Outer {
/*CHECK:
       ^^^^^ type contains cyclic reference
       ^^^^^ type 'Outer' contains cyclic reference to itself.
*/
    Outer next; // non-pointer
    int y;
};
RWStructuredBuffer<int> Buf;

[numthreads(1,1,1)]
void csmain() {
    Outer outer;
    outer.y = 0;
    Buf[0] = outer.y;
}
