// physics.slang — physics-flavored helpers imported by app.slang.
// Imports math.slang in turn, so the demo exercises a three-module
// dependency chain: app → physics → math. The IR coverage pass
// dedupes the synthesized __slang_coverage buffer across all three so
// a single counter buffer covers every instrumented statement.

import math;

public uint computeStep(uint x, uint frame)
{
    uint accelerated = scaleByFactor(x, frame + 1);
    if (accelerated > 1000)
    {
        // Unreachable on this demo's input range — `x` and `frame`
        // are small. The 0-hit on this line shows the IR coverage
        // pass attributes counters even to never-executed branches.
        return 1000;
    }
    return accelerated;
}

public uint applyDamping(uint value, uint factor)
{
    return clampToRange(value / 2u, 500);
}
