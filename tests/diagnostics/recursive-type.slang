//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv

// CHECK: error 41001:

struct Outer {
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
