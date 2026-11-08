//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry main -stage compute

// Regression test for #13380. A POD `bit_cast` of a uniform struct holding a scalar array reads each
// element from its std140 `vec4` slot through a vector-to-scalar `vectorReshape`, whose fold must
// produce a typed scalar read for WGSL emission to succeed.

struct P { uint s[3]; uint o; }
struct Q { uint s[3]; uint o; }

RWStructuredBuffer<uint> outBuf;
uniform P p;

// CHECK-LABEL: fn main
// CHECK: outBuf_0[i32(0)] = {{.*}}p_0.s_0.data_0[u32(1)].x + {{.*}}p_0.o_0;
[numthreads(1, 1, 1)]
void main()
{
    Q q = bit_cast<Q, P>(p);
    outBuf[0] = q.s[1] + q.o;
}
