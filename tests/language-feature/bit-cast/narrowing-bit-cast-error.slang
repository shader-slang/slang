//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -stage compute -entry computeMain

uniform StructuredBuffer<uint64_t> in;
uniform RWStructuredBuffer<uint32_t> out;

//CHECK: error[E41202]: bit_cast size mismatch
//CHECK: '8' casted into {{.*}} '4'

[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain() {
    out[0] = bit_cast<uint32_t, uint64_t>(in[0]);
}