// an int l-value passed to an inout uint parameter inside a generic function compiles for SPIR-V

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

RWStructuredBuffer<int> sb;

void setIt(inout uint x) { x += 7; }

void setInGeneric<T : IInteger>(T unused) { setIt(sb[0]); }

// CHECK: OpEntryPoint
[numthreads(1, 1, 1)]
void computeMain() { setInGeneric(0); }
