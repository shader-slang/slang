//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -incomplete-library -O0

RWByteAddressBuffer buf;

[numthreads(1,1,1)]
[shader("compute")]
// CHECK: OpTypeFunction %void
void main()
{
    buf.Store(0, f(0));
}

// CHECK: OpTypeFunction %int %int
extern int f(int);
