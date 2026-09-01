//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

// Regression for shader-slang/slang#11844: the synthesized constructor for a struct that
// mixes bitfields with a trailing normal field must store zero into the hidden bitfield
// backing word. This emit-level check is deterministic and backend-robust (no memory or
// GPU dependence): the explicit zero initializer lowers to a `store 0` of the backing
// word in the generated constructor. Before the fix, the constructor wrote only the
// normal field and left the backing word unwritten.

struct Test { uint a : 10; uint b : 22; uint c; };
RWStructuredBuffer<Test> outputBuffer;

// CHECK: {{.*}}bit_field_backing{{.*}} = {{.*}}0
[numthreads(1, 1, 1)]
void computeMain()
{
    Test t = {};
    outputBuffer[0] = t;
}
