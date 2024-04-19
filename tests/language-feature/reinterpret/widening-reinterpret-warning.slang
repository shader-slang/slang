//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -stage compute -entry computeMain

uniform StructuredBuffer<uint32_t> in;
uniform RWStructuredBuffer<uint32_t[2]> out;

//CHECK: warning 41203{{.*}}4{{.*}}8

[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain() {
    out[0] = reinterpret<uint32_t[2]>(in[0]);
}