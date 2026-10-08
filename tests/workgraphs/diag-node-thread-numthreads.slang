//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Thread-launch node shaders use an implicit [numthreads(1, 1, 1)].

[shader("node")]
[NodeLaunch("thread")]
[numthreads(8, 8, 1)]
//CHECK: E31119
//CHECK: thread-launch node shaders must not specify '[numthreads]'
void main(RWBuffer<int> output)
{
    output[0] = 0;
}
