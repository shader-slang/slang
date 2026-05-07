// app.slang — entry-point module. Imports physics.slang and math.slang
// (and physics.slang in turn imports math.slang) to demonstrate that
// Slang's shader coverage instrumentation handles multi-module compiles
// end-to-end: counters are attributed to the right file:line regardless
// of which imported module the statement lives in.

import math;
import physics;

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    uint stepped = computeStep(i, 5);
    uint damped = applyDamping(stepped, 2);
    outputBuffer[i] = damped;
}
