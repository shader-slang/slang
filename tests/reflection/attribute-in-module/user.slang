//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK-NOT: warning

import attrib;

[Root]
uniform RWStructuredBuffer<int> buffer;

[numthreads(1,1,1)]
void computeMain()
{
    buffer[0] = 42;
}